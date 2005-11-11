#ifndef StatManConf_dot_h
#define StatManConf_dot_h

#include "scag/config/ConfigView.h"

namespace scag {
namespace config {

class StatManConfig {
public:    
    StatManConfig();
    StatManConfig(ConfigView& cv)  throw(ConfigException);
    void init(ConfigView& cv)  throw(ConfigException);   
    bool check(ConfigView& cv)  throw(ConfigException);
    std::string getDir() const;
    std::string getPerfHost() const;
    int getPerfGenPort() const;
    int getPerfSvcPort() const;
    int getPerfScPort() const;
protected:
        std::string dir;
        std::string perfHost;
        int perfGenPort;
        int perfSvcPort;
        int perfScPort;
};

}
}

#endif
