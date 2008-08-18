#include <memory>

#include "scag/sessions/base/Session2.h"
#include "SessionStore2.h"
#include "scag/sessions/base/SessionFinalizer.h"
#include "scag/sessions/base/SessionExpirationQueue.h"
#include "scag/util/UnlockMutexGuard.h"

namespace scag2 {
namespace sessions {

using namespace transport;
using namespace scag::util::storage;
using scag::util::UnlockMutexGuard;

    /*
    /// temporary: SlowMutex for tests
    class SlowMutex : public smsc::core::synchronization::Mutex
    {
    public:
        SlowMutex() {}
        ~SlowMutex() {}
        /// we sleep before unlock
        void Unlock() {
            timespec t;
            t.tv_sec = 0;
            t.tv_nsec = 100 * 1000 * 1000;
            nanosleep( &t, 0 );
            this->smsc::core::synchronization::Mutex::Unlock();
        }
    };
     */

SessionStoreImpl::SessionStoreImpl( SessionFinalizer& fin,
                                    SessionExpirationQueue& exq,
                                    SessionAllocator* a ) :
nodeNumber_(-1),
fin_(&fin),
expiration_(&exq),
queue_(0),
cache_(0),
stopping_(true),
disk_(0),
allocator_(a),
// expireSet_(0),
log_(0)
{
    log_ = smsc::logger::Logger::getInstance("sess.store");
    if ( !a ) {
        smsc_log_error( log_, "allocator should be passed" );
        throw std::runtime_error( "allocator should be passed" );
    }
    // smsc_log_info(log_, "node=%u created", nodeNumber_ );
}


SessionStoreImpl::~SessionStoreImpl()
{
    stop();
    MutexGuard mg(cacheLock_);
    while ( lockedSessions_ > 0 ) {
        cacheLock_.wait( 100 );
    }
    // smsc_log_debug(log_, "node=%u destroyed", nodeNumber_ );
}


void SessionStoreImpl::init( unsigned nodeNumber,
                             SCAGCommandQueue& queue,
                             const std::string& path,
                             const std::string& name,
                             unsigned indexgrowth,
                             unsigned pagesize,
                             unsigned prealloc )
{
    MutexGuard mg(cacheLock_);
    if ( ! stopping_ ) return; // already inited
    queue_ = &queue;
    nodeNumber_ = nodeNumber;

    totalSessions_ = 0;
    lockedSessions_ = 0;
    storedCommands_ = 0;

    maxcachesize_ = 0;   // maximum of totalSessions
    maxqueuesize_ = 0;   // maximum length of one session command queue
    maxcommands_ = 0;    // maximum of storedCommands

    smsc_log_info( log_, "init path=%s, name=%s, indexgrowth=%d, pagesize=%d, prealloc=%d",
                   path.c_str(), name.c_str(), indexgrowth, pagesize, prealloc );

    cache_.reset( new MemStorage );
    disk_.reset( new DiskStorage );
    const StorageNumbering& n = StorageNumbering::instance();

    for ( unsigned i = 0; i < n.storages(); ++i ) {
        if ( n.node( i ) == nodeNumber_ ) {
                
            const std::string suffix("-pgf");
            std::auto_ptr<PageFile> pgf( new PageFile );
            char buf[10];
            snprintf( buf, sizeof(buf), "%03u", i );
            const std::string idxstr(buf);
            const std::string fn( path + "/" + name + suffix + idxstr + "-data" );
            try {
                pgf->Open( fn );
            } catch (...) {
                pgf->Create( fn, pagesize, prealloc );
            }
            
            std::auto_ptr<EltDiskStorage> eds
                ( new EltDiskStorage
                  ( new DiskIndexStorage( name + suffix + idxstr, path, indexgrowth ),
                    new DiskDataStorage( pgf.release() ) ) );
            disk_->addStorage( i, eds.release() );
            smsc_log_debug(log_, "added storage %d", i );
        }
    }
    stopping_ = false;
}


/// stop processing
void SessionStoreImpl::stop()
{
    smsc_log_info(log_, "stop()");
    MutexGuard mg(cacheLock_);
    stopping_ = true;
}


ActiveSession SessionStoreImpl::fetchSession( const SessionKey& key,
                                              SCAGCommand*      command )
{
    if ( ! command ) return ActiveSession();

    Session* session = command->getSession();
    if ( session ) {
        // fast access to session, w/o locking mutex
        // if command has session, it means that session is locked
        smsc_log_debug(log_, "command %p has session %p, session->cmd=%p", command, session, session->currentCommand() );
        if ( session->currentCommand() != command ) {
            smsc_log_error(log_, "logic error in fetchSession, cmd=%p, cmd->session=%p, session->cmd=%p", command, session, session->currentCommand() );
            ::abort();
        }
        return makeLockedSession(*session,*command);
    }

    // command is not attached

    smsc_log_debug(log_,"fetchSession(key=%s,cmd=%p)", key.toString().c_str(), command );
    std::auto_ptr< SCAGCommand > cmd(command);
    MemStorage::stored_type* v;
    SCAGCommand* prev = 0;
    {
        MutexGuard mg(cacheLock_);
        if ( stopping_ ) return ActiveSession();

        v = cache_->get( key );
        if ( v ) {

            session = cache_->store2val(*v);
            smsc_log_debug( log_, "hit key=%s session=%p session->cmd=%p",
                            key.toString().c_str(), session, session->currentCommand() );

            if ( ! session->currentCommand() || session->currentCommand() == command )
            {
                    
                if ( session->expirationTime() < time(0) ) {
                    smsc_log_debug(log_,"expired key=%s session=%p, cleared", key.toString().c_str(), session );
                    session->clear();
                }

                smsc_log_debug(log_, "fetched key=%s session=%p for cmd=%p", key.toString().c_str(), session, cmd.get() );
                session->setCurrentCommand( command );
                return makeLockedSession(*session, *cmd.release());

            }

            // session is locked
            // reserve a place in queue for a command
            queue_->pushCommand( cmd.get(), SCAGCommandQueue::RESERVE );
            unsigned sz = session->appendCommand( cmd.release() );
            smsc_log_debug(log_, "session is locked, put cmd %p to session queue, sz=%u", command, sz );
            if ( sz > maxqueuesize_ ) maxqueuesize_ = sz;
            if ( ++storedCommands_ > maxcommands_ ) maxcommands_ = storedCommands_;
            return ActiveSession();
        }

        // object not found in cache
        // create a stub to be filled by disk io
        cache_->set( key, cache_->val2store( allocator_->alloc(key) ));
        v = cache_->get( key );
        const unsigned sz = cache_->size();
        if ( sz > maxcachesize_ ) maxcachesize_ = sz;

        session = cache_->store2val(*v);
        prev = session->setCurrentCommand( cmd.release() );
        // release lock
    }
    // commands are owned elsewhere
    // delete prev;

    disk_->get( key, cache_->store2ref(*v) );

    smsc_log_debug( log_, "fetched key=%s session=%p for cmd=%p", key.toString().c_str(), session, command );
    return makeLockedSession(*session,*command);
}


void SessionStoreImpl::releaseSession( Session&                      session,
                                       bool                          flush )
{
    const SessionKey& key = session.sessionKey();
    {
        SCAGCommand* cmd = session.currentCommand();
        smsc_log_debug(log_,"releaseSession(key=%s, session=%p, session->ops=%d, cmd=%p, flush=%d)",
                       key.toString().c_str(), &session, session.operationsCount(), cmd, flush?1:0 );
        if ( ! cmd ) {
            smsc_log_error(log_, "logic error in releaseSession(sess=%p): session->cmd is not set", &session );
            ::abort();
        }
    }

    bool dostopping = false;
    time_t expiration;

    SCAGCommand* prevcmd = 0;
    SCAGCommand* nextcmd = 0;
    {
        MemStorage::stored_type* v;
        MutexGuard mg(cacheLock_);
        dostopping = stopping_;

        v = cache_->get( key );
        if ( !v || cache_->store2val(*v) != &session ) {
            smsc_log_error( log_, "logic error in releaseSession" );
            abort();
            // throw std::runtime_error("SessionStore: logic error in releaseSession" );
        }
        --lockedSessions_;
        expiration = session.expirationTime();

        if ( flush ) {
            UnlockMutexGuard ug(cacheLock_);
            disk_->set( key, cache_->store2ref(*v) );
            smsc_log_debug(log_, "flushed key=%s session=%p", key.toString().c_str(), &session );

        } else {
            UnlockMutexGuard ug(cacheLock_);
            disk_->remove( key );
            smsc_log_debug(log_, "removed key=%s session=%p", key.toString().c_str(), &session );

        }
            
        nextcmd = session.popCommand();
        if ( nextcmd ) setCommandSession( *nextcmd, &session );
        prevcmd = session.setCurrentCommand( nextcmd );
        smsc_log_debug( log_, "released key=%s session=%p, prevcmd=%p nextcmd=%p",
                        key.toString().c_str(), &session, prevcmd, nextcmd );
        // release lock
    }
    // commands are owned elsewhere
    // delete prevcmd;

    if ( ! carryNextCommand(session,nextcmd,true) )
        expiration_->scheduleExpire(expiration, key);
            
}


void SessionStoreImpl::moveLock( Session& s, SCAGCommand* cmd )
{
    if ( !cmd || !s.currentCommand() ) return;
    Session* olds = cmd->getSession();
    if ( olds && olds != &s ) {
        // another session is set
        smsc_log_warn(log_, "cannot moveLock: session=%p session->cmd=%p cmd=%p cmd->session=%p", &s, s.currentCommand(), cmd, olds );
        return;
    }
    MutexGuard mg(cacheLock_);
    s.setCurrentCommand( cmd );
}


inline ActiveSession SessionStoreImpl::makeLockedSession( Session&     s,
                                                          SCAGCommand& c )
{
    Session* olds = c.getSession();
    SCAGCommand* oldc = s.currentCommand();
    setCommandSession(c,&s);
    // should be already set
    // s.setCurrentCommand(&c);
    if ( olds && olds != &s ) {
        smsc_log_error(log_, "logic error in makeAS: cmd->session=%p session=%p", olds, &s );
        ::abort();
    }
    if ( oldc && oldc != &c ) {
        smsc_log_error(log_, "logic error in makeAS: sess->cmd=%p cmd=%p", oldc, &c );
        ::abort();
    }
    ++lockedSessions_;
    return ActiveSession(*this,s);
}


unsigned SessionStoreImpl::storedCommands() const
{
    MutexGuard mg(cacheLock_);
    return storedCommands_;
}


bool SessionStoreImpl::expireSessions( const std::vector< SessionKey >& expired )
{
    if ( expired.size() == 0 ) return true;

    smsc_log_debug( log_, "%u sessions to be expired", expired.size() );

    time_t now = time(0);
    Session* session = 0;
    std::vector< SessionKey >::const_iterator i = expired.begin();
    unsigned longcall = 0;
    unsigned notexpired = 0;
    while ( true ) {

        if ( session ) {
            smsc_log_debug(log_, "finalizing key=%s session=%p", session->sessionKey().toString().c_str(), session );
            // it may take quite long time, so we don't lock mutex yet
            // but the session must be already locked by a fictional command (-1)
            if ( ! fin_->finalize( *session ) ) {
                // session is taken for long call finalization
                ++longcall;
                session = 0;
            }
        }

        MutexGuard mg(cacheLock_);
            
        if ( session ) {
            if ( ! doSessionFinalization(*session) ) ++notexpired;
        }

        if ( i == expired.end() ) break;

        const SessionKey& key = *i;
        ++i;
        MemStorage::stored_type* v = cache_->get( key );
        if ( !v ) {
            // smsc_log_warn(log_,"key=%s to be expired is not found, sz=%u", key.toString().c_str(), curset.size() );
            ++notexpired;
            continue;
        }

        session = cache_->store2val(*v);
        // smsc_log_debug(log_, "expired key=%s session=%p", key.toString().c_str(), session );
        const time_t newexpiration = session->expirationTime();

        if ( session->currentCommand() ) {
            smsc_log_debug(log_,"key=%s session=%p is locked, cmd=%p, skipped",
                           key.toString().c_str(), session, session->currentCommand() );
            ++notexpired;
            session = 0;
            continue;
        }

        // check expiration time again, as it may be prolonged while we were waiting
        if ( ! stopping_ ) {
            if ( now < newexpiration ) {
                smsc_log_debug(log_,"key=%s session=%p is not expired yet",
                               key.toString().c_str(), session );
                ++notexpired;
                session = 0;
                continue;
            }
        }

        // lock session with some fictional command to prevent fetching
        // while finalizing
        session->setCurrentCommand( reinterpret_cast<SCAGCommand*>(-1) );

    } // while

    smsc_log_debug( log_, "sessions not expired: %u, longcalled: %u", notexpired, longcall );
    return (notexpired == 0);
}



void SessionStoreImpl::sessionFinalized( Session& s )
{
    MutexGuard mg(cacheLock_);
    doSessionFinalization( s );
}


void SessionStoreImpl::getSessionsCount( unsigned& sessionsCount,
                                         unsigned& sessionsLockedCount ) const
{
    MutexGuard mg(cacheLock_);
    sessionsCount = totalSessions_;
    sessionsLockedCount = lockedSessions_;
}


/// the cache must be locked here
bool SessionStoreImpl::doSessionFinalization( Session& session )
{
    if ( session.currentCommand() != reinterpret_cast<SCAGCommand*>(-1) ) {
        smsc_log_error(log_, "logic error in session=%p finalization, cmd != -1", &session );
        ::abort();
    }

    SCAGCommand* nextcmd = session.popCommand();
    session.setCurrentCommand( nextcmd );
    if ( nextcmd ) setCommandSession( *nextcmd, &session );
    if ( carryNextCommand(session,nextcmd,false) ) {
        // cannot delete session
        // NOTE: the session was already finalized (possible via session_destroy rule),
        // so make sure it will be initialized
        smsc_log_debug(log_, "key=%s session=%p is revived via clear()", session.sessionKey().toString().c_str(), &session );
        session.clear();
        return false;
    } else {
        // no more commands
        smsc_log_debug(log_, "key=%s session=%p is being destroyed", session.sessionKey().toString().c_str(), &session );
        // NOTE: session should be already deleted from disk
        delete cache_->release( session.sessionKey() );
    }
    return true;
}


bool SessionStoreImpl::carryNextCommand( Session&     session,
                                         SCAGCommand* nextcmd,
                                         bool         dolock )
{
    if ( ! nextcmd ) return false;

    // next command to process, move it to a queue
    const unsigned sz = queue_->pushCommand( nextcmd, SCAGCommandQueue::MOVE );
    if ( sz != unsigned(-1) ) return true;

    // queue is stopped, delete all remaining commands
    std::vector< SCAGCommand* > comlist;
    {
        try {
            if (dolock) cacheLock_.Lock();
            SCAGCommand* com;
            while ( 0 != (com = session.popCommand()) ) {
                comlist.push_back( com );
            }
            // reset the state
            session.setCurrentCommand(0);
        } catch (...) {
        }
        if ( dolock ) cacheLock_.Unlock();
    }
    
    smsc_log_info( log_, "queue is stopped: session=%p deletes %u commands", &session, comlist.size() );
    for ( std::vector< SCAGCommand* >::iterator i = comlist.begin();
          i != comlist.end();
          ++i ) {
        queue_->pushCommand( *i, SCAGCommandQueue::MOVE );
    }
    return false;

} // carryNextCommand

} // namespace sessions
} // namespace scag2
