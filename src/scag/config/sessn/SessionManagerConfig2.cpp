#include "SessionManagerConfig2.h"


namespace scag2 {
namespace config {


    SessionManagerConfig::SessionManagerConfig() :
    usedefault_(true)
    {
        clear();
    }


    SessionManagerConfig::SessionManagerConfig(const ConfigView& cv) throw(ConfigException) :
    usedefault_(true)
    {
        clear();
        init(cv);
    }


    void SessionManagerConfig::init(const ConfigView& cv) throw(ConfigException)
    {
        try {
            dir = getString(cv,"location");
            name = getString(cv,"name");
            indexgrowth = cv.getInt("indexgrowth");
            pagesize = cv.getInt("pagesize");
            prealloc = cv.getInt("prealloc");
        } catch (...) {
            if ( ! usedefault_ ) throw;
        }
    }


    bool SessionManagerConfig::check(const ConfigView& cv) const throw(ConfigException)
    {   
        SessionManagerConfig smc;
        smc.usedefault_ = usedefault_;
        smc.init( cv );
        return ( dir == smc.dir &&
                 name == smc.name &&
                 indexgrowth == smc.indexgrowth &&
                 pagesize == smc.pagesize &&
                 prealloc == smc.prealloc );
}


    std::string SessionManagerConfig::getString
        ( const scag::config::ConfigView& cfg,
          const char* param ) const throw (ConfigException)
    {
        std::auto_ptr< char > p( cfg.getString(param) );
        return std::string( p.get() );
    }


    void SessionManagerConfig::clear()
    {
        dir = "sessions";
        name = "sessions";
        indexgrowth = 1000;
        pagesize = 512;
        prealloc = 100;
    }

}
}
