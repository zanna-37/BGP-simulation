#ifndef BGP_SIMULATION_SERVER_SERVER_H
#define BGP_SIMULATION_SERVER_SERVER_H

#include <string>

#include "../entities/EndPoint.h"
#include "../entities/Router.h"
#include "../logger/Logger.h"
#include "pistache/endpoint.h"
#include "pistache/http.h"
#include "pistache/router.h"
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

    /**
     * @brief Initialize the number of wanted threads for the rest server
     *
     * @param thr Number of threads
     */
    void init(size_t thr, vector<Device*>* devicesMain);

    /**
     * @brief Start servign the APIs
     *
     */
    void start();

    vector<Device*>* devices = nullptr;
    Document         doc;

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
    void getNetwork(const Rest::Request& request,
                    Http::ResponseWriter response);


    /**
     * @brief Add a new device to the network
     *
     * @param response Handler used to send back the reply
     */
    void addNode(const Rest::Request& request, Http::ResponseWriter response);

    /**
     * @brief Deactivate a link of the network
     *
     * @param response Handler used to send back the reply
     */
    void breakLink(const Rest::Request& request, Http::ResponseWriter response);

    /**
     * @brief Remove a node from the Network
     *
     * @param response Handler used to send back the reply
     */
    void removeNode(const Rest::Request& request,
                    Http::ResponseWriter response);
    /**
     * @brief General reply with JSON without rapidjson object
     *
     * @param response Handler used to send back the reply
     */
    void index(const Rest::Request& request, Http::ResponseWriter response);

    /**
     * @brief Initialize the document/Object with all the data information about
     * the Network
     *
     */
    void initDoc();

    /**
     * @brief Show GUI of BGP simulation
     *
     * @param response Handler used to send back the reply
     */
    void showGUI(const Rest::Request& request, Http::ResponseWriter response);

    /**
     * @brief Get image of 'plus' icon for GUI
     *
     * @param response Handler used to send back the reply
     */
    void getAddIcon(const Rest::Request& request,
                    Http::ResponseWriter response);

    /**
     * @brief Get image of 'endpoint' icon for GUI
     *
     * @param response Handler used to send back the reply
     */
    void getEndpointIcon(const Rest::Request& request,
                         Http::ResponseWriter response);

    /**
     * @brief Get image of 'packet' icon for GUI
     *
     * @param response Handler used to send back the reply
     */
    void getPacketIcon(const Rest::Request& request,
                       Http::ResponseWriter response);

    /**
     * @brief Get image of 'router' icon for GUI
     *
     * @param response Handler used to send back the reply
     */
    void getRouterIcon(const Rest::Request& request,
                       Http::ResponseWriter response);

    /**
     * @brief Get js file for events in the GUI
     *
     * @param response Handler used to send back the reply
     */
    void getNodeEvents(const Rest::Request& request,
                       Http::ResponseWriter response);

    /**
     * @brief Get css file for style in the GUI
     *
     * @param response Handler used to send back the reply
     */
    void getMainCSS(const Rest::Request& request,
                    Http::ResponseWriter response);

    using Lock  = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock metricsLock;


    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router                    router;
};

#endif  // BGP_SIMULATION_SERVER_SERVER_H