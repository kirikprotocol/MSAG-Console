#ifndef _SCAG_PVSS_CORE_CLIENTCONFIG_H
#define _SCAG_PVSS_CORE_CLIENTCONFIG_H

#include "scag/pvss/api/core/Config.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace client {

/// interface
class ClientConfig : public Config
{
public:
    static const int MIN_CONNECTIONS_COUNT = 1;
    static const int DEF_CONNECTIONS_COUNT = 10;
    static const int MAX_CONNECTIONS_COUNT = 1000;

public:
    ClientConfig() : Config() {
    }

    int getConnectionsCount() const {
        return connectionsCount_;
    }
    void setConnectionsCount( int connectionsCount ) /* throw(ConfigException) */  {
        if (connectionsCount < MIN_CONNECTIONS_COUNT || connectionsCount > MAX_CONNECTIONS_COUNT)
            throw ConfigException( "Connections count can't be less than %d and more than %d",
                                   MIN_CONNECTIONS_COUNT, MAX_CONNECTIONS_COUNT );
        this->connectionsCount_ = connectionsCount;
    }

    std::string toString() const {
        std::string s( Config::toString() );
        char buf[128];
        snprintf(buf,sizeof(buf)," connections=%d", connectionsCount_ );
        return s + buf;
    }

protected:
    int connectionsCount_;
};

} // namespace client
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CLIENTCONFIG_H */
