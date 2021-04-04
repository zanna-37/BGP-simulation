#ifndef SERVER_H
#define SERVER_H

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <string.h>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "../entities/EndPoint.h"
#include "../entities/Router.h"
#include "../logger/Logger.h"

using namespace std;
using namespace Pistache;
using namespace rapidjson;

class ApiEndpoint {
   public:
    explicit ApiEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    /**
     * @brief Initialize the number of wanted threads for the rest server
     *
     * @param thr Number of threads
     */
    void init(size_t thr, vector<Device *> *devicesMain);

    /**
     * @brief Start servign the APIs
     *
     */
    void start();

    vector<Device *> *devices = nullptr;
    Document doc;

   private:
    /**
     * @brief Define the routes for each api and their methods
     *
     */
    void setupRoutes();

    /**
     * @brief Get the status of the Network
     *
     * @param response Handler used to send back the reply
     */
    void getNetwork(const Rest::Request& request, Http::ResponseWriter response);

    void getNodes(const Rest::Request& request, Http::ResponseWriter response);

    void getLinks(const Rest::Request& request, Http::ResponseWriter response);

    void setLink(const Rest::Request& request, Http::ResponseWriter response);

    void removeLink(const Rest::Request& request, Http::ResponseWriter response);

    void removeNode(const Rest::Request& request, Http::ResponseWriter response);

    void getPackets(const Rest::Request& request, Http::ResponseWriter response);

    void setReady(const Rest::Request& request, Http::ResponseWriter response);

    /**
     * @brief General reply with JSON
     *
     * @param response Handler used to send back the reply
     */
    void index(const Rest::Request& request, Http::ResponseWriter response);

    /**
     * @brief Initialize the document/Object with all the data information about the Network
     * 
     */
    void initDoc();

    using Lock  = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock metricsLock;
    

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router                    router;
};

#endif