#include "extauth.h"

#include "common/common/assert.h"

namespace Envoy {
namespace Http {

ExtAuth::ExtAuth(ExtAuthConfigConstSharedPtr config) : config_(config) {}

ExtAuth::~ExtAuth() { ASSERT(!request_timeout_); }

FilterHeadersStatus ExtAuth::decodeHeaders(HeaderMap&, bool end_stream) {
  config_->stats_.rq_handled_.inc();
  if (end_stream) {
    // This is a header only request
    return FilterHeadersStatus::Continue;
  } else {
    // This request has body data
    return FilterHeadersStatus::Continue;
  }
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

void ExtAuth::onDestroy() { resetInternalState(); }

void ExtAuth::resetInternalState() {}

void ExtAuth::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  callbacks_ = &callbacks;
}

} // Http
} // Envoy
