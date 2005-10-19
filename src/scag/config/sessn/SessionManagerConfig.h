#ifndef SessionManagerConfig_dot_h
#define SessionManagerConfig_dot_h

#include <time.h>
#include <string>
#include "scag/config/ConfigView.h"

namespace scag {
namespace config {

class SessionManagerConfig
{
public:
    static const time_t DEFAULT_EXPIRE_INTERVAL;
    SessionManagerConfig();
    SessionManagerConfig(const std::string& dir_, 
                             time_t ei = DEFAULT_EXPIRE_INTERVAL)  throw(ConfigException);
    SessionManagerConfig(ConfigView& cv)  throw(ConfigException);
    void init(ConfigView& cv)  throw(ConfigException);   
    bool check(ConfigView& cv)  throw(ConfigException);

    std::string dir;
    time_t      expireInterval;
};

}
}

#endif
