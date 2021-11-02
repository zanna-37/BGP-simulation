#ifndef BGP_SIMULATION_CONFIGURATION_PARSER_PARSER_H
#define BGP_SIMULATION_CONFIGURATION_PARSER_PARSER_H

#include <string>
#include <vector>

#include "../../entities/Device.h"
#include "yaml-cpp/yaml.h"


class Parser {
   public:
    static std::vector<Device *> *parseAndBuild(char *filename);

   protected:
    static std::string getNodeTypeName(YAML::NodeType::value nodeType);

    static void throwInvalidKey(const std::string &key, const YAML::Node &node);

    static void assertNodeType(const YAML::Node &    node,
                               YAML::NodeType::value expectedNodeType);

   private:
    Parser() = delete;
};

#endif  // BGP_SIMULATION_CONFIGURATION_PARSER_PARSER_H
