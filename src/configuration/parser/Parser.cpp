#include "Parser.h"

#include "../../entities/Device.h"
#include "../../entities/EndPoint.h"
#include "../../entities/Router.h"
#include "../../logger/Logger.h"
#include "ParserClient.h"
#include "ParserLink.h"
#include "ParserRouter.h"


std::string Parser::getNodeTypeName(YAML::NodeType::value nodeType) {
    std::string name;
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
            throw std::domain_error("Unexpected NodeType");
        }
    }

    return name;
}

void Parser::assertNodeType(const YAML::Node &    node,
                            YAML::NodeType::value expectedNodeType) {
    if (node.Type() != expectedNodeType) {
        throw std::runtime_error(
            "Invalid config at line " + std::to_string(node.Mark().line + 1) +
            ": expected " + getNodeTypeName(expectedNodeType) + " but found " +
            getNodeTypeName(node.Type()));
    }
}

void Parser::throwInvalidKey(const std::string &key, const YAML::Node &node) {
    throw std::runtime_error("Invalid config at line " +
                             std::to_string(node.Mark().line + 1) +
                             ": unknown key \"" + key + "\"");
}

std::vector<Device *> *Parser::parseAndBuild(char *filename) {
    L_VERBOSE("Parser", "PARSING CONFIG FILE");
    L_VERBOSE("Parser", "Parsing from file \"" + std::string(filename) + "\"");

    YAML::Node config_yaml = YAML::LoadFile(filename);
    assertNodeType(config_yaml, YAML::NodeType::value::Map);

    auto *devices_ptr = new std::vector<Device *>;

    for (const auto &device_category_yaml : config_yaml) {
        std::string deviceCategory =
            device_category_yaml.first.as<std::string>();
        YAML::Node devices_yaml = device_category_yaml.second;

        if (deviceCategory == "routers") {
            ParserRouter::parseAndAddBuiltRouters(devices_yaml, devices_ptr);
        } else if (deviceCategory == "clients") {
            ParserClient::parseAndAddBuiltClients(devices_yaml, devices_ptr);
        } else if (deviceCategory == "links") {
            ParserLink::parseAndAddBuiltLinks(devices_yaml, devices_ptr);
        } else {
            throwInvalidKey(deviceCategory, device_category_yaml.first);
        }
    }

    return devices_ptr;
}
