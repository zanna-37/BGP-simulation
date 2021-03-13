#include <yaml-cpp/yaml.h>

#include "../../entities/Connection.h"
#include "../../entities/Router.h"
#include "Parser.h"

void parseAndAddBuiltRouters(const YAML::Node &routers_yaml,
                             vector<Device *> *devices_ptr) {
    assertNodeType(routers_yaml, YAML::NodeType::value::Sequence);

    for (const auto &router_yaml : routers_yaml) {
        string      ID;
        string      AS_number;
        Connection *connections = nullptr;

        for (const auto &router_property_yaml : router_yaml) {
            string     property = router_property_yaml.first.as<std::string>();
            YAML::Node value    = router_property_yaml.second;

            if (property == "id") {
                ID = value.as<string>();
            } else if (property == "AS_number") {
                AS_number = value.as<string>();
            } else if (property == "network") {
                // TODO
            } else {
                throwInvalidKey(property, router_property_yaml.first);
            }
        }

        devices_ptr->push_back(new Router(ID, AS_number, connections));
    }
}