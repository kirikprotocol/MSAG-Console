#include <map>
#include <set>
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
                                  const std::vector<std::string>& paths,
                                  const std::string& name,
                                  unsigned indexgrowth,
                                  unsigned pagesize,
                                  unsigned prealloc,
                                  bool     dodiskio )
{
    clear();
    const StorageNumbering& n = StorageNumbering::instance();
    storages_.resize( n.storages(), 0 );
    assert( paths.size() > 0 );

    unsigned pathidx = 0;
    for ( unsigned i = 0; i < storages_.size(); ++i ) {
        if ( n.node(i) == nodeNumber ) {

            Storage* st = new Storage( *fin_, *expiration_, allocator_ );
            storages_[i] = st;
            st->init( i, queue, paths[pathidx], name, indexgrowth, pagesize, prealloc, dodiskio );
            if ( ++pathidx >= paths.size() ) pathidx = 0;

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


bool CompositeSessionStore::expireSessions( const std::vector< SessionKey >& expired,
                                            const std::vector< std::pair<SessionKey,time_t> >& flush )
{
    typedef std::set< unsigned > keys_type;
    typedef std::map< unsigned, std::vector< SessionKey > > edispatch_type;
    typedef std::map< unsigned, std::vector< std::pair<SessionKey,time_t> > > fdispatch_type;
    keys_type keys;
    edispatch_type edispatch;
    fdispatch_type fdispatch;
    const StorageNumbering& n = StorageNumbering::instance();
    for ( std::vector< SessionKey >::const_iterator i = expired.begin();
          i != expired.end();
          ++i ) {
        const unsigned k = n.storage( i->toIndex() );
        keys.insert(k);
        edispatch[k].push_back( *i );
    }
    for ( std::vector< std::pair<SessionKey,time_t> >::const_iterator i = flush.begin();
          i != flush.end();
          ++i ) {
        const unsigned k = n.storage( i->first.toIndex() );
        keys.insert(k);
        fdispatch[k].push_back( *i );
    }
    bool res = true;
    const std::vector< SessionKey > enull;
    const std::vector< std::pair<SessionKey,time_t> > fnull;
    for ( keys_type::const_iterator i = keys.begin();
          i != keys.end();
          ++i ) {

        if ( ! storages_[*i] ) {
            smsc_log_error( log_, "cannot find storage #%u", *i );
            throw SCAGException( "sess.man: cannot find storage #%u", *i );
        }
        edispatch_type::const_iterator ei = edispatch.find(*i);
        fdispatch_type::const_iterator fi = fdispatch.find(*i);
        if ( !storages_[*i]->expireSessions
             ( ei == edispatch.end() ? enull : ei->second,
               fi == fdispatch.end() ? fnull : fi->second ) ) res = false;
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
