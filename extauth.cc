#include "extauth.h"

#include "common/common/assert.h"
#include "common/http/header_map_impl.h"
#include "common/http/headers.h"

namespace Envoy {
namespace Http {

static LowerCaseString header_to_add(std::string("x-ark3-stuff"));

ExtAuth::ExtAuth(ExtAuthConfigConstSharedPtr config) : config_(config) {}

ExtAuth::~ExtAuth() { ASSERT(!request_timeout_); }

FilterHeadersStatus ExtAuth::decodeHeaders(HeaderMap&, bool) {
  if (false) {
    rejectRequest();
    return FilterHeadersStatus::StopIteration;
  } else if (false) {
    redirectRequest();
    return FilterHeadersStatus::StopIteration;
  }

  acceptRequest();
  return FilterHeadersStatus::Continue;
}

FilterDataStatus ExtAuth::decodeData(Buffer::Instance&, bool end_stream) {
  if (end_stream) {
    resetInternalState();
  }
  return FilterDataStatus::Continue;
}

FilterTrailersStatus ExtAuth::decodeTrailers(HeaderMap&) {
  resetInternalState();
  return FilterTrailersStatus::Continue;
}

ExtAuthStats ExtAuth::generateStats(const std::string& prefix, Stats::Store& store) {
  std::string final_prefix = prefix + "extauth.";
  return {ALL_EXTAUTH_STATS(POOL_COUNTER_PREFIX(store, final_prefix))};
}

void ExtAuth::acceptRequest() {
  log().info("ExtAuth accepting request");
  config_->stats_.rq_passed_.inc();
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
  response_headers->addStaticKey(header_to_add, std::string("Hello world"));
  callbacks_->encodeHeaders(std::move(response_headers), true);
  config_->stats_.rq_redirected_.inc();
  // TODO(ark3): Need a different response flag
  callbacks_->requestInfo().setResponseFlag(Http::AccessLog::ResponseFlag::FaultInjected);
}

void ExtAuth::onDestroy() { resetInternalState(); }

void ExtAuth::resetInternalState() {}

void ExtAuth::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  callbacks_ = &callbacks;
}

} // Http
} // Envoy
