#include "extauth.h"

#include "common/common/assert.h"
#include "common/common/enum_to_int.h"
#include "common/http/message_impl.h"
#include "common/http/utility.h"

namespace Envoy {
namespace Http {

static LowerCaseString header_to_add(std::string("x-ark3-stuff"));

ExtAuth::ExtAuth(ExtAuthConfigConstSharedPtr config) : config_(config) {}

ExtAuth::~ExtAuth() {
  ASSERT(!delay_timer_);
  ASSERT(!auth_request_);
}

FilterHeadersStatus ExtAuth::decodeHeaders(HeaderMap& headers, bool) {
  log().info("ExtAuth Request received; contacting auth server");

  // Copy original headers as a JSON object
  std::string json("{");
  headers.iterate(
      [](const HeaderEntry& header, void* ctx) -> void {
        std::string* jsonPtr = static_cast<std::string*>(ctx);
        std::string key(header.key().c_str());
        std::string value(header.value().c_str());
        // TODO(ark3): Ensure that key and value are sane so generated JSON is valid
        *jsonPtr += "\n \"" + key + "\": \"" + value + "\",";
      },
      &json);
  std::string request_body = json.substr(0, json.size() - 1) + "\n}"; // Drop trailing comma

  // Request external authentication
  auth_complete_ = false;
  MessagePtr request(new RequestMessageImpl());
  request->headers().insertMethod().value(Http::Headers::get().MethodValues.Post);
  request->headers().insertPath().value(std::string("/post"));
  request->headers().insertHost().value(config_->cluster_); // cluster name is Host: header value!
  request->headers().insertContentType().value(std::string("application/json"));
  request->headers().insertContentLength().value(request_body.size());
  request->body() = Buffer::InstancePtr(new Buffer::OwnedImpl(request_body));
  auth_request_ =
      config_->cm_.httpAsyncClientForCluster(config_->cluster_)
          .send(std::move(request), *this, Optional<std::chrono::milliseconds>(config_->timeout_));
  // .send(...) -> onSuccess(...) or onFailure(...)
  // This handle can be used to .cancel() the request.

  // Stop until we have a result
  return FilterHeadersStatus::StopIteration;
}

FilterDataStatus ExtAuth::decodeData(Buffer::Instance&, bool) {
  if (auth_complete_) {
    return FilterDataStatus::Continue;
  }
  return FilterDataStatus::StopIterationAndBuffer;
}

FilterTrailersStatus ExtAuth::decodeTrailers(HeaderMap&) {
  if (auth_complete_) {
    return FilterTrailersStatus::Continue;
  }
  return FilterTrailersStatus::StopIteration;
}

ExtAuthStats ExtAuth::generateStats(const std::string& prefix, Stats::Store& store) {
  std::string final_prefix = prefix + "extauth.";
  return {ALL_EXTAUTH_STATS(POOL_COUNTER_PREFIX(store, final_prefix))};
}

void ExtAuth::onSuccess(Http::MessagePtr&& response) {
  auth_request_ = nullptr;
  uint64_t response_code = Http::Utility::getResponseStatus(response->headers());
  log().info("ExtAuth Auth responded with code {}", response_code);
  if (response_code != enumToInt(Http::Code::OK)) {
    rejectRequest();
    return;
  }
  acceptRequest();
}

void ExtAuth::onFailure(Http::AsyncClient::FailureReason) {
  auth_request_ = nullptr;
  log().warn("ExtAuth Auth request failed");
  config_->stats_.rq_failed_.inc();
  rejectRequest();
}

void ExtAuth::acceptRequest() {
  log().info("ExtAuth accepting request");
  auth_complete_ = true;
  config_->stats_.rq_passed_.inc();
  callbacks_->continueDecoding();
}

void ExtAuth::rejectRequest() {
  log().info("ExtAuth rejecting request");
  Http::HeaderMapPtr response_headers{
      new HeaderMapImpl{{Headers::get().Status, std::string("403")}}};
  callbacks_->encodeHeaders(std::move(response_headers), true);
  config_->stats_.rq_rejected_.inc();
  // TODO(ark3): Need a different response flag
  callbacks_->requestInfo().setResponseFlag(Http::AccessLog::ResponseFlag::FaultInjected);
}

void ExtAuth::redirectRequest() {
  log().info("ExtAuth redirecting request");
  Http::HeaderMapPtr response_headers{
      new HeaderMapImpl{{Headers::get().Status, std::string("307")}}};
  // TODO(ark3): Need to set the Location header here
  response_headers->addStaticKey(header_to_add, std::string("Hello world"));
  callbacks_->encodeHeaders(std::move(response_headers), true);
  config_->stats_.rq_redirected_.inc();
  // TODO(ark3): Need a different response flag
  callbacks_->requestInfo().setResponseFlag(Http::AccessLog::ResponseFlag::FaultInjected);
}

void ExtAuth::onDestroy() { resetInternalState(); }

void ExtAuth::resetInternalState() {
  if (delay_timer_) {
    delay_timer_->disableTimer();
    delay_timer_.reset();
  }
}

void ExtAuth::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  callbacks_ = &callbacks;
}

} // Http
} // Envoy
