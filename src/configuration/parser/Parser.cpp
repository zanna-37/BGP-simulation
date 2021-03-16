#include "Parser.h"

#include <iostream>

#include "../../entities/Client.h"
#include "../../entities/Router.h"
#include "ParserClient.h"
#include "ParserRouter.h"


string getNodeTypeName(YAML::NodeType::value nodeType) {
    string name;
    switch (nodeType) {
        case YAML::NodeType::Null: {
            name = "Null";
            break;
        }
        case YAML::NodeType::Scalar: {
            name = "Scalar";
            break;
        }
        case YAML::NodeType::Sequence: {
            name = "Sequence";
            break;
        }
        case YAML::NodeType::Map: {
            name = "Map";
            break;
        }
        case YAML::NodeType::Undefined: {
            name = "Undefined";
            break;
        }
        default: {
            throw domain_error("Unexpected NodeType");
        }
    }

    return name;
}

void assertNodeType(const YAML::Node &    node,
                    YAML::NodeType::value expectedNodeType) {
    if (node.Type() != expectedNodeType) {
        throw runtime_error("Invalid config at line " +
                            to_string(node.Mark().line + 1) + ": expected " +
                            getNodeTypeName(expectedNodeType) + " but found " +
                            getNodeTypeName(node.Type()));
    }
}

void throwInvalidKey(const string &key, const YAML::Node &node) {
    throw runtime_error("Invalid config at line " +
                        to_string(node.Mark().line + 1) + ": unknown key \"" +
                        key + "\"");
}

vector<Device *> *parseAndBuild(char *filename /*, config mapping*/) {
    cout << "[#] Parsing config file" << endl;
    YAML::Node config_yaml = YAML::LoadFile(filename);
    assertNodeType(config_yaml, YAML::NodeType::value::Map);

    auto *devices_ptr = new vector<Device *>;

    for (const auto &device_category_yaml : config_yaml) {
        string deviceCategory   = device_category_yaml.first.as<std::string>();
        YAML::Node devices_yaml = device_category_yaml.second;

        if (deviceCategory == "routers") {
            parseAndAddBuiltRouters(devices_yaml, devices_ptr);
        } else if (deviceCategory == "clients") {
            parseAndAddBuiltClients(devices_yaml, devices_ptr);
        } else if (deviceCategory == "links") {
            cout << "TODO: parse Links" << endl;  // TODO
        } else {
            throwInvalidKey(deviceCategory, device_category_yaml.first);
        }
    }

    return devices_ptr;
}
