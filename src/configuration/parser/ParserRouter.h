#ifndef BGP_SIMULATION_CONFIGURATION_PARSER_PARSERROUTER_H
#define BGP_SIMULATION_CONFIGURATION_PARSER_PARSERROUTER_H

#include <vector>

#include "../../entities/Device.h"
#include "Parser.h"
#include "yaml-cpp/yaml.h"

class ParserRouter : public virtual Parser {
   public:
    static void parseAndAddBuiltRouters(const YAML::Node &routers_yaml,
                                        vector<Device *> *devices_ptr);

   private:
    ParserRouter() = delete;
};

#endif  // BGP_SIMULATION_CONFIGURATION_PARSER_PARSERROUTER_H
