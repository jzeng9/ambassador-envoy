#include "envoy/network/connection.h"

#include "extauth.h"
#include "server/configuration_impl.h"

namespace Server {
namespace Configuration {

/**
 * Config registration for the echo filter. @see NetworkFilterConfigFactory.
 */
class ExtAuthConfigFactory : public NetworkFilterConfigFactory {
public:
  // NetworkFilterConfigFactory
  NetworkFilterFactoryCb tryCreateFilterFactory(NetworkFilterType type, const std::string& name,
                                                const Json::Object&, Server::Instance&) {
    if (type != NetworkFilterType::Read || name != "extauth") {
      return nullptr;
    }

    return [](Network::FilterManager& filter_manager)
        -> void { filter_manager.addReadFilter(Network::ReadFilterSharedPtr{new Filter::ExtAuth()}); };
  }
};

/**
 * Static registration for the echo filter. @see RegisterNetworkFilterConfigFactory.
 */
static RegisterNetworkFilterConfigFactory<extauthConfigFactory> registered_;

} // Configuration
} // Server
