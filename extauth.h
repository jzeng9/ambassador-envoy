#pragma once

#include "envoy/http/filter.h"

#include "common/common/logger.h"

namespace Envoy {
namespace Http {

/**
 * All stats for the extauth filter. @see stats_macros.h
 */
// clang-format off
#define ALL_EXTAUTH_STATS(COUNTER)                                                           \
  COUNTER(rq_handled)
// clang-format on

/**
 * Wrapper struct for extauth filter stats. @see stats_macros.h
 */
struct ExtAuthStats {
  ALL_EXTAUTH_STATS(GENERATE_COUNTER_STRUCT)
};

/**
 * Configuration for the extauth filter.
 */
struct ExtAuthConfig {
  ExtAuthStats stats_;
  uint64_t unused_;
};

typedef std::shared_ptr<const ExtAuthConfig> ExtAuthConfigConstSharedPtr;

/**
 * A pass-through filter that talks to an external authn/authz service (or will soon...)
 */
class ExtAuth : public StreamDecoderFilter {
public:
  ExtAuth(ExtAuthConfigConstSharedPtr config);
  ~ExtAuth();

  static ExtAuthStats generateStats(const std::string& prefix, Stats::Store& store);

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(HeaderMap& headers, bool end_stream) override;
  FilterDataStatus decodeData(Buffer::Instance& data, bool end_stream) override;
  FilterTrailersStatus decodeTrailers(HeaderMap& trailers) override;
  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) override;

private:
  void resetInternalState();

  ExtAuthConfigConstSharedPtr config_;
  StreamDecoderFilterCallbacks* callbacks_{};
  Event::TimerPtr request_timeout_;
};

} // Http
} // Envoy
