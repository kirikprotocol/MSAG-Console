#include "StatManConfig.h"

namespace scag {
namespace config {

StatManConfig::StatManConfig()
{
    dir = "";
}

StatManConfig::StatManConfig(ConfigView& cv)
{
    std::auto_ptr<char> dir_( cv.getString("statisticsDir") );
    dir = dir_.get();
}

void StatManConfig::init(ConfigView& cv)
{
    std::auto_ptr<char> dir_( cv.getString("statisticsDir") );
    dir = dir_.get();
}

bool StatManConfig::check(ConfigView& cv)
{   
    std::auto_ptr<char> dir_( cv.getString("statisticsDir") );
    if(   strcmp( dir.c_str(), dir_.get() )   )
        return true;
    else
        return false;
}

}
}
