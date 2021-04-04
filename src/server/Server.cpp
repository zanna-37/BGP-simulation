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
    L_VERBOSE("Server","START");
    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serve();
}

void ApiEndpoint::setupRoutes() {
    L_DEBUG("Server","Setting up routes");
    using namespace Rest;

    Routes::Get(router, "/", Routes::bind(&ApiEndpoint::index, this));
    Routes::Get(
        router, "/getNetwork", Routes::bind(&ApiEndpoint::getNetwork, this));
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

            Value link(kObjectType), link_reverse(kObjectType);
            Value dev1, dev2, interface1, interface2, con_status;

            dev1.SetString(
                net->owner->ID.c_str(), net->owner->ID.length(), allocator);
            interface1.SetString(net->netInterface.c_str(),
                                 net->netInterface.length(),
                                 allocator);
            con_status.SetString(net->link->getConnecionStatusString().c_str(),
                                 net->link->getConnecionStatusString().length(),
                                 allocator);

            NetworkCard *net_dev2 = net->link->getPeerNetworkCardOrNull(net);

            dev2.SetString(net_dev2->owner->ID.c_str(),
                           net_dev2->owner->ID.length(),
                           allocator);
            interface2.SetString(net_dev2->netInterface.c_str(),
                                 net_dev2->netInterface.length(),
                                 allocator);

            link.AddMember("from", dev1, allocator);
            link.AddMember("to", dev2, allocator);
            link.AddMember("from_interface", interface1, allocator);
            link.AddMember("to_interface", interface2, allocator);
            link.AddMember("con_status", con_status, allocator);

            link_reverse.AddMember("from", dev2, allocator);
            link_reverse.AddMember("to", dev1, allocator);
            link_reverse.AddMember("from_interface", interface2, allocator);
            link_reverse.AddMember("to_interface", interface1, allocator);
            link_reverse.AddMember("con_status", con_status, allocator);

            if (doc["links"].Empty()) {
                doc["links"].PushBack(link, allocator);
                L_DEBUG("Server", "Pushed back empty links");
            } else {
                for (auto &l : doc["links"].GetArray()) {
                    if (!(l == link || l == link_reverse ||
                          link.IsNull())) {  // The null check is done to the
                                             // move() of Rapidjason
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

    L_DEBUG("Server","Network object created");
}

void ApiEndpoint::index(const Rest::Request & request, Http::ResponseWriter response) {
    StringBuffer               buf;
    PrettyWriter<StringBuffer> writer(buf);


    writer.StartObject();

    writer.Key("nodes");
    writer.StartArray();

    writer.StartObject();
    writer.Key("id");
    writer.String("n1");
    writer.Key("loaded");
    writer.Bool(true);
    writer.Key("style");
    writer.StartObject();
    writer.Key("fillColor");
    writer.String("rgba(236,46,46,0.8)");
    writer.Key("label");
    writer.String("Node1");
    writer.EndObject();
    writer.EndObject();

    writer.StartObject();
    writer.Key("id");
    writer.String("n2");
    writer.Key("loaded");
    writer.Bool(true);
    writer.Key("style");
    writer.StartObject();
    writer.Key("fillColor");
    writer.String("rgba(47,195,47,0.8)");
    writer.Key("label");
    writer.String("Node2");
    writer.EndObject();
    writer.EndObject();

    writer.StartObject();
    writer.Key("id");
    writer.String("n3");
    writer.Key("loaded");
    writer.Bool(true);
    writer.Key("style");
    writer.StartObject();
    writer.Key("fillColor");
    writer.String("rgba(28,124,213,0.8)");
    writer.Key("label");
    writer.String("Node3");
    writer.EndObject();
    writer.EndObject();

    writer.EndArray();

    writer.Key("links");
    writer.StartArray();

    writer.StartObject();
    writer.Key("id");
    writer.String("l1");
    writer.Key("from");
    writer.String("n1");
    writer.Key("to");
    writer.String("n2");
    writer.Key("style");
    writer.StartObject();
    writer.Key("fillColor");
    writer.String("rgba(236,46,46,1)");
    writer.Key("toDecoration");
    writer.String("arrow");
    writer.EndObject();
    writer.EndObject();

    writer.StartObject();
    writer.Key("id");
    writer.String("l2");
    writer.Key("from");
    writer.String("n2");
    writer.Key("to");
    writer.String("n3");
    writer.Key("style");
    writer.StartObject();
    writer.Key("fillColor");
    writer.String("rgba(47,195,47,1)");
    writer.Key("toDecoration");
    writer.String("arrow");
    writer.EndObject();
    writer.EndObject();

    writer.StartObject();
    writer.Key("id");
    writer.String("l3");
    writer.Key("from");
    writer.String("n3");
    writer.Key("to");
    writer.String("n1");
    writer.Key("style");
    writer.StartObject();
    writer.Key("fillColor");
    writer.String("rgba(28,124,213,1)");
    writer.Key("toDecoration");
    writer.String("arrow");
    writer.EndObject();
    writer.EndObject();

    writer.EndArray();


    writer.EndObject();

    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
    response.send(Http::Code::Ok, buf.GetString());
}


void ApiEndpoint::getNetwork(const Rest::Request & request,
                             Http::ResponseWriter response) {
    StringBuffer               buf;
    PrettyWriter<StringBuffer> writer(buf);

    doc.Accept(writer);

    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
    response.send(Http::Code::Ok, buf.GetString());
}

void ApiEndpoint::getNodes(const Rest::Request & request,
                           Http::ResponseWriter response) {}

void ApiEndpoint::getLinks(const Rest::Request & request,
                           Http::ResponseWriter response) {}

void ApiEndpoint::setLink(const Rest::Request & request,
                          Http::ResponseWriter response) {}

void ApiEndpoint::removeLink(const Rest::Request & request,
                             Http::ResponseWriter response) {

void ApiEndpoint::removeNode(const Rest::Request & request,
                             Http::ResponseWriter response) {}

void ApiEndpoint::getPackets(const Rest::Request & request,
                             Http::ResponseWriter response) {}

void ApiEndpoint::setReady(const Rest::Request & request,
                           Http::ResponseWriter response) {}
