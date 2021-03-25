#include "ParserClient.h"

#include "../../entities/Client.h"
#include "../../logger/Logger.h"
#include "ParserNetworkCard.h"

void ParserClient::parseAndAddBuiltClients(const YAML::Node &clients_yaml,
                                           vector<Device *> *devices_ptr) {
    L_DEBUG("Parsing Clients");

    assertNodeType(clients_yaml, YAML::NodeType::value::Sequence);

    for (const auto &client_yaml : clients_yaml) {
        string                 ID;
        pcpp::IPv4Address      defaultGateway;
        vector<NetworkCard *> *networkCards = nullptr;

        for (const auto &client_property_yaml : client_yaml) {
            string     property = client_property_yaml.first.as<std::string>();
            YAML::Node value    = client_property_yaml.second;

            if (property == "id") {
                ID = value.as<string>();
            } else if (property == "default_gateway") {
                string defaultGateway_str = value.as<string>();
                defaultGateway = pcpp::IPv4Address(defaultGateway_str);
            } else if (property == "networkCard") {
                networkCards =
                    ParserNetworkCard::parseAndBuildNetworkCards(value);
            } else {
                throwInvalidKey(property, client_property_yaml.first);
            }
        }

        devices_ptr->push_back(new Client(ID, defaultGateway, networkCards));

        if (networkCards->size() > 1) {
            L_WARNING("Found multiple networkCards on " + ID + "\n" + "Last " +
                      to_string(networkCards->size() - 1) +
                      " will not be used");
        }
    }
}
