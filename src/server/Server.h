#ifndef SERVER_H
#define SERVER_H

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

// #include <algorithm>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;
using namespace Pistache;
using namespace rapidjson;

class ApiEndpoint {
   public:
    explicit ApiEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    void init(size_t thr);

    void start();

    std::shared_ptr<Http::Endpoint> httpEndpoint;

   private:
    void setupRoutes();

    void getNetwork(const Rest::Request&, Http::ResponseWriter response);

    void getNodes(const Rest::Request&, Http::ResponseWriter response);

    void getLinks(const Rest::Request&, Http::ResponseWriter response);

    void setLink(const Rest::Request&, Http::ResponseWriter response);

    void removeLink(const Rest::Request&, Http::ResponseWriter response);

    void removeNode(const Rest::Request&, Http::ResponseWriter response);

    void getPackets(const Rest::Request&, Http::ResponseWriter response);

    void setReady(const Rest::Request&, Http::ResponseWriter response);

    void index(const Rest::Request&, Http::ResponseWriter response);

    using Lock  = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock metricsLock;

    
    Rest::Router router;
};

#endif