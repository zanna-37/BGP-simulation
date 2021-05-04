#include "Server.h"


void ApiEndpoint::init(size_t thr, vector<Device *> *devicesMain) {
    auto opts = Http::Endpoint::options().flags(Tcp::Options::ReuseAddr);
    opts      = Http::Endpoint::options().threads(static_cast<int>(thr));
    httpEndpoint->init(opts);
    devices = devicesMain;
    initDoc();
    setupRoutes();
}

void ApiEndpoint::start() {
    L_VERBOSE("Server", "START");
    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serve();
}

void ApiEndpoint::setupRoutes() {
    L_DEBUG("Server", "Setting up routes");
    using namespace Rest;

    Routes::Get(router, "/", Routes::bind(&ApiEndpoint::index, this));
    Routes::Get(router, "/showGUI", Routes::bind(&ApiEndpoint::showGUI, this));
    Routes::Get(
        router, "/getNetwork", Routes::bind(&ApiEndpoint::getNetwork, this));
    Routes::Post(
        router, "/breakLink", Routes::bind(&ApiEndpoint::breakLink, this));

    Routes::Post(router, "/addNode", Routes::bind(&ApiEndpoint::addNode, this));

    Routes::Post(
        router, "/removeNode", Routes::bind(&ApiEndpoint::removeNode, this));

    // Routes for WebPage content
    Routes::Get(router,
                "/showGUI/add-icon.png",
                Routes::bind(&ApiEndpoint::getAddIcon, this));
    Routes::Get(router,
                "/showGUI/endpoint-icon.png",
                Routes::bind(&ApiEndpoint::getEndpointIcon, this));
    Routes::Get(router,
                "/showGUI/packet-icon.png",
                Routes::bind(&ApiEndpoint::getPacketIcon, this));
    Routes::Get(router,
                "/showGUI/router-icon.png",
                Routes::bind(&ApiEndpoint::getRouterIcon, this));
    Routes::Get(router,
                "/showGUI/node-events.js",
                Routes::bind(&ApiEndpoint::getNodeEvents, this));
    Routes::Get(router,
                "/showGUI/main.css",
                Routes::bind(&ApiEndpoint::getMainCSS, this));
}

void ApiEndpoint::initDoc() {
    // Create JSON with objects

    Document::AllocatorType &allocator = doc.GetAllocator();

    doc.SetObject();

    Value endpoints(kArrayType);
    Value routers(kArrayType);
    Value links(kArrayType);

    doc.AddMember("endpoints", endpoints, allocator);
    doc.AddMember("routers", routers, allocator);
    doc.AddMember("links", links, allocator);

    for (auto device : *devices) {
        Value ob(kObjectType);
        Value id;
        Value gateway;
        Value asNumber;
        Value networkCards(kArrayType);

        id.SetString(device->ID.c_str(), device->ID.length(), allocator);
        ob.AddMember("ID", id, allocator);

        gateway.SetString(device->defaultGateway.toString().c_str(),
                          device->defaultGateway.toString().length(),
                          allocator);
        ob.AddMember("defaultGateway", gateway, allocator);


        for (auto net : *device->networkCards) {
            L_DEBUG("Server", "Handling device " + net->owner->ID);
            L_DEBUG("Server", "NET interface : " + net->netInterface);

            Value netCard(kObjectType);
            Value interface;
            Value IP;
            Value netmask;

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
                Value link(kObjectType), link_reverse(kObjectType);
                Value dev1, dev2, interface1, interface2, con_status;

                dev1.SetString(
                    net->owner->ID.c_str(), net->owner->ID.length(), allocator);
                interface1.SetString(net->netInterface.c_str(),
                                     net->netInterface.length(),
                                     allocator);
                con_status.SetString(
                    net->link->getConnecionStatusString().c_str(),
                    net->link->getConnecionStatusString().length(),
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

                link.AddMember("from", Value(dev1, allocator), allocator);
                link.AddMember("to", Value(dev2, allocator), allocator);
                link.AddMember(
                    "from_interface", Value(interface1, allocator), allocator);
                link.AddMember(
                    "to_interface", Value(interface2, allocator), allocator);
                link.AddMember(
                    "con_status", Value(con_status, allocator), allocator);

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
            asNumber.SetString(
                x->AS_number.c_str(), x->AS_number.length(), allocator);
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

void ApiEndpoint::index(const Rest::Request &request,
                        Http::ResponseWriter response) {
    StringBuffer               buf;
    PrettyWriter<StringBuffer> writer(buf);


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
    writer.Int(0);
    writer.Key("to_interface");
    writer.Int(0);
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
    writer.Int(0);
    writer.Key("to_interface");
    writer.Int(0);
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
    writer.Int(1);
    writer.Key("to_interface");
    writer.Int(5);
    writer.EndObject();
    writer.EndObject();

    writer.EndArray();


    writer.EndObject();

    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
    response.send(Http::Code::Ok, buf.GetString());
}


void ApiEndpoint::getNetwork(const Rest::Request &request,
                             Http::ResponseWriter response) {
    StringBuffer               buf;
    PrettyWriter<StringBuffer> writer(buf);

    doc.Accept(writer);

    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
    response.send(Http::Code::Ok, buf.GetString());
}

void ApiEndpoint::showGUI(const Rest::Request &request,
                          Http::ResponseWriter response) {
    if (request.method() == Http::Method::Get) {
        Http::serveFile(response,
                        "src/net-visualization/bgp-visualization.html");
    }
}

// API to retrive static files
void ApiEndpoint::getAddIcon(const Rest::Request &request,
                             Http::ResponseWriter response) {
    if (request.method() == Http::Method::Get) {
        Http::serveFile(response, "src/net-visualization/img/add-icon.png");
    }
}

void ApiEndpoint::getEndpointIcon(const Rest::Request &request,
                                  Http::ResponseWriter response) {
    if (request.method() == Http::Method::Get) {
        Http::serveFile(response,
                        "src/net-visualization/img/endpoint-icon.png");
    }
}

void ApiEndpoint::getPacketIcon(const Rest::Request &request,
                                Http::ResponseWriter response) {
    if (request.method() == Http::Method::Get) {
        Http::serveFile(response, "src/net-visualization/img/packet-icon.png");
    }
}

void ApiEndpoint::getRouterIcon(const Rest::Request &request,
                                Http::ResponseWriter response) {
    if (request.method() == Http::Method::Get) {
        Http::serveFile(response, "src/net-visualization/img/router-icon.png");
    }
}

void ApiEndpoint::getNodeEvents(const Rest::Request &request,
                                Http::ResponseWriter response) {
    if (request.method() == Http::Method::Get) {
        Http::serveFile(response, "src/net-visualization/js/node-events.js");
    }
}
void ApiEndpoint::getMainCSS(const Rest::Request &request,
                             Http::ResponseWriter response) {
    if (request.method() == Http::Method::Get) {
        Http::serveFile(response, "src/net-visualization/css/main.css");
    }
}


// API to change Network status

void ApiEndpoint::addNode(const Rest::Request &request,
                          Http::ResponseWriter response) {
    // TODO: add logic to add new node to devices array
    StringBuffer               buf;
    PrettyWriter<StringBuffer> writer(buf);

    auto body = request.body();

    Document postDoc;

    L_DEBUG("Server", body);

    postDoc.Parse(body.c_str());

    if (!postDoc.HasParseError() && postDoc.HasMember("id") &&
        postDoc.HasMember("gateway") && postDoc.HasMember("networkCards")) {
        Value device(kObjectType);
        Value networkCards(kArrayType);

        device.AddMember(
            "id",
            Value(postDoc.FindMember("id")->value, postDoc.GetAllocator()),
            postDoc.GetAllocator());

        device.AddMember(
            "gateway",
            Value(postDoc.FindMember("gateway")->value, postDoc.GetAllocator()),
            postDoc.GetAllocator());

        if (postDoc.HasMember("asNumber") &&
            postDoc.FindMember("asNumber")->value.GetString() != "undefined") {
            device.AddMember("asNumber",
                             Value(postDoc.FindMember("asNumber")->value,
                                   postDoc.GetAllocator()),
                             postDoc.GetAllocator());
        }

        for (int i = 0; i < postDoc["networkCards"].Size(); i++) {
            if (postDoc["networkCards"][i].HasMember("interface") &&
                postDoc["networkCards"][i].HasMember("IP") &&
                postDoc["networkCards"][i].HasMember("netmask")) {
                Value networkCard(kObjectType);

                string            interface;
                pcpp::IPv4Address IP;
                pcpp::IPv4Address netmask;

                networkCard.AddMember("interface",
                                      Value(postDoc["networkCards"][i]
                                                .FindMember("interface")
                                                ->value,
                                            postDoc.GetAllocator()),
                                      postDoc.GetAllocator());

                networkCard.AddMember(
                    "IP",
                    Value(postDoc["networkCards"][i].FindMember("IP")->value,
                          postDoc.GetAllocator()),
                    postDoc.GetAllocator());

                networkCard.AddMember(
                    "netmask",
                    Value(
                        postDoc["networkCards"][i].FindMember("netmask")->value,
                        postDoc.GetAllocator()),
                    postDoc.GetAllocator());


                networkCards.PushBack(networkCard, postDoc.GetAllocator());
            }
        }

        device.AddMember("networkCards", networkCards, postDoc.GetAllocator());

        if (device.HasMember("asNumber")) {
            doc["routers"].PushBack(device, postDoc.GetAllocator());
        } else {
            doc["endpoints"].PushBack(device, postDoc.GetAllocator());
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


        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.send(Http::Code::Bad_Request,
                      "Wrong POST request!\nThe request needs to have the "
                      "following JSON format:\n{\n\t\"id\" : "
                      "\"device_ID\""
                      "\n\t\"gateway\" : \"device_gateway\""
                      "\n\t\"networkCards\" : \"device_array_networks_cards\""
                      "\n}");
    }

    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
    response.send(Http::Code::Ok, "New Node Added");
}

void ApiEndpoint::breakLink(const Rest::Request &request,
                            Http::ResponseWriter response) {
    StringBuffer               buf;
    PrettyWriter<StringBuffer> writer(buf);

    auto body = request.body();

    Document postDoc;

    L_DEBUG("Server", body);

    postDoc.Parse(body.c_str());

    // Check if there are parsing errors or Typos into the Keys
    if (!postDoc.HasParseError() && postDoc.HasMember("from") &&
        postDoc.HasMember("to") && postDoc.HasMember("from_interface") &&
        postDoc.HasMember("to_interface")) {
        if (doc["links"].Empty()) {
            L_DEBUG("Server", "There are no links in the network");

            response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
            response.send(Http::Code::Accepted,
                          "There are no links into the network!");
        } else {
            // Search for the link and change the value
            Value link(kObjectType), link_reverse(kObjectType);

            // Link is using copies of the values, Link_reverse is consuming the
            // values.

            link.AddMember("from",
                           Value(postDoc.FindMember("from")->value,
                                 postDoc.GetAllocator()),
                           postDoc.GetAllocator());
            link.AddMember(
                "to",
                Value(postDoc.FindMember("to")->value, postDoc.GetAllocator()),
                postDoc.GetAllocator());
            link.AddMember("from_interface",
                           Value(postDoc.FindMember("from_interface")->value,
                                 postDoc.GetAllocator()),
                           postDoc.GetAllocator());
            link.AddMember("to_interface",
                           Value(postDoc.FindMember("to_interface")->value,
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
                    Value::MemberIterator it = l.FindMember("con_status");
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
                                    L_DEBUG("Server",
                                            "Link status: " +
                                                net->link->connection_status);
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


        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.send(Http::Code::Bad_Request,
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


    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
    response.send(Http::Code::Ok, "Link removed Successfully!");
}

void ApiEndpoint::removeNode(const Rest::Request &request,
                             Http::ResponseWriter response) {
    // TODO: add logic to remove node from devices array
    StringBuffer               buf;
    PrettyWriter<StringBuffer> writer(buf);

    auto body = request.body();

    Document postDoc;

    L_DEBUG("Server", body);

    postDoc.Parse(body.c_str());

    if (!postDoc.HasParseError() && postDoc.HasMember("id")) {
        std::string ID = postDoc["id"].GetString();
        if (ID[0] == 'R') {
            if (doc["routers"].Empty()) {
                L_DEBUG("Server", "There are no routers in the network");

                response.headers().add<Http::Header::AccessControlAllowOrigin>(
                    "*");
                response.send(Http::Code::Accepted,
                              "There are no routers into the network!");
            } else {
                for (rapidjson::Value::ConstValueIterator itr =
                         doc["routers"].GetArray().Begin();
                     itr != doc["routers"].GetArray().End();
                     ++itr) {
                    if (itr->HasMember(postDoc["id"])) {
                        doc["routers"].GetArray().Erase(itr);
                    }
                }
            }
        } else if (ID[0] == 'E') {
            if (doc["endpoints"].Empty()) {
                L_DEBUG("Server", "There are no endpoints in the network");

                response.headers().add<Http::Header::AccessControlAllowOrigin>(
                    "*");
                response.send(Http::Code::Accepted,
                              "There are no endpoints into the network!");
            } else {
                for (rapidjson::Value::ConstValueIterator itr =
                         doc["endpoints"].GetArray().Begin();
                     itr != doc["endpoints"].GetArray().End();
                     ++itr) {
                    if (itr->HasMember(postDoc["id"])) {
                        doc["endpoints"].GetArray().Erase(itr);
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


        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.send(Http::Code::Bad_Request,
                      "Wrong POST request!\nThe request needs to have the "
                      "following JSON format:\n{\n\t\"id\" : "
                      "\"device_id\""
                      "\n}");
    }


    // TODO Modify default reply
    // postDoc.Accept(writer);


    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
    response.send(Http::Code::Ok, "Node removed Successfully!");
}
