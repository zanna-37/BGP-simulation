/*
   Mathieu Stefani, 07 février 2016

   Example of a REST endpoint with routing
*/

#include "Server.h"


void ApiEndpoint::index(const Rest::Request&, Http::ResponseWriter response) {
    Document doc;

    // Create JSON with objects
    /*     doc.SetObject();

        Value v;
        v.SetInt(10);
        Value b(true);
        Value s;
        s.SetString("Ciccio", strlen("Ciccio"), doc.GetAllocator());

        doc.AddMember("Valore", v, doc.GetAllocator());
        doc.AddMember("Booleano", b, doc.GetAllocator());
        doc.AddMember("Sexy", s, doc.GetAllocator());
     */

    StringBuffer               buf;
    PrettyWriter<StringBuffer> writer(buf);
    /*     doc.Accept(writer);
     */

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


void ApiEndpoint::init(size_t thr = 2) {
    auto opts = Http::Endpoint::options().flags(Tcp::Options::ReuseAddr);
    opts = Http::Endpoint::options().threads(static_cast<int>(thr));
    httpEndpoint->init(opts);
    setupRoutes();
}

void ApiEndpoint::start() {
    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serve();
}

void ApiEndpoint::setupRoutes() {
    using namespace Rest;

    Routes::Get(router, "/", Routes::bind(&ApiEndpoint::index, this));
}

void ApiEndpoint::getNetwork(const Rest::Request&,
                             Http::ResponseWriter response) {}

void ApiEndpoint::getNodes(const Rest::Request&,
                           Http::ResponseWriter response) {}

void ApiEndpoint::getLinks(const Rest::Request&,
                           Http::ResponseWriter response) {}

void ApiEndpoint::setLink(const Rest::Request&,
                          Http::ResponseWriter response) {}

void ApiEndpoint::removeLink(const Rest::Request&,
                             Http::ResponseWriter response) {}

void ApiEndpoint::removeNode(const Rest::Request&,
                             Http::ResponseWriter response) {}

void ApiEndpoint::getPackets(const Rest::Request&,
                             Http::ResponseWriter response) {}

void ApiEndpoint::setReady(const Rest::Request&,
                           Http::ResponseWriter response) {}
