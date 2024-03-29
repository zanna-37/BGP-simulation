#include "ParserLink.h"

#include <memory>
#include <string>
#include <utility>

#include "../../entities/Device.h"
#include "../../entities/Link.h"
#include "../../entities/NetworkCard.h"
#include "../../logger/Logger.h"
#include "yaml-cpp/yaml.h"


void ParserLink::parseAndAddBuiltLinks(const YAML::Node &     links_yaml,
                                       std::vector<Device *> *devices_ptr) {
    L_DEBUG("Parser", "Parsing Links");

    assertNodeType(links_yaml, YAML::NodeType::value::Sequence);

    for (const auto &link_yaml : links_yaml) {
        std::pair<std::string, std::string> device_ids;
        std::pair<std::string, std::string> device_source_interfaces;
        Connection_status                   connection_status;

        for (const auto &link_property_yaml : link_yaml) {
            std::string property = link_property_yaml.first.as<std::string>();
            YAML::Node  value    = link_property_yaml.second;

            if (property == "device_ids") {
                auto iterator     = value.begin();
                device_ids.first  = (*iterator++).as<std::string>();
                device_ids.second = (*iterator++).as<std::string>();
            } else if (property == "device_source_interfaces") {
                auto iterator = value.begin();
                device_source_interfaces.first =
                    (*iterator++).as<std::string>();
                device_source_interfaces.second =
                    (*iterator++).as<std::string>();

            } else if (property == "connection_status") {
                connection_status =
                    value.as<std::string>() == "active" ? ACTIVE : FAILED;
            } else {
                throwInvalidKey(property, link_property_yaml.first);
            }
        }

        Device *device1;
        Device *device2;
        for (const auto &device : *devices_ptr) {
            if (device->ID == device_ids.first) {
                device1 = device;
            }  // do not "else if" this to account for the case in which
               // ID1==ID2
            if (device->ID == device_ids.second) {
                device2 = device;
            }
        }

        NetworkCard *networkCard1;
        NetworkCard *networkCard2;
        networkCard1 = device1->getNetworkCardByInterfaceOrNull(
            device_source_interfaces.first);
        networkCard2 = device2->getNetworkCardByInterfaceOrNull(
            device_source_interfaces.second);

        std::shared_ptr<Link> link(new Link(connection_status));
        networkCard1->connect(link);
        networkCard2->connect(link);
    }
}
