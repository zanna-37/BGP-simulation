#include <yaml-cpp/yaml.h>

#include "Parser.h"


vector<Network_details*>* parseAndBuildNetwork_details(
    const YAML::Node& network_details_list_yaml) {
    assertNodeType(network_details_list_yaml, YAML::NodeType::value::Sequence);

    auto* network_details_list = new vector<Network_details*>;

    for (const auto& network_details_single_yaml : network_details_list_yaml) {
        string interface;
        string IP;
        string netmask;
        string default_gateway;

        for (const auto& detail_yaml : network_details_single_yaml) {
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
        network_details_list->push_back(
            new Network_details(interface, IP, netmask, default_gateway));
    }

    return network_details_list;
}