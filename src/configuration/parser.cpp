#include "parser.h"

#include <iostream>

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

void assertNodeType(
    const YAML::Node& node, YAML::NodeType::value expectedNodeType) {
    if (node.Type() != expectedNodeType) {
        throw runtime_error(
            "Invalid config at line " + to_string(node.Mark().line + 1) +
            ": expected " + getNodeTypeName(expectedNodeType) + " but found " +
            getNodeTypeName(node.Type()));
    }
}

void parseAndBuild(char* filename /*, config mapping*/) {
    YAML::Node config = YAML::LoadFile(filename);
    assertNodeType(config, YAML::NodeType::value::Map);

    for (const auto& devicesContainer : config) {
        string     deviceCategory = devicesContainer.first.as<std::string>();
        YAML::Node devices        = devicesContainer.second;

        assertNodeType(devices, YAML::NodeType::value::Sequence);

        if (deviceCategory == "routers") {
            // TODO
            cout << "r" << endl;
        } else if (deviceCategory == "clients") {
            // TODO
            cout << "c" << endl;
        } else {
            // TODO throw error
            cout << "error" << endl;
        }
    }
}
