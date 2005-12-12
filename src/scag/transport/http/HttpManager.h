#ifndef SCAG_TRANSPORT_HTTP_MANAGER
#define SCAG_TRANSPORT_HTTP_MANAGER

#include <string>

#include "HttpProcessor.h"

namespace scag { namespace transport { namespace http 
{
    /**
     * Main HTTP transport class.
     * Contains reference to HttpProcessor interface
     */
    class HttpManager
    {
    protected:

        HttpProcessor& processor;

        HttpManager(const HttpManager& manager);

    public:

        HttpManager();
        ~HttpManager();
        Init(HttpProcessor& _processor, 
             int maxThreads, std::string host, short port);
    };
}}}

#endif //SCAG_TRANSPORT_HTTP_MANAGER
