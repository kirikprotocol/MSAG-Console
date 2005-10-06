#include "BillingManagerConfig.h"

namespace scag {
namespace config {

BillingManagerConfig::BillingManagerConfig(ConfigView& cv) throw(ConfigException)
{
  try {
      std::auto_ptr<char> cfg_dir_( cv.getString("configDir") );
      std::auto_ptr<char> so_dir_( cv.getString("soDir") );
      cfg_dir = cfg_dir_.get();
      so_dir = so_dir_.get();
  }catch(ConfigException& e){
      throw ConfigException(e.what());
  }catch(...){
      throw ConfigException("BillingManagerConfig.BillingManagerConfig, Unknown exception.");
  }
}

void BillingManagerConfig::init(ConfigView& cv) throw(ConfigException)
{
    try {
        std::auto_ptr<char> cfg_dir_( cv.getString("configDir") );
        std::auto_ptr<char> so_dir_( cv.getString("soDir") );
        cfg_dir = cfg_dir_.get();
        so_dir = so_dir_.get();
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("BillingManagerConfig.init, Unknown exception.");
    }

}

bool BillingManagerConfig::check(ConfigView& cv) throw(ConfigException)
{   
    try {
        std::auto_ptr<char> cfg_dir_( cv.getString("configDir") );
        if(   !strcmp( cfg_dir.c_str(), cfg_dir_.get() )   )
            return false;

        std::auto_ptr<char> so_dir_( cv.getString("soDir") );
        if(   !strcmp( so_dir.c_str(), so_dir_.get() )   )
            return false;
    }catch(ConfigException& e){
        throw ConfigException(e.what());
    }catch(...){
        throw ConfigException("BillingManagerConfig.check, Unknown exception.");
    }

    return true;
}

}
}