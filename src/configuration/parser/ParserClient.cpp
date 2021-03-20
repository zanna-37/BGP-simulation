#include <yaml-cpp/yaml.h>

#include <iostream>

#include "../../entities/Client.h"
#include "../../entities/Link.h"
#include "Parser.h"
#include "ParserNetworkCard.h"

void parseAndAddBuiltClients(const YAML::Node &clients_yaml,
                             vector<Device *> *devices_ptr) {
    assertNodeType(clients_yaml, YAML::NodeType::value::Sequence);

    for (const auto &client_yaml : clients_yaml) {
        string                 ID;
        vector<NetworkCard *> *networkCards = nullptr;

        for (const auto &client_property_yaml : client_yaml) {
            string     property = client_property_yaml.first.as<std::string>();
            YAML::Node value    = client_property_yaml.second;

            if (property == "id") {
                ID = value.as<string>();
            } else if (property == "network") {
                networkCards = parseAndBuildNetworkCards(value);
            } else {
                throwInvalidKey(property, client_property_yaml.first);
            }
        }

        devices_ptr->push_back(new Client(ID, networkCards));

        if (networkCards->size() > 1) {
            cout << "[!] Warning: Found multiple networkCards on " << ID << endl
                 << "    Last " << networkCards->size() - 1
                 << " will not be used" << endl;
        }
    }
}