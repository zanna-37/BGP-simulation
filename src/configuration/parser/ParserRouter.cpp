#include "ParserRouter.h"

#include <string>

#include "../../entities/Device.h"
#include "../../entities/NetworkCard.h"
#include "../../entities/Router.h"
#include "../../logger/Logger.h"
#include "IpAddress.h"
#include "ParserNetworkCard.h"
#include "yaml-cpp/yaml.h"


void ParserRouter::parseAndAddBuiltRouters(const YAML::Node &     routers_yaml,
                                           std::vector<Device *> *devices_ptr) {
    L_DEBUG("Parser", "Parsing Routers");

    assertNodeType(routers_yaml, YAML::NodeType::value::Sequence);

    for (const auto &router_yaml : routers_yaml) {
        std::string                    ID;
        std::string                    AS_number;
        pcpp::IPv4Address              defaultGateway;
        std::vector<NetworkCard *> *   networkCards = nullptr;
        std::vector<pcpp::IPv4Address> peer_addresses;

        YAML::Node networkCards_yaml;

        for (const auto &router_property_yaml : router_yaml) {
            std::string property = router_property_yaml.first.as<std::string>();
            YAML::Node  value    = router_property_yaml.second;

            if (property == "id") {
                ID = value.as<std::string>();
            } else if (property == "AS_number") {
                AS_number = value.as<std::string>();
            } else if (property == "default_gateway") {
                std::string defaultGateway_str = value.as<std::string>();
                defaultGateway = pcpp::IPv4Address(defaultGateway_str);
            } else if (property == "networkCard") {
                networkCards_yaml = value;
            } else if (property == "peers") {
                auto peer_addresses_yaml = value;
                assertNodeType(value, YAML::NodeType::value::Sequence);
                for (const auto &peer_address_yaml : peer_addresses_yaml) {
                    peer_addresses.emplace_back(
                        peer_address_yaml.as<std::string>());
                }
            } else {
                throwInvalidKey(property, router_property_yaml.first);
            }
        }
        Router *router =
            new Router(ID, AS_number, defaultGateway, peer_addresses);

        networkCards = ParserNetworkCard::parseAndBuildNetworkCards(
            networkCards_yaml, router);

        router->addCards(networkCards);
        devices_ptr->push_back(router);
    }
}
