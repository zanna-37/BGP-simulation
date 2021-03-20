#ifndef BGP_SIMULATION_CONFIGURATION_PARSER_LINK_H
#define BGP_SIMULATION_CONFIGURATION_PARSER_LINK_H

void parseAndAddBuiltLinks(const YAML::Node &links_yaml,
                           vector<Device *> *devices_ptr);


#endif  // BGP_SIMULATION_CONFIGURATION_PARSER_LINK_H
