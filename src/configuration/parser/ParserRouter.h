#ifndef BGP_SIMULATION_CONFIGURATION_PARSER_PARSERROUTER_H
#define BGP_SIMULATION_CONFIGURATION_PARSER_PARSERROUTER_H


void parseAndAddBuiltRouters(const YAML::Node &routers_yaml,
                             vector<Device *> *devices_ptr);

#endif  // BGP_SIMULATION_CONFIGURATION_PARSER_PARSERROUTER_H