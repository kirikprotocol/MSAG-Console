#include "SessionManagerConfig.h"

namespace scag {
namespace config {

const time_t SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL = 60;

SessionManagerConfig::SessionManagerConfig()
{
    dir = "";
}

SessionManagerConfig::SessionManagerConfig(ConfigView& cv)  throw(ConfigException)
{
    try {
        std::auto_ptr<char> dir_( cv.getString("location") );
        dir = dir_.get();
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("SessionManagerConfig.SessionManagerConfig, Unknown exception.");
    }
}

SessionManagerConfig::SessionManagerConfig(const std::string& dir_, 
                             time_t ei)   throw(ConfigException) 
            : dir(dir_), expireInterval(ei) 
{
}

void SessionManagerConfig::init(ConfigView& cv)   throw(ConfigException)
{
    try {
        std::auto_ptr<char> dir_( cv.getString("location") );
        dir = dir_.get();
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("SessionManagerConfig.init, Unknown exception.");
    }
}

bool SessionManagerConfig::check(ConfigView& cv)   throw(ConfigException)
{   
    try {
        std::auto_ptr<char> dir_( cv.getString("location") );
        if(   strcmp( dir.c_str(), dir_.get() )   )
            return true;
        else
            return false;
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("SessionManagerConfig.check, Unknown exception.");
    }
}

}
}