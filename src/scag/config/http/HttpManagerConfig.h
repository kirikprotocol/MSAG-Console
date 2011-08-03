#ifndef HttpManagerConfig_dot_h
#define HttpManagerConfig_dot_h

#include <time.h>
#include <string>
#include "scag/config/base/ConfigView.h"

namespace scag {
namespace config {

class HttpManagerConfig
{
public:

    HttpManagerConfig(int rs, int ws, int rps, int wps, int sps, int sql, int ct,
    		const std::string h, int p,
    		bool eh, int ph, std::string ch, int th):
        readerSockets(rs), writerSockets(ws),
        readerPoolSize(rps),writerPoolSize(wps),
        scagPoolSize(sps), scagQueueLimit(sql),
        connectionTimeout(ct), host(h), port(p),
    	httpsEnabled(eh), httpsPort(ph),
    	httpsCertificates(ch), httpsTimeout(th)
    {
    }

    HttpManagerConfig():
        readerSockets(0), writerSockets(0),
        readerPoolSize(0),writerPoolSize(0),
        scagPoolSize(0), scagQueueLimit(0),
        connectionTimeout(0), host(""), port(0),
        httpsEnabled(false), httpsPort(0),
        httpsCertificates(""), httpsTimeout(0)
    {
    }

    HttpManagerConfig(HttpManagerConfig& src)
		: readerSockets(src.readerSockets)
		, writerSockets(src.writerSockets)
		, readerPoolSize(src.readerPoolSize)
		, writerPoolSize(src.writerPoolSize)
		, scagPoolSize(src.scagPoolSize)
		, scagQueueLimit(src.scagQueueLimit)
		, connectionTimeout(src.connectionTimeout)
		, host(src.host)
		, port(src.port)
		, httpsEnabled(src.httpsEnabled)
		, httpsPort(src.httpsPort)
		, httpsCertificates(src.httpsCertificates)
		, httpsTimeout(src.httpsTimeout)
    {
    }

    HttpManagerConfig(const ConfigView& cv)  throw(ConfigException);
    void init(const ConfigView& cv)  throw(ConfigException);   
    bool check(const ConfigView& cv)  throw(ConfigException);

    int readerSockets;
    int writerSockets;
    int readerPoolSize;
    int writerPoolSize;
    int scagPoolSize;    
    int scagQueueLimit;
    int connectionTimeout;
    //unsigned int maxHeaderLength;
    std::string host;
    int port;
    bool httpsEnabled;
    int httpsPort;
    std::string httpsCertificates;
    int httpsTimeout;
};

}
}

namespace scag2 {
namespace config {
using scag::config::HttpManagerConfig;
}
}

#endif
