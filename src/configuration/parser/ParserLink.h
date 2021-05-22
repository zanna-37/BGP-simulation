#ifndef BGP_SIMULATION_CONFIGURATION_PARSER_LINK_H
#define BGP_SIMULATION_CONFIGURATION_PARSER_LINK_H

#include <vector>

#include "../../entities/Device.h"
#include "Parser.h"
#include "yaml-cpp/yaml.h"

class ParserLink : public virtual Parser {
   public:
    static void parseAndAddBuiltLinks(const YAML::Node &     links_yaml,
                                      std::vector<Device *> *devices_ptr);

   private:
    ParserLink() = delete;
};

#endif  // BGP_SIMULATION_CONFIGURATION_PARSER_LINK_H
