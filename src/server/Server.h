#ifndef BGP_SIMULATION_SERVER_SERVER_H
#define BGP_SIMULATION_SERVER_SERVER_H

#include <stddef.h>

#include <csignal>
#include <memory>
#include <mutex>
#include <vector>

#include "../entities/Device.h"
#include "pistache/endpoint.h"
#include "pistache/http.h"
#include "pistache/net.h"
#include "pistache/router.h"
#include "rapidjson/document.h"

class ApiEndpoint {
   public:
    explicit ApiEndpoint(Pistache::Address addr)
        : httpEndpoint(std::make_shared<Pistache::Http::Endpoint>(addr)) {}

    /**
     * @brief Initialize the number of wanted threads for the rest server
     *
     * @param thr Number of threads
     */
    void init(size_t thr, std::vector<Device*>* devicesMain);

    /**
     * @brief Start servign the APIs
     *
     */
    void start(volatile sig_atomic_t *stop);

    std::vector<Device*>* devices = nullptr;
    rapidjson::Document   doc;

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
    void getNetwork(const Pistache::Rest::Request& request,
                    Pistache::Http::ResponseWriter response);

    /**
     * @brief Get the status of the Network for ZoomCharts
     *
     * @param response Handler used to send back the reply
     */
    void getNetZoomCharts(const Pistache::Rest::Request& request,
                    Pistache::Http::ResponseWriter response);


    /**
     * @brief Add a new device to the network
     *
     * @param response Handler used to send back the reply
     */
    void addNode(const Pistache::Rest::Request& request,
                 Pistache::Http::ResponseWriter response);

    /**
     * @brief Deactivate a link of the network
     *
     * @param response Handler used to send back the reply
     */
    void breakLink(const Pistache::Rest::Request& request,
                   Pistache::Http::ResponseWriter response);

    /**
     * @brief Remove a node from the Network
     *
     * @param response Handler used to send back the reply
     */
    void removeNode(const Pistache::Rest::Request& request,
                    Pistache::Http::ResponseWriter response);

    /**
     * @brief Activate a link of the network
     *
     * @param response Handler used to send back the reply
     */
    void addLink(const Pistache::Rest::Request& request,
                 Pistache::Http::ResponseWriter response);

    /**
     * @brief General reply with JSON without rapidjson object
     *
     * @param response Handler used to send back the reply
     */
    void index(const Pistache::Rest::Request& request,
               Pistache::Http::ResponseWriter response);

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
    void showGUI(const Pistache::Rest::Request& request,
                 Pistache::Http::ResponseWriter response);

    /**
     * @brief Get image of 'plus' icon for GUI
     *
     * @param response Handler used to send back the reply
     */
    void getAddIcon(const Pistache::Rest::Request& request,
                    Pistache::Http::ResponseWriter response);

    /**
     * @brief Get image of 'endpoint' icon for GUI
     *
     * @param response Handler used to send back the reply
     */
    void getEndpointIcon(const Pistache::Rest::Request& request,
                         Pistache::Http::ResponseWriter response);

    /**
     * @brief Get image of 'packet' icon for GUI
     *
     * @param response Handler used to send back the reply
     */
    void getPacketIcon(const Pistache::Rest::Request& request,
                       Pistache::Http::ResponseWriter response);

    /**
     * @brief Get image of 'router' icon for GUI
     *
     * @param response Handler used to send back the reply
     */
    void getRouterIcon(const Pistache::Rest::Request& request,
                       Pistache::Http::ResponseWriter response);

    /**
     * @brief Get js file for events in the GUI
     *
     * @param response Handler used to send back the reply
     */
    void getNodeEvents(const Pistache::Rest::Request& request,
                       Pistache::Http::ResponseWriter response);

    /**
     * @brief Get css file for style in the GUI
     *
     * @param response Handler used to send back the reply
     */
    void getMainCSS(const Pistache::Rest::Request& request,
                    Pistache::Http::ResponseWriter response);

    using Lock  = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock metricsLock;


    std::shared_ptr<Pistache::Http::Endpoint> httpEndpoint;
    Pistache::Rest::Router                    router;
};

#endif  // BGP_SIMULATION_SERVER_SERVER_H
