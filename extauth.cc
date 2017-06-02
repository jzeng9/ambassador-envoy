#include "extauth.h"

#include "common/common/assert.h"
#include "common/http/header_map_impl.h"
#include "common/http/headers.h"

namespace Envoy {
namespace Http {

static LowerCaseString header_to_add(std::string("x-ark3-stuff"));

ExtAuth::ExtAuth(ExtAuthConfigConstSharedPtr config) : config_(config) {}

ExtAuth::~ExtAuth() { ASSERT(!delay_timer_); }

FilterHeadersStatus ExtAuth::decodeHeaders(HeaderMap&, bool) {
  log().info("ExtAuth Request received; contacting auth server");

  // Request external authentication
  auth_complete_ = false;
  delay_timer_ = callbacks_->dispatcher().createTimer([this]() -> void { onAuthResult(); });
  delay_timer_->enableTimer(std::chrono::milliseconds(1500));

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

void ExtAuth::onAuthResult(/* Http::HeaderMapPtr&& headers */) {
  resetInternalState();
  log().info("ExtAuth Auth Result received");

  if (false) {
    rejectRequest();
  } else if (false) {
    redirectRequest();
  } else {
    acceptRequest();
  }
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
