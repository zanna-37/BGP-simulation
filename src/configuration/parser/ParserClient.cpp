#include "ParserClient.h"

#include <string>

#include "../../entities/Device.h"
#include "../../entities/EndPoint.h"
#include "../../entities/NetworkCard.h"
#include "../../logger/Logger.h"
#include "IpAddress.h"
#include "ParserNetworkCard.h"


void ParserClient::parseAndAddBuiltClients(const YAML::Node &     clients_yaml,
                                           std::vector<Device *> *devices_ptr) {
    L_DEBUG("Parser", "Parsing Clients");

    assertNodeType(clients_yaml, YAML::NodeType::value::Sequence);

    for (const auto &client_yaml : clients_yaml) {
        std::string                 ID;
        pcpp::IPv4Address           defaultGateway;
        std::vector<NetworkCard *> *networkCards = nullptr;

        YAML::Node networkCards_yaml;

        for (const auto &client_property_yaml : client_yaml) {
            std::string property = client_property_yaml.first.as<std::string>();
            YAML::Node  value    = client_property_yaml.second;

            if (property == "id") {
                ID = value.as<std::string>();
            } else if (property == "default_gateway") {
                std::string defaultGateway_str = value.as<std::string>();
                defaultGateway = pcpp::IPv4Address(defaultGateway_str);
            } else if (property == "networkCard") {
                networkCards_yaml = value;
            } else {
                throwInvalidKey(property, client_property_yaml.first);
            }
        }

        EndPoint *endPoint = new EndPoint(ID, defaultGateway);

        networkCards = ParserNetworkCard::parseAndBuildNetworkCards(
            networkCards_yaml, endPoint);

        endPoint->addCards(networkCards);
        devices_ptr->push_back(endPoint);

        if (networkCards->size() > 1) {
            L_WARNING("Parser",
                      "Found multiple networkCards on " + ID + "\n" + "Last " +
                          std::to_string(networkCards->size() - 1) +
                          " will not be used");
        }
    }
}
