#ifndef PersClientConfig_dot_h
#define PersClientConfig_dot_h

#include <time.h>
#include <string>
#include "scag/config/ConfigView.h"

namespace scag {
namespace config {

class PersClientConfig
{
public:

    PersClientConfig(const std::string h, int p, int to):
        host(h), port(p), timeout(to)
    {
    }

    PersClientConfig(): port(1200), timeout(60)
    {
    }

    PersClientConfig(ConfigView& cv)  throw(ConfigException);
    void init(ConfigView& cv)  throw(ConfigException);   
    bool check(ConfigView& cv)  throw(ConfigException);

    std::string host;
    int port, timeout;    
};

}
}

#endif
