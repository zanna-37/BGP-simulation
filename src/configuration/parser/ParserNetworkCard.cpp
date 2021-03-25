#include "ParserNetworkCard.h"

vector<NetworkCard*>* ParserNetworkCard::parseAndBuildNetworkCards(
    const YAML::Node& networkCards_yaml) {
    assertNodeType(networkCards_yaml, YAML::NodeType::value::Sequence);

    auto* networkCards = new vector<NetworkCard*>;

    for (const auto& networkCard_yaml : networkCards_yaml) {
        string interface;
        string IP_str;
        string netmask_str;
        string defaultGateway;

        for (const auto& detail_yaml : networkCard_yaml) {
            string     property = detail_yaml.first.as<std::string>();
            YAML::Node value    = detail_yaml.second;

            if (property == "interface") {
                interface = value.as<string>();
            } else if (property == "IP") {
                IP_str = value.as<string>();
            } else if (property == "netmask") {
                netmask_str = value.as<string>();
            } else if (property == "default_gateway") {
                defaultGateway = value.as<string>();
            } else {
                throwInvalidKey(property, detail_yaml.first);
            }
        }
        networkCards->push_back(
            new NetworkCard(interface,
                            pcpp::IPv4Address(IP_str),
                            pcpp::IPv4Address(netmask_str)));
    }

    return networkCards;
}
