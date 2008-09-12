#ifndef _SCAG_TRANSPORT_HTTP_BASE_HTTPMANAGER_H
#define _SCAG_TRANSPORT_HTTP_BASE_HTTPMANAGER_H

#include "scag/config/http/HttpManagerConfig.h"

namespace scag2 {
namespace transport {
namespace http {

class HttpContext;

class HttpManager
{
public:
    static HttpManager& Instance();

    virtual ~HttpManager();

    virtual void shutdown() = 0;
    virtual void process(HttpContext *cx) = 0;
    virtual void readerProcess(HttpContext *cx) = 0;
    virtual void writerProcess(HttpContext* cx) = 0;
    virtual config::HttpManagerConfig& getConfig() = 0;
    virtual void getQueueLen(uint32_t& reqLen, uint32_t& respLen, uint32_t& lcmLen) = 0;
    virtual bool isLicenseExpired() = 0;

protected:
    HttpManager();

private:
    HttpManager( const HttpManager& );
    HttpManager& operator = ( const HttpManager& );

};

}
}
}

#endif /* !_SCAG_TRANSPORT_HTTP_BASE_HTTPMANAGER_H */
