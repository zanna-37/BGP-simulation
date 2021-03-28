/*
   Mathieu Stefani, 07 février 2016

   Example of a REST endpoint with routing
*/

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

#include <algorithm>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

using namespace std;
using namespace Pistache;
using namespace rapidjson;

void printCookies(const Http::Request& req) {
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c : cookies) {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

namespace Generic {

void handleReady(const Rest::Request&, Http::ResponseWriter response) {
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

    StringBuffer         buf;
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

}  // namespace Generic

class StatsEndpoint {
   public:
    explicit StatsEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options().threads(static_cast<int>(thr));
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

   private:
    void setupRoutes() {
        using namespace Rest;

        Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
    }


    using Lock  = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock metricsLock;

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router                    router;
};

int main(int argc, char* argv[]) {
    Port port(9080);

    int thr = 2;

    if (argc >= 2) {
        port = static_cast<uint16_t>(std::stol(argv[1]));

        if (argc == 3) thr = std::stoi(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

    StatsEndpoint stats(addr);

    stats.init(thr);
    stats.start();
}
