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
        init(cv);
    }


    void SessionManagerConfig::init(const ConfigView& cv) throw(ConfigException)
    {
        unsigned expire = 0;
        // dir = getString(cv,"location");
        clear();
        getInt(cv, "indexgrowth", indexgrowth );
        getInt(cv, "pagesize", pagesize );
        getInt(cv, "prealloc", prealloc );
        getInt(cv, "flushCountLimit", flushlimitsize );
        getInt(cv, "flushTimeLimit", flushlimittime );
        getInt(cv, "initUploadCount", initUploadCount );
        getInt(cv, "initUploadInterval", initUploadInterval );
        try {
            getInt(cv, "expireInterval", expire);
        } catch (...) {}

        getBool(cv, "diskio", diskio);

        // locations
        std::auto_ptr<ConfigView> locs( cv.getSubConfig( "locations" ) );
        std::auto_ptr<config::CStrSet> names(locs->getStrParamNames());
        std::vector< std::string > newlocs;
        newlocs.reserve(names->size());
        for ( config::CStrSet::const_iterator i = names->begin();
              i != names->end();
              ++i ) {

            std::string loc;
            getString( *locs.get(), i->c_str(), loc );
            if ( ! loc.empty() ) newlocs.push_back( loc );

        }

        if ( newlocs.size() > 0 ) {
            dirs = newlocs;
        } else if ( ! usedefault_ ) {
            throw ConfigException("SessionManager.locations has no elements");
        }

        // ugly: applying config right after creation
        if ( expire >= 1000 ) {
            Session::setDefaultLiveTime( expire / 1000 );
        }
    }


    bool SessionManagerConfig::check(const ConfigView& cv) const throw(ConfigException)
    {   
        SessionManagerConfig smc;
        smc.usedefault_ = usedefault_;
        smc.init( cv );
        return ( dirs == smc.dirs &&
                 indexgrowth == smc.indexgrowth &&
                 pagesize == smc.pagesize &&
                 prealloc == smc.prealloc &&
                 flushlimitsize == smc.flushlimitsize &&
                 flushlimittime == smc.flushlimittime &&
                 initUploadCount == smc.initUploadCount &&
                 initUploadInterval == smc.initUploadInterval &&
                 diskio == smc.diskio );
}


void SessionManagerConfig::getString
    ( const scag::config::ConfigView& cfg,
      const char* param,
      std::string& result ) const throw (ConfigException)
{
    try {
        std::auto_ptr< char > p( cfg.getString(param) );
        result = std::string( p.get() );
    } catch (ConfigException&) {
        if ( ! usedefault_ ) throw;
    }
}


void SessionManagerConfig::getInt
    ( const scag::config::ConfigView& cfg,
      const char* param,
      unsigned& result ) const throw (ConfigException)
{
    try {
        unsigned p = unsigned(cfg.getInt(param));
        result = p;
    } catch (ConfigException&) {
        if ( ! usedefault_ ) throw;
    }
}


void SessionManagerConfig::getBool
    ( const scag::config::ConfigView& cfg,
      const char* param,
      bool& result ) const throw (ConfigException)
{
    try {
        bool p = unsigned(cfg.getBool(param));
        result = p;
    } catch (ConfigException&) {
        if ( ! usedefault_ ) throw;
    }
}


void SessionManagerConfig::clear()
{
    dirs.clear();
    dirs.push_back( "sessions" );
    indexgrowth = 1000;
    pagesize = 512;
    prealloc = 100;
    flushlimitsize = 10000;
    flushlimittime = 120;
    initUploadCount = 10;
    initUploadInterval = 100;
    diskio = true;
}

}
}
