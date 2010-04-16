#ifndef SessionManagerConfig2_dot_h
#define SessionManagerConfig2_dot_h

#include <time.h>
#include <vector>
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

    std::vector<std::string> dirs;
    unsigned                 indexgrowth;
    unsigned                 pagesize;
    unsigned                 prealloc;           // in pages/blocks
    unsigned                 flushlimitsize;
    unsigned                 flushlimittime;     // in seconds
    unsigned                 initUploadCount;
    unsigned                 initUploadInterval; // in ms
    unsigned                 expireInterval;     // in ms
    unsigned                 ussdExpireInterval; // in ms
    bool                     diskio;

private:
    void getString( const scag::config::ConfigView& cfg,
                    const char* param,
                    std::string& result ) const throw (ConfigException);
    void getInt( const scag::config::ConfigView& cfg,
                 const char* param,
                 unsigned& result ) const throw (ConfigException);
    void getBool( const scag::config::ConfigView& cfg,
                  const char* param,
                  bool& result ) const throw (ConfigException);
    void clear();

private:
    bool        usedefault_;
};

}
}

#endif
