#include <map>
#include <set>
#include "CompositeSessionStore.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "scag/counter/Manager.h"
#include "scag/counter/Accumulator.h"

namespace scag2 {
namespace sessions {

using namespace util::storage;
using smsc::core::threads::Thread;

CompositeSessionStore::CompositeSessionStore( smsc::logger::Logger* thelog,
                                              SessionFinalizer&       fin,
                                              SessionExpirationQueue& exq,
                                              SessionAllocator*       a ) :
stopped_(true),
log_(thelog),
fin_(&fin),
expiration_(&exq),
allocator_(a),
initialThread_(0)
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
                                  unsigned indexgrowth,
                                  unsigned pagesize,
                                  unsigned prealloc,
                                  unsigned initialCount,
                                  unsigned initialTime,
                                  bool     dodiskio )
{
    MutexGuard mg(stopLock_);
    if ( ! stopped_ ) return;
    const StorageNumbering& n = StorageNumbering::instance();
    storages_.resize( n.storages(), 0 );
    assert( paths.size() > 0 );
    
    initialChunk_ = initialCount;
    initialTime_ = initialTime;

    {
        counter::Manager& mgr = counter::Manager::getInstance();
        totalSessions_ = mgr.registerAnyCounter
            (new counter::Accumulator("sys.sessions.total",
                                      mgr.getObserver("sys.sessions.total").get()));
        loadedSessions_ = mgr.registerAnyCounter
            (new counter::Accumulator("sys.sessions.active",
                                      mgr.getObserver("sys.sessions.active").get()));
        lockedSessions_ = mgr.registerAnyCounter
            (new counter::Accumulator("sys.sessions.locked",
                                      mgr.getObserver("sys.sessions.locked").get()));
        storedCommands_ = mgr.registerAnyCounter
            (new counter::Accumulator("sys.sessions.cmdqueue",
                                      mgr.getObserver("sys.sessions.cmdqueue").get()));
        if (!totalSessions_.get() || !loadedSessions_.get() ||
            !lockedSessions_.get() || !storedCommands_.get() )
            throw std::runtime_error("cannot create session counters");
        // totalSessions_->setMaxVal(2000);
        // loadedSessions_->setMaxVal(500);
        // lockedSessions_->setMaxVal(200);
    }

    unsigned pathidx = 0;
    for ( unsigned i = 0; i < storages_.size(); ++i ) {
        if ( n.node(i) == nodeNumber ) {
            if ( ! storages_[i] ) {
                Storage* st = new Storage( *fin_, *expiration_, allocator_ );
                storages_[i] = st;
            }
            storages_[i]->init( i, queue, paths[pathidx], indexgrowth, pagesize, prealloc, dodiskio );
            if ( ++pathidx >= paths.size() ) pathidx = 0;
        }
    }
}


void CompositeSessionStore::start()
{
    if ( ! stopped_ ) return;
    if ( ! initialThread_ ) {
        initialThread_ = new InitialThread(*this);
    }
    smsc_log_debug( log_, "initial upload thread is starting" );
    stopped_ = false;
    initialThread_->Start();
}


void CompositeSessionStore::stop()
{
    {
        MutexGuard mg(stopLock_);
        if ( stopped_ ) return;
        smsc_log_debug( log_, "stop issued" );
        stopped_ = true;
        stopLock_.notify();
    }
    if ( initialThread_ ) initialThread_->WaitFor();
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


/*
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
 */


bool CompositeSessionStore::expireSessions( std::vector< std::pair<SessionKey,time_t> >& expired,
                                            const std::vector< std::pair<SessionKey,time_t> >& flush )
{
    typedef std::set< unsigned > keys_type;
    typedef std::map< unsigned, std::vector< std::pair<SessionKey,time_t> > > fdispatch_type;
    keys_type keys;
    fdispatch_type edispatch;
    fdispatch_type fdispatch;
    const StorageNumbering& n = StorageNumbering::instance();
    for ( std::vector< std::pair<SessionKey,time_t> >::const_iterator i = expired.begin();
          i != expired.end();
          ++i ) {
        const unsigned k = n.storage( i->first.toIndex() );
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
    std::vector< std::pair<SessionKey,time_t> > fnull;
    Thread::Yield();
    expired.clear();
    for ( keys_type::const_iterator i = keys.begin();
          i != keys.end();
          ++i ) {

        if ( ! storages_[*i] ) {
            smsc_log_error( log_, "cannot find storage #%u", *i );
            throw SCAGException( "sess.man: cannot find storage #%u", *i );
        }
        fdispatch_type::iterator ei = edispatch.find(*i);
        fdispatch_type::const_iterator fi = fdispatch.find(*i);
        if ( !storages_[*i]->expireSessions
             ( ei == edispatch.end() ? fnull : ei->second,
               fi == fdispatch.end() ? fnull : fi->second ) ) {
            if (ei != edispatch.end()) {
                expired.insert(expired.end(),
                               ei->second.begin(),
                               ei->second.end());
            }
            res = false;
        }
        Thread::Yield();
    }
    return res;
}


void CompositeSessionStore::sessionFinalized( Session& s )
{
    store( s.sessionKey() )->sessionFinalized( s );
}


void CompositeSessionStore::getSessionsCount( unsigned& sessionsCount,
                                              unsigned& sessionsLoadedCount,
                                              unsigned& sessionsLockedCount ) const
{
    if ( stopped_ ) return;
    sessionsCount = totalSessions_->getValue();
    sessionsLoadedCount = loadedSessions_->getValue();
    sessionsLockedCount = lockedSessions_->getValue();
    
    /*
    sessionsCount = 0;
    sessionsLoadedCount = 0;
    sessionsLockedCount = 0;
    for ( std::vector< Storage* >::const_iterator i = storages_.begin();
          i != storages_.end();
          ++i ) {
        if ( ! *i ) continue;
        unsigned sc, slc, skc;
        (*i)->getSessionsCount( sc, slc, skc );
        sessionsCount += sc;
        sessionsLoadedCount += slc;
        sessionsLockedCount += skc;
    }
     */
}


void CompositeSessionStore::clear()
{
    if ( initialThread_ ) initialThread_->WaitFor();
    delete initialThread_;
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
    CompositeSessionStore::store( const SessionKey& key ) const /*throw ( SCAGException )*/
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


int CompositeSessionStore::InitialThread::Execute()
{
    smsc_log_debug(store_.log_, "initial upload thread is started");
    std::list< Storage* > hasinit_;
    {
        MutexGuard mg(store_.stopLock_);
        if ( ! store_.stopped_ ) {
            for ( std::vector< Storage* >::iterator i = store_.storages_.begin();
                  i != store_.storages_.end();
                  ++i ) {
                if ( ! *i ) continue;
                hasinit_.push_back( *i );
            }
        }
    }
    while ( ! hasinit_.empty() ) {

        for ( std::list< Storage* >::iterator i = hasinit_.begin();
              i != hasinit_.end();
              ++i ) {
            if ( ! (*i)->uploadInitial( store_.initialChunk_ ) ) {
                hasinit_.erase( i );
                break;
            }
        }

        MutexGuard mg(store_.stopLock_);
        if ( store_.stopped_ ) break;
        store_.stopLock_.wait( store_.initialTime_ );
        if ( store_.stopped_ ) break;
    }
    smsc_log_debug(store_.log_, "initial upload thread is stopped");
    return 0;
}


} // namespace sessions
} // namespace scag2
