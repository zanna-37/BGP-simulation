#ifndef BGP_SIMULATION_CONFIGURATION_PARSER_PARSERNETWORKCARD_H
#define BGP_SIMULATION_CONFIGURATION_PARSER_PARSERNETWORKCARD_H

#include <vector>

#include "../../entities/Device.h"
#include "Parser.h"
#include "yaml-cpp/yaml.h"

class ParserNetworkCard : public virtual Parser {
   public:
    static vector<NetworkCard*>* parseAndBuildNetworkCards(
        const YAML::Node& networkCards_yaml, Device* owner);

   private:
    ParserNetworkCard() = delete;
};

#endif  // BGP_SIMULATION_CONFIGURATION_PARSER_PARSERNETWORKCARD_H
