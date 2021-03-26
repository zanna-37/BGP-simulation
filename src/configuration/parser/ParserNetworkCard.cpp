#include "ParserNetworkCard.h"

vector<NetworkCard*>* ParserNetworkCard::parseAndBuildNetworkCards(
    const YAML::Node& networkCards_yaml, Device* owner) {
    assertNodeType(networkCards_yaml, YAML::NodeType::value::Sequence);

    auto* networkCards = new vector<NetworkCard*>;

    for (const auto& networkCard_yaml : networkCards_yaml) {
        string            interface;
        pcpp::IPv4Address IP;
        pcpp::IPv4Address netmask;

        for (const auto& detail_yaml : networkCard_yaml) {
            string     property = detail_yaml.first.as<std::string>();
            YAML::Node value    = detail_yaml.second;

            if (property == "interface") {
                interface = value.as<string>();
            } else if (property == "IP") {
                string IP_str = value.as<string>();
                IP            = pcpp::IPv4Address(IP_str);
            } else if (property == "netmask") {
                string netmask_str = value.as<string>();
                netmask            = pcpp::IPv4Address(netmask_str);
            } else {
                throwInvalidKey(property, detail_yaml.first);
            }
        }
        networkCards->push_back(new NetworkCard(interface, IP, netmask, owner));
    }

    return networkCards;
}
