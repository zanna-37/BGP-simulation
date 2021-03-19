
#include "../../entities/Device.h"
#include "Parser.h"


void parseAndAddBuiltLinks(const YAML::Node &links_yaml,
                           vector<Device *> *devices_ptr) {
    assertNodeType(links_yaml, YAML::NodeType::value::Sequence);

    for (const auto &link_yaml : links_yaml) {
        pair<Device *, Device *> devices;
        pair<string, string>     device_source_interfaces;
        Connection_status        connection_status;

        for (const auto &link_property_yaml : link_yaml) {
            string     property = link_property_yaml.first.as<std::string>();
            YAML::Node value    = link_property_yaml.second;

            if (property == "device_ids") {
                auto   iterator = value.begin();
                string ID1      = (*iterator++).as<std::string>();
                string ID2      = (*iterator++).as<std::string>();
                for (const auto &device : *devices_ptr) {
                    if (device->ID == ID1) {
                        devices.first = device;
                    }  // do not "else if" this to account for the case in which
                       // ID1==ID2
                    if (device->ID == ID2) {
                        devices.second = device;
                    }
                }
            } else if (property == "device_source_interfaces") {
                auto iterator = value.begin();
                device_source_interfaces.first =
                    (*iterator++).as<std::string>();
                device_source_interfaces.second =
                    (*iterator++).as<std::string>();

            } else if (property == "connection_status") {
                connection_status =
                    value.as<std::string>() == "active" ? active : failed;
            } else {
                throwInvalidKey(property, link_property_yaml.first);
            }
        }

        Link *link =
            new Link(devices, device_source_interfaces, connection_status);

        devices.first->links.push_back(link);
        // to avoid duplication when devices.first == devices.second
        if (devices.first != devices.second) {
            devices.second->links.push_back(link);
        }
    }
}