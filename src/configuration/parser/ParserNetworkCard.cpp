#include "ParserNetworkCard.h"

std::vector<NetworkCard*>* ParserNetworkCard::parseAndBuildNetworkCards(
    const YAML::Node& networkCards_yaml, Device* owner) {
    assertNodeType(networkCards_yaml, YAML::NodeType::value::Sequence);

    auto* networkCards = new std::vector<NetworkCard*>;

    for (const auto& networkCard_yaml : networkCards_yaml) {
        std::string       interface;
        pcpp::IPv4Address IP;
        pcpp::IPv4Address netmask;
        pcpp::MacAddress  mac(pcpp::MacAddress::Zero);

        for (const auto& detail_yaml : networkCard_yaml) {
            std::string property = detail_yaml.first.as<std::string>();
            YAML::Node  value    = detail_yaml.second;

            if (property == "interface") {
                interface = value.as<std::string>();
            } else if (property == "IP") {
                std::string IP_str = value.as<std::string>();
                IP            = pcpp::IPv4Address(IP_str);
            } else if (property == "netmask") {
                std::string netmask_str = value.as<std::string>();
                netmask            = pcpp::IPv4Address(netmask_str);
            } else if (property == "mac") {
                std::string mac_str = value.as<std::string>();
                mac            = pcpp::MacAddress(mac_str);
            } else {
                throwInvalidKey(property, detail_yaml.first);
            }
        }
        networkCards->push_back(
            new NetworkCard(interface, IP, netmask, mac, owner));
    }

    return networkCards;
}
