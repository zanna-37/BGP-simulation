#ifndef BGP_SIMULATION_CONFIGURATION_PARSER_PARSER_H
#define BGP_SIMULATION_CONFIGURATION_PARSER_PARSER_H

#include "../../entities/Device.h"
#include "yaml-cpp/yaml.h"

using namespace std;

vector<Device *> *parseAndBuild(char *filename /*, config mapping*/);

string getNodeTypeName(YAML::NodeType::value nodeType);

void throwInvalidKey(const string &key, const YAML::Node &node);

void assertNodeType(const YAML::Node &    node,
                    YAML::NodeType::value expectedNodeType);

#endif  // BGP_SIMULATION_CONFIGURATION_PARSER_PARSER_H
