#include "Server.h"

#include <unistd.h>

#include <cstdio>
#include <filesystem>
#include <new>
#include <string>

#include "../entities/Device.h"
#include "../entities/EndPoint.h"
#include "../entities/Link.h"
#include "../entities/NetworkCard.h"
#include "../entities/Router.h"
#include "../logger/Logger.h"
#include "../utils/Filesystem.h"
#include "IpAddress.h"
#include "MacAddress.h"
#include "pistache/http_defs.h"
#include "pistache/http_header.h"
#include "pistache/http_headers.h"
#include "pistache/mime.h"
#include "pistache/tcp.h"
#include "rapidjson/allocators.h"
#include "rapidjson/encodings.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"


void ApiEndpoint::init(size_t thr, std::vector<Device *> *devicesMain) {
    auto opts = Pistache::Http::Endpoint::options().flags(
        Pistache::Tcp::Options::ReuseAddr);
    opts = Pistache::Http::Endpoint::options().threads(static_cast<int>(thr));
    httpEndpoint->init(opts);
    devices = devicesMain;
    initDoc();
    setupRoutes();
}

void ApiEndpoint::start(volatile sig_atomic_t *stop) {
    L_VERBOSE("Server", "START");
    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serveThreaded();
    while (!*stop) {
        sleep(1);  // TODO change from polling to wait
    }
    httpEndpoint->shutdown();
}

void ApiEndpoint::setupRoutes() {
    L_DEBUG("Server", "Setting up routes");

    Pistache::Rest::Routes::Get(
        router, "/", Pistache::Rest::Routes::bind(&ApiEndpoint::index, this));
    Pistache::Rest::Routes::Get(
        router,
        "/showGUI",
        Pistache::Rest::Routes::bind(&ApiEndpoint::showGUI, this));
    Pistache::Rest::Routes::Get(
        router,
        "/getNetwork",
        Pistache::Rest::Routes::bind(&ApiEndpoint::getNetwork, this));
    Pistache::Rest::Routes::Get(
        router,
        "/getNetZoomCharts",
        Pistache::Rest::Routes::bind(&ApiEndpoint::getNetZoomCharts, this));
    Pistache::Rest::Routes::Post(
        router,
        "/breakLink",
        Pistache::Rest::Routes::bind(&ApiEndpoint::breakLink, this));

    Pistache::Rest::Routes::Post(
        router,
        "/addNode",
        Pistache::Rest::Routes::bind(&ApiEndpoint::addNode, this));

    Pistache::Rest::Routes::Post(
        router,
        "/removeNode",
        Pistache::Rest::Routes::bind(&ApiEndpoint::removeNode, this));

    Pistache::Rest::Routes::Post(
        router,
        "/addLink",
        Pistache::Rest::Routes::bind(&ApiEndpoint::addLink, this));

    // Routes for WebPage content
    Pistache::Rest::Routes::Get(
        router,
        "/showGUI/add-icon.png",
        Pistache::Rest::Routes::bind(&ApiEndpoint::getAddIcon, this));
    Pistache::Rest::Routes::Get(
        router,
        "/showGUI/endpoint-icon.png",
        Pistache::Rest::Routes::bind(&ApiEndpoint::getEndpointIcon, this));
    Pistache::Rest::Routes::Get(
        router,
        "/showGUI/packet-icon.png",
        Pistache::Rest::Routes::bind(&ApiEndpoint::getPacketIcon, this));
    Pistache::Rest::Routes::Get(
        router,
        "/showGUI/router-icon.png",
        Pistache::Rest::Routes::bind(&ApiEndpoint::getRouterIcon, this));
    Pistache::Rest::Routes::Get(
        router,
        "/showGUI/node-events.js",
        Pistache::Rest::Routes::bind(&ApiEndpoint::getNodeEvents, this));
    Pistache::Rest::Routes::Get(
        router,
        "/showGUI/main.css",
        Pistache::Rest::Routes::bind(&ApiEndpoint::getMainCSS, this));
}

void ApiEndpoint::initDoc() {
    // Create JSON with objects

    rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

    doc.SetObject();

    rapidjson::Value endpoints(rapidjson::kArrayType);
    rapidjson::Value routers(rapidjson::kArrayType);
    rapidjson::Value links(rapidjson::kArrayType);

    doc.AddMember("endpoints", endpoints, allocator);
    doc.AddMember("routers", routers, allocator);
    doc.AddMember("links", links, allocator);

    for (auto device : *devices) {
        rapidjson::Value ob(rapidjson::kObjectType);
        rapidjson::Value id;
        rapidjson::Value gateway;
        rapidjson::Value asNumber;
        rapidjson::Value networkCards(rapidjson::kArrayType);

        id.SetString(device->ID.c_str(), device->ID.length(), allocator);
        ob.AddMember("ID", id, allocator);

        gateway.SetString(device->defaultGateway.toString().c_str(),
                          device->defaultGateway.toString().length(),
                          allocator);
        ob.AddMember("defaultGateway", gateway, allocator);


        for (auto net : *device->networkCards) {
            L_DEBUG("Server", "Handling device " + net->owner->ID);
            L_DEBUG("Server", "NET interface : " + net->netInterface);

            rapidjson::Value netCard(rapidjson::kObjectType);
            rapidjson::Value interface;
            rapidjson::Value IP;
            rapidjson::Value netmask;

            interface.SetString(net->netInterface.c_str(),
                                net->netInterface.length(),
                                allocator);
            IP.SetString(net->IP.toString().c_str(),
                         net->IP.toString().length(),
                         allocator);
            netmask.SetString(net->netmask.toString().c_str(),
                              net->netmask.toString().length(),
                              allocator);

            netCard.AddMember("interface", interface, allocator);
            netCard.AddMember("IP", IP, allocator);
            netCard.AddMember("netmask", netmask, allocator);
            networkCards.PushBack(netCard, allocator);

            if (!(net->link == nullptr)) {
                rapidjson::Value link(rapidjson::kObjectType),
                    link_reverse(rapidjson::kObjectType);
                rapidjson::Value dev1, dev2, interface1, interface2, con_status;

                dev1.SetString(
                    net->owner->ID.c_str(), net->owner->ID.length(), allocator);
                interface1.SetString(net->netInterface.c_str(),
                                     net->netInterface.length(),
                                     allocator);
                con_status.SetString(
                    net->link->getConnectionStatusString().c_str(),
                    net->link->getConnectionStatusString().length(),
                    allocator);

                NetworkCard *net_dev2 =
                    net->link->getPeerNetworkCardOrNull(net);

                dev2.SetString(net_dev2->owner->ID.c_str(),
                               net_dev2->owner->ID.length(),
                               allocator);
                interface2.SetString(net_dev2->netInterface.c_str(),
                                     net_dev2->netInterface.length(),
                                     allocator);

                // Link is using copies of the values, Link_reverse is consuming
                // the values.

                link.AddMember(
                    "from", rapidjson::Value(dev1, allocator), allocator);
                link.AddMember(
                    "to", rapidjson::Value(dev2, allocator), allocator);
                link.AddMember("from_interface",
                               rapidjson::Value(interface1, allocator),
                               allocator);
                link.AddMember("to_interface",
                               rapidjson::Value(interface2, allocator),
                               allocator);
                link.AddMember("con_status",
                               rapidjson::Value(con_status, allocator),
                               allocator);

                link_reverse.AddMember("from", dev2, allocator);
                link_reverse.AddMember("to", dev1, allocator);
                link_reverse.AddMember("from_interface", interface2, allocator);
                link_reverse.AddMember("to_interface", interface1, allocator);
                link_reverse.AddMember("con_status", con_status, allocator);

                if (doc["links"].Empty()) {
                    doc["links"].PushBack(link, allocator);
                    L_DEBUG("Server", "Pushed back empty links");
                } else {
                    bool exists = false;
                    for (auto &l : doc["links"].GetArray()) {
                        if (l == link || l == link_reverse) {
                            L_DEBUG("Server", "Link :" + to_string(l == link));
                            L_DEBUG("Server",
                                    "Reverse Link :" +
                                        to_string(l == link_reverse));

                            exists = true;
                        }
                    }
                    if (!exists) {
                        doc["links"].PushBack(link, allocator);
                        L_DEBUG("Server", "Pushed back non existing link");
                    }
                }
            }
        }

        if (auto *x = dynamic_cast<Router *>(device)) {
            asNumber.SetInt(x->AS_number);
            ob.AddMember("asNumber", asNumber, allocator);

            ob.AddMember("networkCards", networkCards, allocator);

            doc["routers"].PushBack(ob, allocator);

        } else if (auto *x = dynamic_cast<EndPoint *>(device)) {
            ob.AddMember("networkCards", networkCards, allocator);

            doc["endpoints"].PushBack(ob, allocator);
        }
    }

    L_DEBUG("Server", "Network object created");
}

void ApiEndpoint::index(const Pistache::Rest::Request &request,
                        Pistache::Http::ResponseWriter response) {
    rapidjson::StringBuffer                          buf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);


    writer.StartObject();

    writer.Key("nodes");
    writer.StartArray();

    writer.StartObject();
    writer.Key("id");
    writer.String("R1");
    writer.Key("loaded");
    writer.Bool(true);
    writer.Key("style");
    writer.StartObject();
    writer.Key("fillColor");
    writer.String("rgba(236,46,46,0.8)");
    writer.Key("label");
    writer.String("Router1");
    writer.Key("image");
    writer.String("showGUI/router-icon.png");
    writer.EndObject();
    writer.Key("extra");
    writer.StartObject();
    writer.Key("AS_number");
    writer.String("AS_1");
    writer.Key("networkCard");
    writer.StartArray();
    writer.StartObject();
    writer.Key("interface");
    writer.String("eth0");
    writer.Key("IP");
    writer.String("90.36.25.1");
    writer.Key("netmask");
    writer.String("255.255.0.0");
    writer.EndObject();
    writer.StartObject();
    writer.Key("interface");
    writer.String("eth1");
    writer.Key("IP");
    writer.String("85.90.20.1");
    writer.Key("netmask");
    writer.String("255.255.0.0");
    writer.EndObject();
    writer.EndArray();
    writer.EndObject();
    writer.EndObject();

    writer.StartObject();
    writer.Key("id");
    writer.String("R2");
    writer.Key("loaded");
    writer.Bool(true);
    writer.Key("style");
    writer.StartObject();
    writer.Key("fillColor");
    writer.String("rgba(47,195,47,0.8)");
    writer.Key("label");
    writer.String("Router2");
    writer.Key("image");
    writer.String("showGUI/router-icon.png");
    writer.EndObject();
    writer.Key("extra");
    writer.StartObject();
    writer.Key("AS_number");
    writer.String("AS_2");
    writer.Key("default_gateway");
    writer.String("85.90.20.1");
    writer.Key("networkCard");
    writer.StartArray();
    writer.StartObject();
    writer.Key("interface");
    writer.String("eth0");
    writer.Key("IP");
    writer.String("26.37.3.2");
    writer.Key("netmask");
    writer.String("255.255.0.0");
    writer.EndObject();
    writer.StartObject();
    writer.Key("interface");
    writer.String("eth5");
    writer.Key("IP");
    writer.String("85.90.20.2");
    writer.Key("netmask");
    writer.String("255.255.0.0");
    writer.EndObject();
    writer.EndArray();
    writer.EndObject();
    writer.EndObject();

    writer.StartObject();
    writer.Key("id");
    writer.String("E1");
    writer.Key("loaded");
    writer.Bool(true);
    writer.Key("style");
    writer.StartObject();
    writer.Key("fillColor");
    writer.String("rgba(28,124,213,0.8)");
    writer.Key("label");
    writer.String("Endpoint1");
    writer.Key("image");
    writer.String("showGUI/endpoint-icon.png");
    writer.EndObject();
    writer.Key("extra");
    writer.StartObject();
    writer.Key("default_gateway");
    writer.String("90.36.25.1");
    writer.Key("networkCard");
    writer.StartArray();
    writer.StartObject();
    writer.Key("interface");
    writer.String("eth0");
    writer.Key("IP");
    writer.String("90.36.25.123");
    writer.Key("netmask");
    writer.String("255.255.0.0");
    writer.EndObject();
    writer.EndArray();
    writer.EndObject();
    writer.EndObject();

    writer.StartObject();
    writer.Key("id");
    writer.String("E2");
    writer.Key("loaded");
    writer.Bool(true);
    writer.Key("style");
    writer.StartObject();
    writer.Key("fillColor");
    writer.String("rgba(58,174,0,0.8)");
    writer.Key("label");
    writer.String("Endpoint2");
    writer.Key("image");
    writer.String("showGUI/endpoint-icon.png");
    writer.EndObject();
    writer.Key("extra");
    writer.StartObject();
    writer.Key("default_gateway");
    writer.String("26.37.3.2");
    writer.Key("networkCard");
    writer.StartArray();
    writer.StartObject();
    writer.Key("interface");
    writer.String("eth0");
    writer.Key("IP");
    writer.String("26.37.3.1");
    writer.Key("netmask");
    writer.String("255.255.0.0");
    writer.EndObject();
    writer.EndArray();
    writer.EndObject();
    writer.EndObject();

    writer.EndArray();

    writer.Key("links");
    writer.StartArray();

    writer.StartObject();
    writer.Key("id");
    writer.String("link_R1-E1");
    writer.Key("from");
    writer.String("R1");
    writer.Key("to");
    writer.String("E1");
    writer.Key("style");
    writer.StartObject();
    writer.Key("toDecoration");
    writer.String("arrow");
    writer.EndObject();
    writer.Key("extra");
    writer.StartObject();
    writer.Key("from_interface");
    writer.String("eth0");
    writer.Key("to_interface");
    writer.String("eth0");
    writer.EndObject();
    writer.EndObject();

    writer.StartObject();
    writer.Key("id");
    writer.String("link_R2-E2");
    writer.Key("from");
    writer.String("R2");
    writer.Key("to");
    writer.String("E2");
    writer.Key("style");
    writer.StartObject();
    writer.Key("toDecoration");
    writer.String("arrow");
    writer.EndObject();
    writer.Key("extra");
    writer.StartObject();
    writer.Key("from_interface");
    writer.String("eth0");
    writer.Key("to_interface");
    writer.String("eth0");
    writer.EndObject();
    writer.EndObject();

    writer.StartObject();
    writer.Key("id");
    writer.String("link_R1-R2");
    writer.Key("from");
    writer.String("R1");
    writer.Key("to");
    writer.String("R2");
    writer.Key("style");
    writer.StartObject();
    writer.Key("toDecoration");
    writer.String("arrow");
    writer.EndObject();
    writer.Key("extra");
    writer.StartObject();
    writer.Key("from_interface");
    writer.String("eth1");
    writer.Key("to_interface");
    writer.String("eth5");
    writer.EndObject();
    writer.EndObject();

    writer.EndArray();


    writer.EndObject();

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>(
        "*");
    response.send(Pistache::Http::Code::Ok, buf.GetString());
}


void ApiEndpoint::getNetwork(const Pistache::Rest::Request &request,
                             Pistache::Http::ResponseWriter response) {
    rapidjson::StringBuffer                          buf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);

    doc.Accept(writer);

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>(
        "*");
    response.send(Pistache::Http::Code::Ok, buf.GetString());
}

void ApiEndpoint::getNetZoomCharts(const Pistache::Rest::Request &request,
                                   Pistache::Http::ResponseWriter response) {
    using namespace rapidjson;

    StringBuffer                          buf;
    PrettyWriter<rapidjson::StringBuffer> writer(buf);

    Document                 zoomDoc;
    Document::AllocatorType &allocator = zoomDoc.GetAllocator();

    zoomDoc.SetObject();

    Value nodes(kArrayType);
    Value links(kArrayType);

    zoomDoc.AddMember("nodes", nodes, allocator);
    zoomDoc.AddMember("links", links, allocator);

    L_DEBUG("Server", "Creating nodes form endpoints");

    for (auto &endpoint : doc["endpoints"].GetArray()) {
        Value node(kObjectType);
        Value image;
        Value loaded(true);
        Value style(kObjectType);
        Value extra(kObjectType);

        node.AddMember("id", Value(endpoint["ID"], allocator), allocator);
        node.AddMember("loaded", loaded, allocator);

        image.SetString("showGUI/endpoint-icon.png", allocator);
        style.AddMember("label", Value(endpoint["ID"], allocator), allocator);
        style.AddMember("image", image, allocator);

        Value netCards(endpoint["networkCards"], allocator);
        extra.AddMember("default_gateway",
                        Value(endpoint["defaultGateway"], allocator),
                        allocator);
        extra.AddMember("networkCard", netCards, allocator);

        node.AddMember("style", style, allocator);
        node.AddMember("extra", extra, allocator);

        zoomDoc["nodes"].PushBack(node, allocator);
    }

    L_DEBUG("Server", "Creating nodes form routers");

    for (auto &router : doc["routers"].GetArray()) {
        Value node(kObjectType);
        Value image;
        Value loaded(true);
        Value style(kObjectType);
        Value extra(kObjectType);

        node.AddMember("id", Value(router["ID"], allocator), allocator);
        node.AddMember("loaded", loaded, allocator);

        image.SetString("showGUI/router-icon.png", allocator);
        style.AddMember("label", Value(router["ID"], allocator), allocator);
        style.AddMember("image", image, allocator);

        extra.AddMember(
            "AS_number", Value(router["asNumber"], allocator), allocator);
        extra.AddMember("default_gateway",
                        Value(router["defaultGateway"], allocator),
                        allocator);
        extra.AddMember(
            "networkCard", Value(router["networkCards"], allocator), allocator);

        node.AddMember("style", style, allocator);
        node.AddMember("extra", extra, allocator);

        zoomDoc["nodes"].PushBack(node, allocator);
    }

    L_DEBUG("Server", "Creating links");

    for (auto &link_it : doc["links"].GetArray()) {
        Value link(kObjectType);
        Value id;
        Value loaded(true);
        Value style(kObjectType);
        Value extra(kObjectType);

        char buffer[20];
        int  len = sprintf(buffer,
                          "link_%s-%s",
                          link_it["from"].GetString(),
                          link_it["to"].GetString());

        id.SetString(buffer, len, allocator);

        link.AddMember("id", id, allocator);
        link.AddMember("from", Value(link_it["from"], allocator), allocator);
        link.AddMember("to", Value(link_it["to"], allocator), allocator);

        style.AddMember("toDecoration", Value("arrow", allocator), allocator);
        style.AddMember("fromDecoration", Value("arrow", allocator), allocator);

        extra.AddMember("from_interface",
                        Value(link_it["from_interface"], allocator),
                        allocator);
        extra.AddMember("to_interface",
                        Value(link_it["to_interface"], allocator),
                        allocator);

        link.AddMember("style", style, allocator);
        link.AddMember("extra", extra, allocator);

        zoomDoc["links"].PushBack(link, allocator);
    }


    zoomDoc.Accept(writer);

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>(
        "*");
    response.send(Pistache::Http::Code::Ok, buf.GetString());
}

void ApiEndpoint::showGUI(const Pistache::Rest::Request &request,
                          Pistache::Http::ResponseWriter response) {
    if (request.method() == Pistache::Http::Method::Get) {
        Pistache::Http::serveFile(response,
                                  getExecutableParentPath().string() +
                                      "/static_files/bgp-visualization.html");
    }
}

// API to retrive static files
void ApiEndpoint::getAddIcon(const Pistache::Rest::Request &request,
                             Pistache::Http::ResponseWriter response) {
    if (request.method() == Pistache::Http::Method::Get) {
        Pistache::Http::serveFile(response,
                                  getExecutableParentPath().string() +
                                      "/static_files/img/add-icon.png");
    }
}

void ApiEndpoint::getEndpointIcon(const Pistache::Rest::Request &request,
                                  Pistache::Http::ResponseWriter response) {
    if (request.method() == Pistache::Http::Method::Get) {
        Pistache::Http::serveFile(response,
                                  getExecutableParentPath().string() +
                                      "/static_files/img/endpoint-icon.png");
    }
}

void ApiEndpoint::getPacketIcon(const Pistache::Rest::Request &request,
                                Pistache::Http::ResponseWriter response) {
    if (request.method() == Pistache::Http::Method::Get) {
        Pistache::Http::serveFile(response,
                                  getExecutableParentPath().string() +
                                      "/static_files/img/packet-icon.png");
    }
}

void ApiEndpoint::getRouterIcon(const Pistache::Rest::Request &request,
                                Pistache::Http::ResponseWriter response) {
    if (request.method() == Pistache::Http::Method::Get) {
        Pistache::Http::serveFile(response,
                                  getExecutableParentPath().string() +
                                      "/static_files/img/router-icon.png");
    }
}

void ApiEndpoint::getNodeEvents(const Pistache::Rest::Request &request,
                                Pistache::Http::ResponseWriter response) {
    if (request.method() == Pistache::Http::Method::Get) {
        Pistache::Http::serveFile(response,
                                  getExecutableParentPath().string() +
                                      "/static_files/js/node-events.js");
    }
}
void ApiEndpoint::getMainCSS(const Pistache::Rest::Request &request,
                             Pistache::Http::ResponseWriter response) {
    if (request.method() == Pistache::Http::Method::Get) {
        Pistache::Http::serveFile(
            response,
            getExecutableParentPath().string() + "/static_files/css/main.css");
    }
}


// API to change Network status

void ApiEndpoint::addNode(const Pistache::Rest::Request &request,
                          Pistache::Http::ResponseWriter response) {
    // TODO: add logic to add new node to devices array
    rapidjson::StringBuffer                          buf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);

    auto body = request.body();

    rapidjson::Document postDoc;

    L_DEBUG("Server", body);

    postDoc.Parse(body.c_str());

    if (!postDoc.HasParseError() && postDoc.HasMember("id") &&
        postDoc.HasMember("gateway") && postDoc.HasMember("networkCards")) {
        rapidjson::Value device(rapidjson::kObjectType);
        rapidjson::Value networkCards(rapidjson::kArrayType);

        Router *                    router;
        EndPoint *                  endpoint;
        std::vector<NetworkCard *> *networkCardsVector =
            new std::vector<NetworkCard *>();

        std::string      ID = postDoc.FindMember("id")->value.GetString();
        rapidjson::Value id;
        id.SetString(ID.c_str(), ID.length(), doc.GetAllocator());
        device.AddMember("ID", id, doc.GetAllocator());

        pcpp::IPv4Address defaultGateway(
            postDoc.FindMember("gateway")->value.GetString());
        rapidjson::Value gateway;
        gateway.SetString(defaultGateway.toString().c_str(),
                          defaultGateway.toString().length(),
                          doc.GetAllocator());
        device.AddMember("defaultGateway", gateway, doc.GetAllocator());


        if (postDoc.HasMember("asNumber")) {
            int AS_number = postDoc.FindMember("asNumber")->value.GetInt();
            rapidjson::Value asNumber;
            asNumber.SetInt(AS_number);
            device.AddMember("asNumber", asNumber, doc.GetAllocator());

            router = new Router(
                ID, AS_number, defaultGateway, std::vector<pcpp::IPv4Address>() /* TODO IMPORTANT ← placeholder put the read value*/);
        } else {
            endpoint = new EndPoint(ID, defaultGateway);
        }

        for (int i = 0; i < postDoc["networkCards"].Size(); i++) {
            if (postDoc["networkCards"][i].HasMember("interface") &&
                postDoc["networkCards"][i].HasMember("IP") &&
                postDoc["networkCards"][i].HasMember("netmask")) {
                rapidjson::Value networkCard(rapidjson::kObjectType);

                std::string interface = postDoc["networkCards"][i]
                                            .FindMember("interface")
                                            ->value.GetString();
                rapidjson::Value interface_val;
                interface_val.SetString(
                    interface.c_str(), interface.length(), doc.GetAllocator());
                networkCard.AddMember(
                    "interface", interface_val, doc.GetAllocator());


                pcpp::IPv4Address IP(postDoc["networkCards"][i]
                                         .FindMember("IP")
                                         ->value.GetString());
                rapidjson::Value  ip;
                ip.SetString(IP.toString().c_str(),
                             IP.toString().length(),
                             doc.GetAllocator());
                networkCard.AddMember("IP", ip, doc.GetAllocator());


                pcpp::IPv4Address netmask(postDoc["networkCards"][i]
                                              .FindMember("netmask")
                                              ->value.GetString());
                rapidjson::Value  netmask_val;
                netmask_val.SetString(netmask.toString().c_str(),
                                      netmask.toString().length(),
                                      doc.GetAllocator());
                networkCard.AddMember(
                    "netmask", netmask_val, doc.GetAllocator());


                networkCards.PushBack(networkCard, doc.GetAllocator());

                if (device.HasMember("asNumber")) {
                    NetworkCard *netCard = new NetworkCard(
                        interface, IP, netmask, pcpp::MacAddress::Zero, router);
                    networkCardsVector->push_back(netCard);
                } else {
                    NetworkCard *netCard =
                        new NetworkCard(interface,
                                        IP,
                                        netmask,
                                        pcpp::MacAddress::Zero,
                                        endpoint);
                    networkCardsVector->push_back(netCard);
                }
            }
        }

        device.AddMember("networkCards", networkCards, doc.GetAllocator());

        if (device.HasMember("asNumber")) {
            doc["routers"].PushBack(device, doc.GetAllocator());
            L_DEBUG(
                "Server",
                "Routers doc size: " + std::to_string(doc["routers"].Size()));
            router->addCards(networkCardsVector);
            devices->push_back(router);
            L_DEBUG("Server",
                    "Devices vector size: " + std::to_string(devices->size()));
            L_DEBUG("Server", "Router added. Router ID: " + router->ID);
        } else {
            doc["endpoints"].PushBack(device, doc.GetAllocator());
            L_DEBUG("Server",
                    "Endpoints doc size: " +
                        std::to_string(doc["endpoints"].Size()));
            endpoint->addCards(networkCardsVector);
            devices->push_back(endpoint);
            L_DEBUG("Server",
                    "Devices vector size: " + std::to_string(devices->size()));
            L_DEBUG("Server", "Endpoint added. Endpoint ID: " + endpoint->ID);
        }

        response.headers().add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json));
        response.headers()
            .add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
        response.send(Pistache::Http::Code::Ok, "New Node Added");

    } else {
        if (postDoc.HasParseError()) {
            L_WARNING("Server",
                      "Parsing Error(offset " +
                          to_string((unsigned)postDoc.GetErrorOffset()) +
                          "): " + to_string((postDoc.GetParseError())));
        } else {
            L_WARNING("Server", "JSON key values are wrong");
        }


        response.headers()
            .add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
        response.send(Pistache::Http::Code::Bad_Request,
                      "Wrong POST request!\nThe request needs to have the "
                      "following JSON format:\n{\n\t\"id\" : "
                      "\"device_ID\""
                      "\n\t\"gateway\" : \"device_gateway\""
                      "\n\t\"networkCards\" : \"device_array_networks_cards\""
                      "\n}");
    }
}

void ApiEndpoint::breakLink(const Pistache::Rest::Request &request,
                            Pistache::Http::ResponseWriter response) {
    rapidjson::StringBuffer                          buf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);

    auto body = request.body();

    rapidjson::Document postDoc;

    L_DEBUG("Server", body);

    postDoc.Parse(body.c_str());

    // Check if there are parsing errors or Typos into the Keys
    if (!postDoc.HasParseError() && postDoc.HasMember("from") &&
        postDoc.HasMember("to") && postDoc.HasMember("from_interface") &&
        postDoc.HasMember("to_interface")) {
        if (doc["links"].Empty()) {
            L_DEBUG("Server", "There are no links in the network");

            response.headers()
                .add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
            response.send(Pistache::Http::Code::Accepted,
                          "There are no links into the network!");
        } else {
            // Search for the link and change the value
            rapidjson::Value link(rapidjson::kObjectType),
                link_reverse(rapidjson::kObjectType);

            // Link is using copies of the values, Link_reverse is consuming the
            // values.

            link.AddMember("from",
                           rapidjson::Value(postDoc.FindMember("from")->value,
                                            postDoc.GetAllocator()),
                           postDoc.GetAllocator());
            link.AddMember("to",
                           rapidjson::Value(postDoc.FindMember("to")->value,
                                            postDoc.GetAllocator()),
                           postDoc.GetAllocator());
            link.AddMember(
                "from_interface",
                rapidjson::Value(postDoc.FindMember("from_interface")->value,
                                 postDoc.GetAllocator()),
                postDoc.GetAllocator());
            link.AddMember(
                "to_interface",
                rapidjson::Value(postDoc.FindMember("to_interface")->value,
                                 postDoc.GetAllocator()),
                postDoc.GetAllocator());
            link.AddMember("con_status", "active", postDoc.GetAllocator());

            link_reverse.AddMember("from",
                                   postDoc.FindMember("to")->value,
                                   postDoc.GetAllocator());
            link_reverse.AddMember("to",
                                   postDoc.FindMember("from")->value,
                                   postDoc.GetAllocator());
            link_reverse.AddMember("from_interface",
                                   postDoc.FindMember("to_interface")->value,
                                   postDoc.GetAllocator());
            link_reverse.AddMember("to_interface",
                                   postDoc.FindMember("from_interface")->value,
                                   postDoc.GetAllocator());
            link_reverse.AddMember(
                "con_status", "active", postDoc.GetAllocator());

            for (auto &l : doc["links"].GetArray()) {
                if ((l == link || l == link_reverse)) {
                    rapidjson::Value::MemberIterator it =
                        l.FindMember("con_status");
                    it->value.SetString("failed", postDoc.GetAllocator());
                    L_DEBUG("Server", "Value in the the RapidJSON doc changed");

                    for (auto dev : *devices) {
                        if (dev->ID.compare(link["from"].GetString())) {
                            for (auto net : *dev->networkCards) {
                                if (net->netInterface.compare(
                                        link["from_interface"].GetString())) {
                                    net->link->connection_status =
                                        FAILED;  // TODO Use setter when it will
                                                 // be ready on the class.
                                    L_INFO("Server",
                                           "Link disconnetcted. Device: " +
                                               dev->ID + " Interface: " +
                                               net->netInterface);
                                    L_DEBUG(
                                        "Server",
                                        "Link status: " +
                                            net->link
                                                ->getConnectionStatusString());
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        if (postDoc.HasParseError()) {
            L_WARNING("Server",
                      "Parsing Error(offset " +
                          to_string((unsigned)postDoc.GetErrorOffset()) +
                          "): " + to_string((postDoc.GetParseError())));
        } else {
            L_WARNING("Server", "JSON key values are wrong");
        }


        response.headers()
            .add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
        response.send(Pistache::Http::Code::Bad_Request,
                      "Wrong POST request!\nThe request needs to have the "
                      "following JSON format:\n{\n\t\"from\" : "
                      "\"device_x\""
                      "\n\t\"to\" : \"device_y\""
                      "\n\t\"from_interface\" : \"int_dev_x\""
                      "\n\t\"to_interface\" : \"int_dev_y\""
                      "\n}");
    }


    // TODO Modify default reply
    // postDoc.Accept(writer);


    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>(
        "*");
    response.send(Pistache::Http::Code::Ok, "Link removed Successfully!");
}

void ApiEndpoint::removeNode(const Pistache::Rest::Request &request,
                             Pistache::Http::ResponseWriter response) {
    // TODO: add logic to remove node from devices array
    rapidjson::StringBuffer                          buf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);

    auto body = request.body();

    rapidjson::Document postDoc;

    L_DEBUG("Server", body);

    postDoc.Parse(body.c_str());

    if (!postDoc.HasParseError() && postDoc.HasMember("id")) {
        std::string ID = postDoc["id"].GetString();
        if (ID[0] == 'R') {
            if (doc["routers"].Empty()) {
                L_DEBUG("Server", "There are no routers in the network");

                response.headers()
                    .add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
                response.send(Pistache::Http::Code::Accepted,
                              "There are no routers into the network!");
            } else {
                for (rapidjson::Value::ConstValueIterator itr =
                         doc["routers"].Begin();
                     itr != doc["routers"].End();) {
                    std::string currentID =
                        itr->FindMember("ID")->value.GetString();
                    if (currentID == ID) {
                        itr == doc["routers"].Erase(itr);
                        L_DEBUG("Server",
                                "Router removed from doc. Endpoint ID: " + ID);
                        L_DEBUG("Server",
                                "Routers doc size: " +
                                    std::to_string(doc["routers"].Size()));
                        for (auto it = devices->begin();
                             it != devices->end();) {
                            if ((*it)->ID == ID) {
                                it = devices->erase(it);
                                L_DEBUG("Server",
                                        "Devices vector size: " +
                                            std::to_string(devices->size()));
                            } else {
                                ++it;
                            }
                        }
                    } else {
                        ++itr;
                    }
                }
            }
        } else if (ID[0] == 'E') {
            if (doc["endpoints"].Empty()) {
                L_DEBUG("Server", "There are no endpoints in the network");

                response.headers()
                    .add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
                response.send(Pistache::Http::Code::Accepted,
                              "There are no endpoints into the network!");
            } else {
                for (rapidjson::Value::ConstValueIterator itr =
                         doc["endpoints"].Begin();
                     itr != doc["endpoints"].End();) {
                    std::string currentID =
                        itr->FindMember("ID")->value.GetString();
                    if (currentID == ID) {
                        itr == doc["endpoints"].Erase(itr);
                        L_DEBUG(
                            "Server",
                            "Endpoint removed from doc. Endpoint ID: " + ID);
                        L_DEBUG("Server",
                                "Endpoints doc size: " +
                                    std::to_string(doc["endpoints"].Size()));
                        for (auto it = devices->begin();
                             it != devices->end();) {
                            if ((*it)->ID == ID) {
                                it = devices->erase(it);
                                L_DEBUG("Server",
                                        "Devices vector size: " +
                                            std::to_string(devices->size()));
                            } else {
                                ++it;
                            }
                        }
                    } else {
                        ++itr;
                    }
                }
            }
        }
    } else {
        if (postDoc.HasParseError()) {
            L_WARNING("Server",
                      "Parsing Error(offset " +
                          to_string((unsigned)postDoc.GetErrorOffset()) +
                          "): " + to_string((postDoc.GetParseError())));
        } else {
            L_WARNING("Server", "JSON key values are wrong");
        }


        response.headers()
            .add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
        response.send(Pistache::Http::Code::Bad_Request,
                      "Wrong POST request!\nThe request needs to have the "
                      "following JSON format:\n{\n\t\"id\" : "
                      "\"device_id\""
                      "\n}");
    }


    // TODO Modify default reply
    // postDoc.Accept(writer);


    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>(
        "*");
    response.send(Pistache::Http::Code::Ok, "Node removed Successfully!");
}

void ApiEndpoint::addLink(const Pistache::Rest::Request &request,
                          Pistache::Http::ResponseWriter response) {
    rapidjson::StringBuffer                          buf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);

    auto body = request.body();

    rapidjson::Document postDoc;

    L_DEBUG("Server", body);

    postDoc.Parse(body.c_str());

    if (!postDoc.HasParseError() && postDoc.HasMember("from") &&
        postDoc.HasMember("to") && postDoc.HasMember("from_interface") &&
        postDoc.HasMember("to_interface")) {
        // Search for the link and change the value
        rapidjson::Value link(rapidjson::kObjectType),
            link_reverse(rapidjson::kObjectType);

        // Link is using copies of the values, Link_reverse is consuming the
        // values.

        link.AddMember("from",
                       rapidjson::Value(postDoc.FindMember("from")->value,
                                        postDoc.GetAllocator()),
                       postDoc.GetAllocator());
        link.AddMember("to",
                       rapidjson::Value(postDoc.FindMember("to")->value,
                                        postDoc.GetAllocator()),
                       postDoc.GetAllocator());
        link.AddMember(
            "from_interface",
            rapidjson::Value(postDoc.FindMember("from_interface")->value,
                             postDoc.GetAllocator()),
            postDoc.GetAllocator());
        link.AddMember(
            "to_interface",
            rapidjson::Value(postDoc.FindMember("to_interface")->value,
                             postDoc.GetAllocator()),
            postDoc.GetAllocator());
        link.AddMember("con_status", "active", postDoc.GetAllocator());

        link_reverse.AddMember(
            "from", postDoc.FindMember("to")->value, postDoc.GetAllocator());
        link_reverse.AddMember(
            "to", postDoc.FindMember("from")->value, postDoc.GetAllocator());
        link_reverse.AddMember("from_interface",
                               postDoc.FindMember("to_interface")->value,
                               postDoc.GetAllocator());
        link_reverse.AddMember("to_interface",
                               postDoc.FindMember("from_interface")->value,
                               postDoc.GetAllocator());
        link_reverse.AddMember("con_status", "active", postDoc.GetAllocator());

        if (doc["links"].Empty()) {
            doc["links"].PushBack(link, postDoc.GetAllocator());
            L_DEBUG("Server", "Pushed back empty links");
        } else {
            bool exists = false;
            for (auto &l : doc["links"].GetArray()) {
                if (l == link || l == link_reverse) {
                    L_DEBUG("Server", "Link :" + to_string(l == link));
                    L_DEBUG("Server",
                            "Reverse Link :" + to_string(l == link_reverse));

                    exists = true;
                }
            }
            if (!exists) {
                doc["links"].PushBack(link, postDoc.GetAllocator());
                L_DEBUG("Server", "Pushed back non existing link");
            }
        }

    } else {
        if (postDoc.HasParseError()) {
            L_WARNING("Server",
                      "Parsing Error(offset " +
                          to_string((unsigned)postDoc.GetErrorOffset()) +
                          "): " + to_string((postDoc.GetParseError())));
        } else {
            L_WARNING("Server", "JSON key values are wrong");
        }


        response.headers()
            .add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
        response.send(Pistache::Http::Code::Bad_Request,
                      "Wrong POST request!\nThe request needs to have the "
                      "following JSON format:\n{\n\t\"from\" : "
                      "\"device_x\""
                      "\n\t\"to\" : \"device_y\""
                      "\n\t\"from_interface\" : \"int_dev_x\""
                      "\n\t\"to_interface\" : \"int_dev_y\""
                      "\n}");
    }

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>(
        "*");
    response.send(Pistache::Http::Code::Ok, "Link added Successfully!");
}
