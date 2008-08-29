#include <map>
#include "CompositeSessionStore.h"

namespace scag2 {
namespace sessions {

CompositeSessionStore::CompositeSessionStore( smsc::logger::Logger* thelog,
                                              SessionFinalizer&       fin,
                                              SessionExpirationQueue& exq,
                                              SessionAllocator*       a ) :
stopped_(true),
log_(thelog),
fin_(&fin),
expiration_(&exq),
allocator_(a)
{
    smsc_log_debug( log_, "composite session store created");
}


CompositeSessionStore::~CompositeSessionStore()
{
    clear();
}


void CompositeSessionStore::init( unsigned nodeNumber,
                                  SCAGCommandQueue& queue,
                                  const std::string& path,
                                  const std::string& name,
                                  unsigned indexgrowth,
                                  unsigned pagesize,
                                  unsigned prealloc )
{
    clear();
    const StorageNumbering& n = StorageNumbering::instance();
    storages_.resize( n.storages(), 0 );

    for ( unsigned i = 0; i < storages_.size(); ++i ) {
        if ( n.node(i) == nodeNumber ) {

            Storage* st = new Storage( *fin_, *expiration_, allocator_ );
            storages_[i] = st;
            st->init( i, queue, path, name, indexgrowth, pagesize, prealloc );

        }
    }
    stopped_ = false;
}


void CompositeSessionStore::stop()
{
    stopped_ = true;
    smsc_log_debug( log_, "stop issued" );
    for ( std::vector< Storage* >::const_iterator i = storages_.begin();
          i != storages_.end();
          ++i ) {
        if ( ! *i ) continue;
        (*i)->stop();
    }
    smsc_log_debug( log_, "stop sent to storages" );
}


ActiveSession CompositeSessionStore::fetchSession( const SessionKey&             key,
                                                   std::auto_ptr< SCAGCommand >& cmd,
                                                   bool                          create )
{
    return store(key)->fetchSession( key, cmd, create );
}


unsigned CompositeSessionStore::storedCommands() const
{
    unsigned ret = 0;
    if ( ! stopped_ ) {
        for ( std::vector< Storage* >::const_iterator i = storages_.begin();
              i != storages_.end();
              ++i ) {
            if ( ! *i ) continue;
            ret += (*i)->storedCommands();
        }
    }
    return ret;
}


bool CompositeSessionStore::expireSessions( const std::vector< SessionKey >& expired )
{
    std::map< unsigned, std::vector< SessionKey > > dispatch;
    const StorageNumbering& n = StorageNumbering::instance();
    for ( std::vector< SessionKey >::const_iterator i = expired.begin();
          i != expired.end();
          ++i ) {
        const unsigned k = n.storage( i->toIndex() );
        dispatch[k].push_back( *i );
    }

    bool res = true;
    for ( std::map< unsigned, std::vector< SessionKey > >::const_iterator i = dispatch.begin();
          i != dispatch.end();
          ++i ) {

        if ( ! storages_[i->first] ) {
            smsc_log_error( log_, "cannot find storage #%u", i->first );
            throw SCAGException( "sess.man: cannot find storage #%u", i->first );
        }
        if ( !storages_[i->first]->expireSessions( i->second ) ) res = false;
    }
    return res;
}


void CompositeSessionStore::sessionFinalized( Session& s )
{
    store( s.sessionKey() )->sessionFinalized( s );
}


void CompositeSessionStore::getSessionsCount( unsigned& sessionsCount,
                                              unsigned& sessionsLockedCount ) const
{
    sessionsCount = 0;
    sessionsLockedCount = 0;
    if ( stopped_ ) return;
    for ( std::vector< Storage* >::const_iterator i = storages_.begin();
          i != storages_.end();
          ++i ) {
        if ( ! *i ) continue;
        unsigned sc, slc;
        (*i)->getSessionsCount( sc, slc );
        sessionsCount += sc;
        sessionsLockedCount += slc;
    }
}


void CompositeSessionStore::clear()
{
    stop();
    for ( std::vector< Storage* >::iterator i = storages_.begin();
          i != storages_.end();
          ++i ) {
        if ( ! *i ) continue;
        Storage* s = *i;
        *i = 0;
        delete s;
    }
}


CompositeSessionStore::Storage* 
    CompositeSessionStore::store( const SessionKey& key ) const throw ( SCAGException )
{
    const unsigned n = StorageNumbering::instance().storage( key.toIndex() );
    if ( ! storages_[n] ) {
        smsc_log_error( log_, "cannot find storage #%u for key=%s", n, key.toString().c_str() );
        throw SCAGException( "sess.man: storage for key=%s, idx=%llu is not found",
                             key.toString().c_str(), 
                             static_cast< unsigned long long>(key.toIndex()) );
    }
    return storages_[n];
}

} // namespace sessions
} // namespace scag2
