#ifndef SessionManagerConfig_dot_h
#define SessionManagerConfig_dot_h

#include <time.h>
#include <string>
#include "scag/config/base/ConfigView.h"

namespace scag {
namespace config {

class SessionManagerConfig
{
public:
    SessionManagerConfig();
    SessionManagerConfig(const std::string& dir_)  throw(ConfigException);
    SessionManagerConfig(const ConfigView& cv)  throw(ConfigException);
    void init(const ConfigView& cv)  throw(ConfigException);   
    bool check(const ConfigView& cv)  throw(ConfigException);

    std::string dir;
};

}
}

#endif
