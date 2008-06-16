#include "SessionManagerConfig.h"

namespace scag {
namespace config {


SessionManagerConfig::SessionManagerConfig()
{
    dir = "";
}

SessionManagerConfig::SessionManagerConfig(const ConfigView& cv) throw(ConfigException)
{
    init(cv);
}

SessionManagerConfig::SessionManagerConfig(const std::string& dir_)   throw(ConfigException) 
            : dir(dir_) 
{
}

void SessionManagerConfig::init(const ConfigView& cv)   throw(ConfigException)
{
    try {
        std::auto_ptr<char> dir_( cv.getString("location") );
        dir = dir_.get();
        /*
        expireInterval = cv.getInt("expireInterval");
        if (expireInterval <=0) expireInterval = DEFAULT_EXPIRE_INTERVAL;
        */
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("SessionManagerConfig.init, Unknown exception.");
    }
}

bool SessionManagerConfig::check(const ConfigView& cv)   throw(ConfigException)
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
