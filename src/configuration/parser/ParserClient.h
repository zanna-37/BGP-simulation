#ifndef BGP_SIMULATION_CONFIGURATION_PARSER_PARSERCLIENT_H
#define BGP_SIMULATION_CONFIGURATION_PARSER_PARSERCLIENT_H

#include <vector>

#include "../../entities/Device.h"
#include "Parser.h"
#include "yaml-cpp/yaml.h"

class ParserClient : public virtual Parser {
   public:
    static void parseAndAddBuiltClients(const YAML::Node &     clients_yaml,
                                        std::vector<Device *> *devices_ptr);

   private:
    ParserClient() = delete;
};

#endif  // BGP_SIMULATION_CONFIGURATION_PARSER_PARSERCLIENT_H
