#ifndef SCAG_TRANSPORT_HTTP_BASE_PROCESSOR
#define SCAG_TRANSPORT_HTTP_BASE_PROCESSOR

#include <string>

#include "HttpCommand2.h"

namespace scag2 {
namespace transport {
namespace http {

class HttpProcessor 
{
public:
    static HttpProcessor& Instance();

public:
    virtual ~HttpProcessor();
        
    /**
     * Should be called on HttpRequest comes from abonent.
     * 
     * @param   request - HttpRequest to be processed
     * @return  true if request was successfuly processed and
     *          accepted to forward to service
     */
    virtual int processRequest(HttpRequest& request) = 0;
        
    /**
     * Should be called on HttpResponse comes from service.
     * 
     * @param   response - HttpResponse to be processed, 
     *          should contains command context data
     * @return  true if response was successfuly processed and
     *          accepted to return to abonent
     */
    virtual int processResponse(HttpResponse& response) = 0;

    /**
     * Should be called on HttpResponse delivery (ok or failed).
     * 
     * @param   response - HttpResponse tried to deliver
     * @param   delivered - packet delivery outcome status (ok or failed).
     */
    virtual int statusResponse(HttpResponse& response, bool delivered=true) = 0;

    virtual void ReloadRoutes() = 0;

protected:
    HttpProcessor();

private:
    HttpProcessor( const HttpProcessor& );
    HttpProcessor& operator = ( const HttpProcessor& );
    
};

}}}

#endif //SCAG_TRANSPORT_HTTP_PROCESSOR
