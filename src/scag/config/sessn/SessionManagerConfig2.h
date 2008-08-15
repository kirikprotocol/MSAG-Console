#ifndef SessionManagerConfig2_dot_h
#define SessionManagerConfig2_dot_h

#include <time.h>
#include <string>
#include "scag/config/base/ConfigView.h"

namespace scag2 {
namespace config {

class SessionManagerConfig
{
public:
    SessionManagerConfig();
    // SessionManagerConfig(const std::string& dir_)  throw(ConfigException);
    SessionManagerConfig(const ConfigView& cv)  throw(ConfigException);
    void init(const ConfigView& cv)  throw(ConfigException);   
    bool check(const ConfigView& cv) const throw(ConfigException);

    void useDefault( bool ud ) { usedefault_ = ud; }

    std::string dir;
    std::string name;
    unsigned    indexgrowth;
    unsigned    pagesize;
    unsigned    prealloc;
private:
    std::string getString( const scag::config::ConfigView& cfg,
                           const char* param ) const throw (ConfigException);

    void clear();

private:
    bool        usedefault_;
};

}
}

#endif
