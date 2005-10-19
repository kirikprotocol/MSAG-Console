#include "StatManConfig.h"

namespace scag {
namespace config {

StatManConfig::StatManConfig()
{
    dir = "";
}

StatManConfig::StatManConfig(ConfigView& cv)  throw(ConfigException)
{
    try {
        std::auto_ptr<char> dir_( cv.getString("statisticsDir") );
        dir = dir_.get();
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("StatManConfig.StatManConfig, Unknown exception.");
    }
}

void StatManConfig::init(ConfigView& cv) throw(ConfigException)
{
    try {
        std::auto_ptr<char> dir_( cv.getString("statisticsDir") );
        dir = dir_.get();
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("StatManConfig.init, Unknown exception.");
    }
}

bool StatManConfig::check(ConfigView& cv)  throw(ConfigException)
{   
    try {
        std::auto_ptr<char> dir_( cv.getString("statisticsDir") );
        if(   strcmp( dir.c_str(), dir_.get() )   )
            return true;
        else
            return false;
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("StatManConfig.check, Unknown exception.");
    }
}

}
}
