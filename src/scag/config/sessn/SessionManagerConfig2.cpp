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

            // dir = getString(cv,"location");
            name = getString(cv,"name");
            indexgrowth = cv.getInt("indexgrowth");
            pagesize = cv.getInt("pagesize");
            prealloc = cv.getInt("prealloc");
            diskio = (cv.getInt("diskio") != 0);

            // locations
            dirs.clear();
            std::auto_ptr<ConfigView> locs( cv.getSubConfig( "locations" ) );
            std::auto_ptr<config::CStrSet> names(locs->getStrParamNames());
            for ( config::CStrSet::const_iterator i = names->begin();
                  i != names->end();
                  ++i ) {

                dirs.push_back( getString( *locs.get(), i->c_str() ) );

            }

        } catch (...) {
            if ( ! usedefault_ ) throw;
        }

        if ( dirs.size() == 0 ) {
            if ( ! usedefault_ ) throw ConfigException("SessionManager.locations has no elements");
            dirs.push_back( "sessions" );
        }
    }


    bool SessionManagerConfig::check(const ConfigView& cv) const throw(ConfigException)
    {   
        SessionManagerConfig smc;
        smc.usedefault_ = usedefault_;
        smc.init( cv );
        return ( dirs == smc.dirs &&
                 name == smc.name &&
                 indexgrowth == smc.indexgrowth &&
                 pagesize == smc.pagesize &&
                 prealloc == smc.prealloc &&
                 diskio == smc.diskio );
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
        dirs.clear();
        dirs.push_back( "sessions" );
        name = "sessions";
        indexgrowth = 1000;
        pagesize = 512;
        prealloc = 100;
        diskio = true;
    }

}
}
