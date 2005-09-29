#include "BillingManagerConfig.h"

namespace scag {
namespace config {

BillingManagerConfig::BillingManagerConfig(ConfigView& cv)
{
  std::auto_ptr<char> cfg_dir_( cv.getString("configDir") );
  std::auto_ptr<char> so_dir_( cv.getString("soDir") );
  cfg_dir = cfg_dir_.get();
  so_dir = so_dir_.get();
}

void BillingManagerConfig::init(ConfigView& cv)
{
    std::auto_ptr<char> cfg_dir_( cv.getString("configDir") );
    std::auto_ptr<char> so_dir_( cv.getString("soDir") );
    cfg_dir = cfg_dir_.get();
    so_dir = so_dir_.get();
}

bool BillingManagerConfig::check(ConfigView& cv)
{   
    std::auto_ptr<char> cfg_dir_( cv.getString("configDir") );
    if(   !strcmp( cfg_dir.c_str(), cfg_dir_.get() )   )
        return false;

    std::auto_ptr<char> so_dir_( cv.getString("soDir") );
    if(   !strcmp( so_dir.c_str(), so_dir_.get() )   )
        return false;

    return true;
}

}
}