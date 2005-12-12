#ifndef SCAG_TRANSPORT_HTTP_PROCESSOR
#define SCAG_TRANSPORT_HTTP_PROCESSOR

#include <string>

#include "HttpCommand.h"

namespace scag { namespace transport { namespace http 
{
    class HttpProcessor 
    {
    public:
        
        /**
         * Should be called on HttpRequest comes from abonent.
         * 
         * @param   request - HttpRequest to be processed
         * @return  true if request was successfuly processed and
         *          accepted to forward to service
         */
        bool processRequest(HttpRequest& request) = 0;
        
        /**
         * Should be called on HttpResponse comes from service.
         * 
         * @param   response - HttpResponse to be processed, 
         *          should contains command context data
         * @return  true if response was successfuly processed and
         *          accepted to return to abonent
         */
        bool processResponse(HttpResponse& response) = 0;

        /**
         * Should be called on HttpResponse delivery (ok or failed).
         * 
         * @param   response - HttpResponse tried to deliver
         * @param   delivered - packet delivery outcome status (ok or failed).
         */
        void statusResponse(const HttpResponse& response, bool delivered=true) = 0;

    protected:

        virtual ~HttpProcessor() {}
    };
}}}

#endif //SCAG_TRANSPORT_HTTP_PROCESSOR