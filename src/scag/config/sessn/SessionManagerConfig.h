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
    SessionManagerConfig();
    SessionManagerConfig(const std::string& dir_)  throw(ConfigException);
    SessionManagerConfig(ConfigView& cv)  throw(ConfigException);
    void init(ConfigView& cv)  throw(ConfigException);   
    bool check(ConfigView& cv)  throw(ConfigException);

    std::string dir;
};

}
}

#endif
