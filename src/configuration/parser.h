#ifndef BGP_SIMULATION_CONFIGURATION_PARSER_H
#define BGP_SIMULATION_CONFIGURATION_PARSER_H

#include "yaml-cpp/yaml.h"

using namespace std;

string getNodeTypeName(YAML::NodeType::value nodeType);
void   assertNodeType(
      const YAML::Node& node, YAML::NodeType::value expectedNodeType);
void parseAndBuild(char* filename /*, config mapping*/);

#endif  // BGP_SIMULATION_CONFIGURATION_PARSER_H
