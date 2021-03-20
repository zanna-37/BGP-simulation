#ifndef BGP_SIMULATION_CONFIGURATION_PARSER_PARSERCLIENT_H
#define BGP_SIMULATION_CONFIGURATION_PARSER_PARSERCLIENT_H

void parseAndAddBuiltClients(const YAML::Node &clients_yaml,
                             vector<Device *> *devices_ptr);

#endif  // BGP_SIMULATION_CONFIGURATION_PARSER_PARSERCLIENT_H
