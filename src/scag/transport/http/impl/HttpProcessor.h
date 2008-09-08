#ifndef SCAG_TRANSPORT_HTTP_IMPL_PROCESSOR
#define SCAG_TRANSPORT_HTTP_IMPL_PROCESSOR

#include <string>

#include "scag/transport/http/base/HttpProcessor.h"
#include "HttpRouter.h"

namespace scag2 {

namespace sessions {
class ActiveSession;
}

namespace transport {
namespace http {

class HttpProcessorImpl : public HttpProcessor
{
public:
    virtual ~HttpProcessorImpl() {}

    // httpprocessor
    virtual int processRequest(HttpRequest& request);
    virtual int processResponse(HttpResponse& response);
    virtual int statusResponse(HttpResponse& response, bool delivered=true);
    virtual void ReloadRoutes();

    void init(const std::string& cfg);

protected:
    PlacementKindArray defInPlaces;
    PlacementKindArray defOutPlaces;

    smsc::logger::Logger* logger;
    HttpRouterImpl router;
    std::string inURLFields;

    //        bool findPlace(std::string& rs, const PlacementArray& places, HttpRequest& request);
    bool findPlace(const char* wh, std::string& rs, const PlacementArray& places, HttpRequest& request, std::string& url);
    bool findUSR(HttpRequest& request, const PlacementArray& places);
    const PlacementArray& getOutPlaces(const HttpRoute& r, uint32_t t);
    const PlacementArray& getInPlaces(const HttpRoute& r, uint32_t t);
    bool defInURL(uint32_t t);
    void setPlaces(const std::string& rs, const PlacementArray& places, HttpRequest& request, std::string& url);
    void setFields(HttpRequest& request, HttpRoute& r);
    bool parsePath(const std::string &path, HttpRequest& cx);
    void registerEvent(int event, HttpCommand& cmd, bool delivery = false);
    bool makeLongCall( HttpCommand& cmd, sessions::ActiveSession& se );
    void clearPlaces(const PlacementArray& places, HttpRequest& request);

};

}}}

#endif //SCAG_TRANSPORT_HTTP_PROCESSOR
