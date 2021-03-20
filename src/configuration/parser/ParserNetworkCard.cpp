#include <yaml-cpp/yaml.h>

#include "Parser.h"


vector<NetworkCard*>* parseAndBuildNetworkCards(
    const YAML::Node& networkCards_yaml) {
    assertNodeType(networkCards_yaml, YAML::NodeType::value::Sequence);

    auto* networkCards = new vector<NetworkCard*>;

    for (const auto& networkCard_yaml : networkCards_yaml) {
        string interface;
        string IP;
        string netmask;
        string default_gateway;

        for (const auto& detail_yaml : networkCard_yaml) {
            string     property = detail_yaml.first.as<std::string>();
            YAML::Node value    = detail_yaml.second;

            if (property == "interface") {
                interface = value.as<string>();
            } else if (property == "IP") {
                IP = value.as<string>();
            } else if (property == "netmask") {
                netmask = value.as<string>();
            } else if (property == "default_gateway") {
                default_gateway = value.as<string>();
            } else {
                throwInvalidKey(property, detail_yaml.first);
            }
        }
        networkCards->push_back(
            new NetworkCard(interface, IP, netmask, default_gateway));
    }

    return networkCards;
}
