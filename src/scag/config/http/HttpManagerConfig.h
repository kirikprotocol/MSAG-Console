#ifndef HttpManagerConfig_dot_h
#define HttpManagerConfig_dot_h

#include <time.h>
#include <string>
#include "scag/config/ConfigView.h"

namespace scag {
namespace config {

class HttpManagerConfig
{
public:

    HttpManagerConfig(int rs, int ws, int rps, int wps, int sps, int sql, int ct, const std::string h, int p):
        readerSockets(rs), writerSockets(ws),
        readerPoolSize(rps),writerPoolSize(wps),
        scagPoolSize(sps), scagQueueLimit(sql),
        connectionTimeout(ct), host(h), port(p)
    {
    }

    HttpManagerConfig():
        readerSockets(0), writerSockets(0),
        readerPoolSize(0),writerPoolSize(0),
        scagPoolSize(0), scagQueueLimit(0),
        connectionTimeout(0), host(""), port(0)
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
};

}
}

#endif
