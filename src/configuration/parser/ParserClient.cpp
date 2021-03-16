#include <yaml-cpp/yaml.h>

#include <iostream>

#include "../../entities/Client.h"
#include "../../entities/Connection.h"
#include "Parser.h"

void parseAndAddBuiltClients(const YAML::Node &clients_yaml,
                             vector<Device *> *devices_ptr) {
    assertNodeType(clients_yaml, YAML::NodeType::value::Sequence);

    for (const auto &client_yaml : clients_yaml) {
        string      ID;
        Connection *connection = nullptr;

        for (const auto &client_property_yaml : client_yaml) {
            string     property = client_property_yaml.first.as<std::string>();
            YAML::Node value    = client_property_yaml.second;

            if (property == "id") {
                ID = value.as<string>();
            } else if (property == "network") {
                cout << "TODO: parse Client network" << endl;  // TODO
            } else {
                throwInvalidKey(property, client_property_yaml.first);
            }
        }

        devices_ptr->push_back(new Client(ID, connection));
    }
}