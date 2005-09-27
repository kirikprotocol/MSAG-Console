#include "SessionManagerConfig.h"

namespace scag {
namespace config {

const time_t SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL = 60;

SessionManagerConfig::SessionManagerConfig()
{
    dir = "";
}

SessionManagerConfig::SessionManagerConfig(ConfigView& cv)
{
    std::auto_ptr<char> dir_( cv.getString("location") );
    dir = dir_.get();
}

SessionManagerConfig::SessionManagerConfig(const std::string& dir_, 
                             time_t ei) 
            : dir(dir_), expireInterval(ei) 
{
}

void SessionManagerConfig::init(ConfigView& cv)
{
    std::auto_ptr<char> dir_( cv.getString("location") );
    dir = dir_.get();
}

bool SessionManagerConfig::check(ConfigView& cv)
{   
    std::auto_ptr<char> dir_( cv.getString("location") );
    if(   strcmp( dir.c_str(), dir_.get() )   )
        return true;
    else
        return false;
}

}
}