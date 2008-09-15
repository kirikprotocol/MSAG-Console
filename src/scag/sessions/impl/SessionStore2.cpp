#include <memory>

#include "scag/sessions/base/Session2.h"
#include "scag/sessions/base/Operation.h"
#include "SessionStore2.h"
#include "scag/sessions/base/SessionFinalizer.h"
#include "scag/sessions/base/SessionExpirationQueue.h"
#include "scag/util/UnlockMutexGuard.h"
#include "scag/util/Print.h"

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
// nodeNumber_(-1),
fin_(&fin),
expiration_(&exq),
queue_(0),
stopping_(true),
allocator_(a),
cache_(0),
disk_(0),
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


void SessionStoreImpl::init( unsigned eltNumber,
                             SCAGCommandQueue& queue,
                             const std::string& path,
                             const std::string& name,
                             unsigned indexgrowth,
                             unsigned pagesize,
                             unsigned prealloc,
                             bool     dodiskio )
{
    MutexGuard mg(cacheLock_);
    if ( ! stopping_ ) return; // already inited

    diskio_ = dodiskio;
    queue_ = &queue;
    // eltNumber_ = eltNumber;

    totalSessions_ = 0;
    lockedSessions_ = 0;
    storedCommands_ = 0;

    maxcachesize_ = 0;   // maximum of totalSessions
    maxqueuesize_ = 0;   // maximum length of one session command queue
    maxcommands_ = 0;    // maximum of storedCommands

    smsc_log_info( log_, "init path=%s, name=%s, indexgrowth=%d, pagesize=%d, prealloc=%d",
                   path.c_str(), name.c_str(), indexgrowth, pagesize, prealloc );

    cache_.reset( new MemStorage( 0 ) );
    // cache_.reset( new MemStorage(smsc::logger::Logger::getInstance("sess.sto.c")) );

    // disk_.reset( new DiskStorage );
    // const StorageNumbering& n = StorageNumbering::instance();

    // for ( unsigned i = 0; i < n.storages(); ++i ) {
    // if ( n.node( i ) == nodeNumber_ ) {
                
    {
        unsigned i = eltNumber;
        const std::string suffix("-pgf");
        std::auto_ptr<PageFile> pgf( new PageFile );
        char buf[10];
        snprintf( buf, sizeof(buf), "%03u", i );
        const std::string idxstr(buf);
        const std::string realpath = path + "/" + idxstr;
        const std::string fn( realpath + "/" + name + suffix + idxstr + "-data" );
        try {
            pgf->Open( fn );
        } catch (...) {
            pgf->Create( fn, pagesize, prealloc );
        }
            
        std::auto_ptr<EltDiskStorage> eds
            ( new EltDiskStorage
              ( new DiskIndexStorage( name + suffix + idxstr,
                                      realpath, indexgrowth, false,
                                      smsc::logger::Logger::getInstance("sess.sto.i")),
                new DiskDataStorage( pgf.release(),
                                     smsc::logger::Logger::getInstance("sess.sto.d"))));
        // disk_->addStorage( i, eds.release() );
        // smsc_log_debug(log_, "added storage %d", i );
        disk_ = eds;
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


ActiveSession SessionStoreImpl::fetchSession( const SessionKey&           key,
                                              std::auto_ptr<SCAGCommand>& cmd,
                                              bool                        create )
{
    SCAGCommand* cmdaddr = cmd.get();
    if ( ! cmdaddr ) return ActiveSession();

    const char* what = "";
    Session* session = cmd->getSession();
    if ( session ) {
        // fast access to session, w/o locking mutex
        // if cmd has session, it means that session is locked
        // smsc_log_debug(log_, "fetchSession(key=%s,cmd=%p,cmd->serial=%u) => session=%p fast lookup",
        // key.toString().c_str(),
        // cmdaddr, cmd->getSerial(),
        // session );
        if ( session->currentCommand() != cmd->getSerial() ) {
            smsc_log_error(log_, "logic error in fetchSession, cmd=%p, cmd->serial=%u, cmd->session=%p, session->cmd=%u",
                           cmdaddr, cmd->getSerial(),
                           session, session->currentCommand() );
            ::abort();
        }
        what = " fast lookup";
        // session is already locked
        // return makeLockedSession(*session,*cmd.get());
    }

    // session is not attached to the command

    MemStorage::stored_type* v = 0;
    while ( ! session ) { // fake loop
        MutexGuard mg(cacheLock_);
        if ( stopping_ ) break;
        // return ActiveSession();

        v = cache_->get( key );
        if ( ! v ) {
            // not found

        } else {

            session = cache_->store2val(*v);
            
            if ( session ) {

                v = 0;
                // smsc_log_debug( log_, "hit key=%s session=%p session->cmd=%u",
                // key.toString().c_str(), session, session->currentCommand() );

                if ( ! session->currentCommand() ) {
                    ++lockedSessions_;
                    session->setCurrentCommand( cmd->getSerial() );
                    break;
                } else if ( session->currentCommand() == cmd->getSerial() ) {
                    break;
                }

                // session is locked
                what = " is locked";
                // reserve a place in queue for a command
                queue_->pushCommand( cmd.get(), SCAGCommandQueue::RESERVE );
                SCAGCommand* com = cmd.release();
                const unsigned sqsz = session->appendCommand( com );
                // smsc_log_debug(log_, "session is locked, put cmd=%p cmd->serial=%u to session queue, sz=%u",
                // com, com->getSerial(), sz );
                if ( sqsz > maxqueuesize_ ) maxqueuesize_ = sqsz;
                if ( ++storedCommands_ > maxcommands_ ) maxcommands_ = storedCommands_;
                session = 0;
                break;
            }
        }

        // NOTE: session may be flushed out to disk, that's why
        // we check if it is not found after upload from disk.
        // object not found in cache
        // if ( !create ) {
        // what = " is not found";
        // break;
        // }

        // create a stub to be filled by disk io
        cache_->set( key, cache_->val2store( allocator_->alloc(key) ));
        v = cache_->get( key );
        const unsigned sz = cache_->size();
        if ( sz > maxcachesize_ ) maxcachesize_ = sz;

        session = cache_->store2val(*v);
        session->setCurrentCommand( cmd->getSerial() );

        ++totalSessions_;
        ++lockedSessions_;

        // release lock
        break;
    }
    // commands are owned elsewhere
    // delete prev;

    if (v && diskio_) {

        if ( ! disk_->get( key, cache_->store2ref(*v) ) && ! create ) {
            // failure to upload from disk and creation flag is not set
            {
                MutexGuard mg(cacheLock_);
                session = cache_->release( key );
                --totalSessions_;
                --lockedSessions_;
            }
            delete session;
            session = 0;
            what = " is not found";
        }
    }

    // smsc_log_debug( log_, "fetched key=%s session=%p for cmd=%p", key.toString().c_str(), session, cmd.get() );
    smsc_log_debug( log_,"fetchSession(key=%s,cmd=%p,create=%d) => session=%p%s",
                    key.toString().c_str(), cmdaddr, create ? 1:0,
                    session, what );

    if ( session )
        return makeLockedSession(*session,*cmd.get());
    else
        return ActiveSession();
}


void SessionStoreImpl::releaseSession( Session& session )
{
    const SessionKey& key = session.sessionKey();
    {
        uint32_t cmd = session.currentCommand();
        // smsc_log_debug(log_,"releaseSession(session=%p): key=%s, sess->ops=%d, sess->pers=%d, sess->cmd=%u)",
        // &session, key.toString().c_str(), session.operationsCount(), session.isPersistent() ? 1 : 0, cmd );
        if ( ! cmd ) {
            smsc_log_error(log_, "logic error in releaseSession(sess=%p): session->cmd is not set", &session );
            ::abort();
        }
    }

    // debugging printout of the session
    {
        scag_plog_debug(pl,log_);
        session.print(pl);
    }

    bool dostopping = false;
    time_t expiration;

    // SCAGCommand* prevcmd = 0;
    SCAGCommand* nextcmd = 0;
    uint32_t nextuid = 0;
    uint32_t prevuid;
    unsigned tot;
    unsigned lck;
    
    bool needflush = false;
    bool ispersist = false;
    if ( diskio_ ) {
        ispersist = session.getCurrentOperation() &&
            session.getCurrentOperation()->flagSet( OperationFlags::PERSISTENT );
        if ( session.needsFlush() || ispersist ) {
            needflush = true;
            // if ( !ispersist) ispersist = session.hasPersistentOperation();
        }
    }

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

        if ( needflush ) {
            UnlockMutexGuard ug(cacheLock_);
            // if ( ispersist ) {
            disk_->set( key, cache_->store2ref(*v) );
            smsc_log_debug(log_, "flushed key=%s session=%p", key.toString().c_str(), &session );
            // } else {
            // disk_->remove( key );
            // smsc_log_debug(log_, "removed key=%s session=%p", key.toString().c_str(), &session );
            // }
        }
            
        expiration = session.expirationTime();
        nextcmd = session.popCommand();
        if ( nextcmd ) {
            setCommandSession( *nextcmd, &session );
            nextuid = nextcmd->getSerial();
        } else {
            --lockedSessions_;
        }
        prevuid = session.setCurrentCommand( nextuid );

        tot = totalSessions_;
        lck = lockedSessions_;
        // release lock
    }
    // commands are owned elsewhere
    // delete prevcmd;

    smsc_log_debug( log_, "releaseSession(session=%p,key=%s) => prevcmd=%u nextcmd=%u, tot/lck=%u/%u",
                    &session, key.toString().c_str(), prevuid, nextuid, tot, lck );

    if ( ! carryNextCommand(session,nextcmd,true) )
        expiration_->scheduleExpire(expiration,session);

}


void SessionStoreImpl::moveLock( Session& s, SCAGCommand* cmd )
{
    if ( !cmd || !s.currentCommand() ) return;
    if ( s.currentCommand() == cmd->getSerial() ) return; // already set
    Session* olds = cmd->getSession();
    if ( olds && olds != &s ) {
        // another session is set
        smsc_log_warn(log_, "cannot moveLock: session=%p session->cmd=%u cmd=%p cmd->serial=%u cmd->session=%p",
                      &s, s.currentCommand(), cmd, cmd->getSerial(), olds );
        return;
    }
    MutexGuard mg(cacheLock_);
    s.setCurrentCommand( cmd->getSerial() );
}


inline ActiveSession SessionStoreImpl::makeLockedSession( Session&     s,
                                                          SCAGCommand& c )
{
    Session* olds = c.getSession();
    const uint32_t oldc = s.currentCommand();
    setCommandSession(c,&s);
    // should be already set
    // s.setCurrentCommand(&c);
    if ( olds && olds != &s ) {
        smsc_log_error(log_, "logic error in makeLocked: cmd->session=%p session=%p", olds, &s );
        ::abort();
    }
    if ( oldc && oldc != c.getSerial() ) {
        smsc_log_error(log_, "logic error in makeLocked: sess->cmd=%u cmd->serial=%u", oldc, c.getSerial() );
        ::abort();
    }
    // ++lockedSessions_;
    s.setLastAccessTime( time(0) );
    return ActiveSession(*this,s);
}


unsigned SessionStoreImpl::storedCommands() const
{
    MutexGuard mg(cacheLock_);
    return storedCommands_;
}


bool SessionStoreImpl::expireSessions( const std::vector< SessionKey >& expired,
                                       const std::vector< SessionKey >& flush )
{
    assert( expired.size() > 0 || flush.size() > 0 );

    smsc_log_debug( log_, "%u/%u sessions to be expired/flushed", expired.size(), flush.size() );

    time_t now = time(0);
    Session* session = 0;
    std::vector< SessionKey >::const_iterator i = expired.begin();
    std::vector< SessionKey >::const_iterator j = flush.begin();
    unsigned longcall = 0;
    unsigned notexpired = 0;

    bool keep = false; // keep session on disk and destroy
    while ( true ) {

        if ( session ) {

            if ( keep ) {

                // session should be kept on disk.
                // NOTE: we'll save session only if cache is overwhelmed,
                // otherwise the session should be returned to expirationQueue.
                disk_->set( session->sessionKey(), *session );

            } else if ( ! fin_->finalize( *session ) ) {

                // session is taken for long call finalization
                ++longcall;
                session = 0;

            } else {

                // finalization is done, remove the session from disk
                disk_->remove( session->sessionKey() );

            }

        }

        MutexGuard mg(cacheLock_);
            
        if ( session ) {
            if ( ! doSessionFinalization(*session,keep) )
                ++notexpired;
        }

        // take the next session in the list

        std::vector< SessionKey >::const_iterator ij;
        if ( i != expired.end() ) {
            ij = i;
            ++i;
        } else if ( j != flush.end() ) {
            ij = j;
            ++j;
            keep = true;
        } else {
            break;
        }
        const SessionKey& key = *ij;
        // keep = ( std::distance(expired.begin(),i) > expiredCount );

        MemStorage::stored_type* v = cache_->get( key );
        if ( v ) {
            session = cache_->store2val(*v);

        } else if ( keep ) {

            // session is already flushed?
            ++notexpired;
            session = 0;
            continue;

        } else {
            // smsc_log_debug(log_,"key=%s to be expired is not found",
            // key.toString().c_str() );
            cache_->set( key, cache_->val2store( allocator_->alloc(key)) );
            v = cache_->get( key );
            session = cache_->store2val(*v);
            session->setCurrentCommand(2);
            ++totalSessions_;
            {
                UnlockMutexGuard umg(cacheLock_);
                if ( ! disk_->get( key, cache_->store2ref(*v) ) ) v = 0;
            }
            if ( !v ) {
                --totalSessions_;
                delete cache_->release(key);
                ++notexpired;
                session = 0;
                continue;
            } else {
                session->setCurrentCommand(0);
            }
        }


        if ( session->currentCommand() ) {
            // smsc_log_debug(log_,"key=%s session=%p is locked, cmd=%u, skipped",
            // key.toString().c_str(), session, session->currentCommand() );
            ++notexpired;
            session = 0;
            continue;
        }

        // session is not locked

        if ( keep ) {

            // the session was not accessed for too long time?
            SCAGCommand* nextcmd = session->popCommand();
            if ( nextcmd ) {
                ++lockedSessions_;
                setCommandSession( *nextcmd, session );
                session->setCurrentCommand( nextcmd->getSerial() );
                if ( carryNextCommand(*session,nextcmd,false) ) {
                    // activity
                    ++notexpired;
                    session = 0;
                    continue;
                }
            }

        } else {

            // expired session?

            // smsc_log_debug(log_, "expired key=%s session=%p", key.toString().c_str(), session );
            const time_t newexpiration = session->expirationTime();

            // check expiration time again, as it may be prolonged while we were waiting
            if ( ! stopping_ ) {
                if ( now < newexpiration ) {
                    // smsc_log_debug(log_,"key=%s session=%p is not expired yet",
                    // key.toString().c_str(), session );
                    ++notexpired;
                    session = 0;
                    continue;
                }
            }
        }

        // lock session with some fictional command serial to prevent fetching
        // while finalizing
        session->setCurrentCommand( 1 );
        ++lockedSessions_;

    } // while

    // smsc_log_debug( log_, "sessions not expired: %u, longcalled: %u", notexpired, longcall );
    return (notexpired == 0);
}



// return from longcall from SessionManager
void SessionStoreImpl::sessionFinalized( Session& s )
{
    MutexGuard mg(cacheLock_);
    doSessionFinalization(s,false);
}


void SessionStoreImpl::getSessionsCount( unsigned& sessionsCount,
                                         unsigned& sessionsLockedCount ) const
{
    MutexGuard mg(cacheLock_);
    sessionsCount = totalSessions_;
    sessionsLockedCount = lockedSessions_;
}


/// the cache must be locked here
bool SessionStoreImpl::doSessionFinalization( Session& session, bool keep )
{
    if ( session.currentCommand() != 1 ) {
        smsc_log_error(log_, "logic error in session=%p finalization, session->cmd=%u != 1",
                       &session, session.currentCommand() );
        ::abort();
    }
    --lockedSessions_;

    SCAGCommand* nextcmd = session.popCommand();
    if ( nextcmd ) {
        ++lockedSessions_;
        setCommandSession( *nextcmd, &session );
        session.setCurrentCommand( nextcmd->getSerial() );
    } else
        session.setCurrentCommand( 0 );

    if ( carryNextCommand(session,nextcmd,false) ) {
        // cannot delete session
        // NOTE: the session was already finalized (possible via session_destroy rule),
        // so make sure it will be initialized
        // smsc_log_debug(log_, "key=%s session=%p is revived via clear()", session.sessionKey().toString().c_str(), &session );
        if ( !keep ) session.clear();
        return false;
    } else {
        // no more commands
        // smsc_log_debug(log_, "key=%s session=%p is being destroyed", session.sessionKey().toString().c_str(), &session );
        // NOTE: session should be already saved/deleted from disk
        --totalSessions_;
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
            --lockedSessions_;
            session.setCurrentCommand( 0 );
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
