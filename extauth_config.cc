#include "extauth_config.h"

#include "extauth.h"

namespace Envoy {
namespace Server {
namespace Configuration {

const std::string EXTAUTH_HTTP_FILTER_SCHEMA(R"EOF(
  {
    "$schema": "http://json-schema.org/schema#",
    "type" : "object",
    "properties" : {
      "unused" : {"type" : "integer"}
    },
    "required" : ["unused"],
    "additionalProperties" : false
  }
  )EOF");

HttpFilterFactoryCb ExtAuthConfig::tryCreateFilterFactory(HttpFilterType type,
                                                          const std::string& name,
                                                          const Json::Object& json_config,
                                                          const std::string& stats_prefix,
                                                          Server::Instance& server) {
  if (type != HttpFilterType::Decoder || name != "extauth") {
    return nullptr;
  }

  json_config.validateSchema(EXTAUTH_HTTP_FILTER_SCHEMA);

  Http::ExtAuthConfigConstSharedPtr config(
      new Http::ExtAuthConfig{Http::ExtAuth::generateStats(stats_prefix, server.stats()),
                              static_cast<uint64_t>(json_config.getInteger("unused"))});
  return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamDecoderFilter(Http::StreamDecoderFilterSharedPtr{new Http::ExtAuth(config)});
  };
}

/**
 * Static registration for the extauth filter. @see RegisterHttpFilterConfigFactory.
 */
static RegisterHttpFilterConfigFactory<ExtAuthConfig> register_;

} // Configuration
} // Server
} // Envoy
