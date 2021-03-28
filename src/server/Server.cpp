/*
   Mathieu Stefani, 07 février 2016

   Example of a REST endpoint with routing
*/

#include <algorithm>

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace Pistache;
using namespace rapidjson;

void printCookies(const Http::Request& req) {
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

namespace Generic {

void handleReady(const Rest::Request&, Http::ResponseWriter response) {
    Document doc;
    doc.SetObject();

    Value v;
    v.SetInt(10);
    Value b(true);
    Value s;
    s.SetString("Ciccio", strlen("Ciccio"), doc.GetAllocator());

    doc.AddMember("Valore", v, doc.GetAllocator());
    doc.AddMember("Booleano", b, doc.GetAllocator());
    doc.AddMember("Sexy", s, doc.GetAllocator());


    StringBuffer buf;
    Writer<StringBuffer> writer(buf);
    doc.Accept(writer);

    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, buf.GetString());
}

}

class StatsEndpoint {
public:
    explicit StatsEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { }

    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options()
            .threads(static_cast<int>(thr));
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

  
    using Lock = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock metricsLock;

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main(int argc, char *argv[]) {
    Port port(9080);

    int thr = 2;

    if (argc >= 2) {
        port = static_cast<uint16_t>(std::stol(argv[1]));

        if (argc == 3)
            thr = std::stoi(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

    StatsEndpoint stats(addr);

    stats.init(thr);
    stats.start();
}
