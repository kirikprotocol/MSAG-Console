#ifndef _INFORMER_CONFIGWRAPPER_H
#define _INFORMER_CONFIGWRAPPER_H

#include <string>

namespace smsc {
namespace logger {
class Logger;
}
namespace util {
namespace config {
class Config;
}
}
}

namespace eyeline {
namespace informer {

class ConfigWrapper
{
public:
    ConfigWrapper( const smsc::util::config::Config& cfg,
                   smsc::logger::Logger*             thelog ) :
    cfg_(cfg), log_(thelog) {}
    
    bool getBool( const char* name,
                  bool def, bool usedefault = true ) const;
    int getInt( const char* name, int def, int min, int max,
                bool usedefault = true ) const;
    std::string getString( const char* name,
                           const char* def = 0,
                           bool strip = true ) const;
private:
    const smsc::util::config::Config& cfg_;
    smsc::logger::Logger*             log_;
};

} // informer
} // smsc

#endif
