#include <memory>

#include "scag/sessions/base/Session2.h"
#include "scag/sessions/base/Operation.h"
#include "SessionStore2.h"
#include "scag/sessions/base/SessionFinalizer.h"
#include "scag/sessions/base/SessionExpirationQueue.h"
#include "scag/util/UnlockMutexGuard.h"
#include "scag/util/io/Print.h"
#include "core/threads/Thread.hpp"
#include "scag/counter/Manager.h"

namespace scag2 {
namespace sessions {

using namespace transport;
using namespace scag::util::storage;
using scag::util::UnlockMutexGuard;
using smsc::core::threads::Thread;

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
    unsigned pass = 0;
    while ( lockedSessions_->getValue() > 0 ) {
        if ( lockedSessions_->getValue() < 300 && ++pass == 10 ) {
            smsc_log_info( log_, "the following %d sessions are still locked:", lockedSessions_->getValue() );
            scag_plog_info(pl,log_);
            for ( MemStorage::iterator_type i = cache_->begin(); i.next(); ) {
                Session* s = cache_->store2val(i.value());
                if (s) s->print(pl);
            }
        }
        cacheLock_.wait( 100 );
    }
    // smsc_log_debug(log_, "node=%u destroyed", nodeNumber_ );
}


void SessionStoreImpl::init( unsigned eltNumber,
                             SCAGCommandQueue& queue,
                             const std::string& path,
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

    {
        counter::Manager& cntmgr = counter::Manager::getInstance();
        totalSessions_ = cntmgr.getAnyCounter("sys.sessions.total");
        loadedSessions_ = cntmgr.getAnyCounter("sys.sessions.active");
        lockedSessions_ = cntmgr.getAnyCounter("sys.sessions.locked");
        if (!totalSessions_.get() || !loadedSessions_.get() || !lockedSessions_.get())
            throw std::runtime_error("cannot create session counters");
    }
    storedCommands_ = 0;

    maxcachesize_ = 0;   // maximum of loadedSessions
    maxqueuesize_ = 0;   // maximum length of one session command queue
    maxcommands_ = 0;    // maximum of storedCommands

    smsc_log_info( log_, "#%u init path=%s, indexgrowth=%d, pagesize=%d, prealloc=%d",
                   eltNumber, path.c_str(), indexgrowth, pagesize, prealloc );

    cache_.reset( new MemStorage( 0 ) );
    // cache_.reset( new MemStorage(smsc::logger::Logger::getInstance("sess.sto.c")) );

    // disk_.reset( new DiskStorage );
    // const StorageNumbering& n = StorageNumbering::instance();

    // for ( unsigned i = 0; i < n.storages(); ++i ) {
    // if ( n.node( i ) == nodeNumber_ ) {
                
    {
        unsigned i = eltNumber;
        const std::string suffix("pgf");
        std::auto_ptr<PageFile> pgf( new PageFile );
        char buf[10];
        snprintf( buf, sizeof(buf), "%03u", i );
        const std::string idxstr(buf);
        const std::string realpath = path + "/" + idxstr;
        const std::string fn( realpath + "/" + suffix + idxstr + "-data" );
        bool ok = false;
        bool opened = false;
        try {
            pgf->Open( fn );
            ok = opened = true;
        } catch (...) {
            // check that directory exist
            struct stat st;
            if ( stat(realpath.c_str(), &st) ) {
                // not found?
                if ( mkdir( realpath.c_str(), 0777 ) ) {
                    smsc_log_error( log_, "#%u cannot create a directory %s", eltNumber, realpath.c_str() );
                }
            } else if ( ! S_ISDIR(st.st_mode) ) {
                smsc_log_error( log_, "#%u path %s exists but is not a directory", eltNumber, realpath.c_str() );
                ::abort();
            }
        }

        try {
            if ( ! ok ) pgf->Create( fn, pagesize, prealloc );
            ok = true;
        } catch ( std::exception& e ) {
            smsc_log_error( log_, "#%u cannot open/create file %s: %s", eltNumber, fn.c_str(), e.what() );
        } catch (...) {
            smsc_log_error( log_, "#%u cannot open/create file %s: unknown error", eltNumber );
        }
        if ( ! ok ) ::abort();
            
        DiskIndexStorage* di = new DiskIndexStorage( suffix + idxstr, 
                                                     realpath, 
                                                     indexgrowth,
                                                     ! opened,  // cleanup (should be true if failed to open datafile)
                                                     smsc::logger::Logger::getInstance("sess.sto.i") );

        std::auto_ptr<EltDiskStorage> eds
            ( new EltDiskStorage
              ( di,
                new DiskDataStorage( pgf.release(),
                                     smsc::logger::Logger::getInstance("sess.sto.d"))));

        if ( opened ) {
            // collect all keys from disk storage
            for ( DiskIndexStorage::iterator_type ii(di->begin());
                  ii.next();
                  ) {
                if ( ii.idx() != di->invalidIndex() )
                    initialkeys_.push_back( ii.key() );
            }
            smsc_log_info( log_, "#%u has %u initial sessions",
                           eltNumber,
                           unsigned(initialkeys_.size()) );
            // totalSessions_ = unsigned(initialkeys_.size());
            totalSessions_->setValue(initialkeys_.size());
        }

        // disk_->addStorage( i, eds.release() );
        // smsc_log_debug(log_, "added storage %d", i );
        disk_ = eds;
    }
    stopping_ = false;
}


/// stop processing
void SessionStoreImpl::stop()
{
    // smsc_log_info(log_, "stop()");
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
    int sqsz=-1;
    Session* session = cmd->getSession();
    if ( session ) {
        // fast access to session, w/o locking mutex
        // if cmd has session, it means that session is locked
        // smsc_log_debug(log_, "fetchSession(key=%s,cmd=%p,cmd->serial=%u) => session=%p fast lookup",
        // key.toString().c_str(),
        // cmdaddr, cmd->getSerial(),
        // session );
        if ( session->currentCommand() != cmd->getSerial() ) {
            smsc_log_error(log_, "logic error in fetchSession, cmd=%p, cmd->serial=%u, cmd->session=%p/%s, session->cmd=%u",
                           cmdaddr, cmd->getSerial(),
                           session, key.toString().c_str(),
                           session->currentCommand() );
            ::abort();
        }
        what = " fast lookup";
        // session is already locked
        // return makeLockedSession(*session,*cmd.get());
    }

    // session is not attached to the command

    MemStorage::stored_type* v = 0;
    bool firsttime; // a flag if session should be loaded for the first time
    while ( ! session ) { // fake loop
        MutexGuard mg(cacheLock_);
        if ( stopping_ ) {
            what = " store is stopped";
            break;
        }
        // return ActiveSession();

        v = cache_->get( key );
        if ( ! v ) {
            // not found
            firsttime = true;

        } else {
            firsttime = false;

            session = cache_->store2val(*v);
            
            if ( session ) {

                sqsz = int(session->commandCount());
                v = 0;
                // smsc_log_debug( log_, "hit key=%s session=%p session->cmd=%u",
                // key.toString().c_str(), session, session->currentCommand() );

                if ( ! session->currentCommand() ) {
                    // ++lockedSessions_;
                    lockedSessions_->increment();
                    session->setCurrentCommand( cmd->getSerial() );
                    what = " was free";
                    break;
                } else if ( session->currentCommand() == cmd->getSerial() ) {
                    what = " already mine";
                    break;
                }

                // session is locked
                what = " is locked";
                // reserve a place in queue for a command
                queue_->pushCommand( cmd.get(), SCAGCommandQueue::RESERVE );
                ++sqsz;
                SCAGCommand* com = cmd.release();
                session->appendCommand( com );
                // smsc_log_debug(log_, "session is locked, put cmd=%p cmd->serial=%u to session queue, sz=%u",
                // com, com->getSerial(), sz );
                if ( unsigned(sqsz) > maxqueuesize_ ) maxqueuesize_ = unsigned(sqsz);
                if ( ++storedCommands_ > maxcommands_ ) maxcommands_ = storedCommands_;
                session = 0;
                break;
            }
        }

        // object not found in cache
        sqsz = 0;

        if ( !create && !diskio_ ) {
            // creation and diskio are not allowed
            what = " is not found";
            v = 0;
            session = 0;
            break;
        }

        // NOTE: session may be flushed out to disk, that's why
        // we check if it is not found after upload from disk.

        // if ( !v ) {
        // create a stub to be filled by disk io
        if ( !v ) {
            cache_->set( key, cache_->val2store( allocator_->alloc(key) ));
            v = cache_->get( key );
        } else {
            *v = cache_->val2store( allocator_->alloc(key) );
        }
        const unsigned sz = cache_->size();
        if ( sz > maxcachesize_ ) maxcachesize_ = sz;

        session = cache_->store2val(*v);
        session->setCurrentCommand( cmd->getSerial() );

        // ++loadedSessions_;
        //++lockedSessions_;
        loadedSessions_->increment();
        lockedSessions_->increment();

        // release lock
        break;
    }
    // commands are owned elsewhere
    // delete prev;

    if ( v && diskio_ ) {

        if ( ! disk_->get( key, cache_->store2ref(*v) ) ) {
            // session is not found on disk
            {
                MutexGuard mg(cacheLock_);
                if ( ! create ) {
                    // creation is forbidden
                    session = cache_->release( key );
                    //--loadedSessions_;
                    //--lockedSessions_;
                    loadedSessions_->increment(-1);
                    lockedSessions_->increment(-1);
                } else {
                    // ++totalSessions_;
                    totalSessions_->increment();
                }
            }
            if ( !create ) {
                // failure to upload from disk and creation flag is not set
                delete session;
                session = 0;
                what = " is not found";
            } else {
                what = " created";
            }
        } else if ( session->expirationTime() < time(0) ) {
            what = " expired on disk";
            queue_->pushCommand( cmd.get(), SCAGCommandQueue::RESERVE );
            SCAGCommand* com = cmd.release();
            { 
                MutexGuard mg(cacheLock_);
                sqsz = session->appendCommand( com );
                session->setCurrentCommand( 1 ); // expiration command
            }
            expiration_->scheduleExpire( session->expirationTime(), session->lastAccessTime(), key );
            session = 0;
        } else {
            // session was just uploaded and is not expired
            if ( firsttime && ! session->hasPersistentOperation() ) {
                // session was not persistent
                session->clear();
                what = " upload&clear";
            } else {
                what = " uploaded";
            }
        }
    }

    // smsc_log_debug( log_, "fetched key=%s session=%p for cmd=%p", key.toString().c_str(), session, cmd.get() );
    smsc_log_debug( log_,"fetchSession(key=%s,cmd=%p,create=%d) => session=%p qsz=%d%s",
                    key.toString().c_str(), cmdaddr, create ? 1:0, session, sqsz, what );

    if ( session )
        return makeLockedSession(*session,*cmd.get());
    else
        return ActiveSession();
}


void SessionStoreImpl::releaseSession( Session& session )
{
    // NOTE: key should not be a reference, as session may be destroyed at the end of the method
    const SessionKey key = session.sessionKey();

    {
        uint32_t cmd = session.currentCommand();
        // smsc_log_debug(log_,"releaseSession(session=%p): key=%s, sess->ops=%d, sess->pers=%d, sess->cmd=%u)",
        // &session, key.toString().c_str(), session.operationsCount(), session.isPersistent() ? 1 : 0, cmd );
        if ( ! cmd ) {
            smsc_log_error(log_, "logic error in releaseSession(session=%p/%s): session->cmd is not set",
                           &session, key.toString().c_str() );
            ::abort();
        }
    }

    // debugging printout of the session while it is locked
    {
        scag_plog_debug(pl,log_);
        session.print(pl);
    }

    // NOTE: this is ugly to clear lcm in store
    if ( session.getLongCallContext().continueExec ) {
        smsc_log_warn( log_, "session=%p/%s has lcm continue set",
                       &session, key.toString().c_str() );
        session.getLongCallContext().continueExec = false;
    }

    time_t expiration, lastaccess = time(0);
    session.setLastAccessTime(lastaccess);

    // SCAGCommand* prevcmd = 0;
    SCAGCommand* nextcmd = 0;
    uint32_t nextuid = 0;
    uint32_t prevuid;
    unsigned tot, ldd, lck;
    
    bool needflush = false;
    if ( diskio_ ) {
        bool ispersist = session.getCurrentOperation() &&
            session.getCurrentOperation()->flagSet( OperationFlags::PERSISTENT );
        if ( session.needsFlush() || ispersist ) {
            needflush = true;
            // if ( !ispersist) ispersist = session.hasPersistentOperation();
        }
    }

    {
        MemStorage::stored_type* v;
        MutexGuard mg(cacheLock_);

        v = cache_->get( key );
        if ( !v || cache_->store2val(*v) != &session ) {
            smsc_log_error( log_, "logic error in releaseSession(session=%p/%s): not found in cache",
                            &session, key.toString().c_str() );
            ::abort();
            // throw std::runtime_error("SessionStore: logic error in releaseSession" );
        }

        if ( needflush ) {
            UnlockMutexGuard ug(cacheLock_);
            // if ( ispersist ) {
            disk_->set( key, cache_->store2ref(*v) );
            smsc_log_debug(log_, "session=%p/%s is flushed", &session, key.toString().c_str() );
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
            // --lockedSessions_;
            lockedSessions_->increment(-1);
        }
        prevuid = session.setCurrentCommand( nextuid );

        tot = totalSessions_->getValue();
        ldd = loadedSessions_->getValue();
        lck = lockedSessions_->getValue();
        // release lock
    }
    // commands are owned elsewhere
    // delete prevcmd;

    smsc_log_debug( log_, "releaseSession(session=%p/%s) => prevcmd=%u nextcmd=%u, tot/ldd/lck=%u/%u/%u",
                    &session, key.toString().c_str(), prevuid, nextuid, tot, ldd, lck );

    if ( ! (nextcmd && carryNextCommand( session, nextcmd, true)) )
        expiration_->scheduleExpire(expiration,lastaccess,key);
}


void SessionStoreImpl::moveLock( Session& s, SCAGCommand* cmd )
{
    if ( !cmd || !s.currentCommand() ) return;
    if ( s.currentCommand() == cmd->getSerial() ) return; // already set
    Session* olds = cmd->getSession();
    if ( olds && olds != &s ) {
        // another session is set
        smsc_log_warn(log_, "cannot moveLock: session=%p/%s session->cmd=%u cmd=%p cmd->serial=%u cmd->session=%p",
                      &s, s.sessionKey().toString().c_str(), s.currentCommand(), cmd, cmd->getSerial(), olds );
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
        smsc_log_error(log_, "logic error in makeLocked(session=%p/%s): cmd->session=%p",
                       &s, s.sessionKey().toString().c_str(), olds );
        ::abort();
    }
    if ( oldc && oldc != c.getSerial() ) {
        smsc_log_error(log_, "logic error in makeLocked(session=%p/%s): sess->cmd=%u cmd->serial=%u",
                       &s, s.sessionKey().toString().c_str(), oldc, c.getSerial() );
        ::abort();
    }
    s.setLastAccessTime( time(0) );
    return ActiveSession(*this,s);
}


/*
unsigned SessionStoreImpl::storedCommands() const
{
    MutexGuard mg(cacheLock_);
    return storedCommands_;
}
 */


bool SessionStoreImpl::expireSessions( const std::vector< SessionKey >& expired,
                                       const std::vector<std::pair<SessionKey,time_t> >& flush )
{
    assert( expired.size() > 0 || flush.size() > 0 );

    smsc_log_debug( log_, "%u/%u sessions to be expired/flushed", expired.size(), flush.size() );

    time_t now = time(0);
    Session* session = 0;
    std::vector< SessionKey >::const_iterator i = expired.begin();
    std::vector< std::pair<SessionKey,time_t> >::const_iterator j = flush.begin();
    // unsigned longcall = 0;
    unsigned notexpired = 0;

    bool keep = false; // keep session on disk and destroy
    bool finw = true;   // waiting finalization (only if keep = false)
    unsigned count = 0;
    while ( true ) {

        if ( session ) {

            if ( keep ) {
                
                if ( !stopping_ || session->needsFlush() || session->hasPersistentOperation() ) {

                    // session should be kept on disk.
                    smsc_log_debug( log_, "session=%p/%s is being flushed",
                                    session, session->sessionKey().toString().c_str() );
                    scag_plog_debug(pl,log_);
                    session->print(pl);
                    disk_->set( session->sessionKey(), *session );

                } else {
                    smsc_log_debug( log_, "session=%p/%s is not flushed",
                                    session, session->sessionKey().toString().c_str() );
                }

            } else if ( finw && ! fin_->finalize( *session ) ) {

                // session is taken for long call finalization
                // ++longcall;
                session = 0;

            } else {

                // finalization is done, remove the session from disk
                if ( diskio_ ) {
                    smsc_log_debug( log_, "session=%p/%s is being erased from disk",
                                    session, session->sessionKey().toString().c_str() );
                    disk_->remove( session->sessionKey() );
                }

            }

        }
        
        if ( session && ( ++count % 50 == 0 ) ) Thread::Yield();

        MutexGuard mg(cacheLock_);
            
        if ( session ) {
            if ( ! doSessionFinalization(*session,keep) )
                ++notexpired;
        }

        // take the next session in the list

        const SessionKey* pkey;
        time_t lastaccess;
        if ( i != expired.end() ) {
            pkey = &(*i);
            finw = true;
            ++i;
        } else if ( j != flush.end() ) {
            if ( ! diskio_ ) break;
            pkey = &(j->first);
            lastaccess = j->second;
            ++j;
            keep = true;
            finw = false;
        } else {
            break;
        }
        const SessionKey& key = *pkey;

        MemStorage::stored_type* v = cache_->get( key );
        session = v ? cache_->store2val(*v) : 0;
        if ( session ) {
            // ok
            if ( stopping_ ) keep = true;
        } else if ( stopping_ ) {
            // storage is stopping and session is not found, ok
            continue;

        } else if ( keep ) {
            // session is already flushed -- ok
            continue;

        } else if ( diskio_ ) {
            // session is asked to be expired, but is not found in cache:
            // was it flushed to disk?
            bool firsttime = bool(!v);
            if ( !v ) {
                cache_->set( key, cache_->val2store( allocator_->alloc(key)) );
                v = cache_->get( key );
            } else {
                *v = cache_->val2store( allocator_->alloc(key) );
            }
            session = cache_->store2val(*v);
            session->setCurrentCommand(2);
            // ++loadedSessions_;
            loadedSessions_->increment();
            bool uploaded;
            {
                UnlockMutexGuard umg(cacheLock_);
                smsc_log_debug(log_,"key=%s to be expired is not found",
                               key.toString().c_str() );
                uploaded = disk_->get( key, cache_->store2ref(*v) );
                if ( ! uploaded ) {
                    smsc_log_debug( log_, "session key=%s cannot be uploaded",
                                   key.toString().c_str() );
                } else {
                    smsc_log_debug( log_,"session=%p/%s has been just uploaded",
                                    session, key.toString().c_str() );
                    if ( firsttime && ! session->hasPersistentOperation() ) {
                        smsc_log_debug( log_, "... but session is not persistent" );
                        finw = false;
                    } else {
                        scag_plog_debug(pl,log_);
                        session->print(pl);
                    }
                }
            }
            if ( ! uploaded ) totalSessions_->increment(); // ++totalSessions_;
            session->setCurrentCommand(0);
        } else {
            continue;
        }


        if ( session->currentCommand() == 1 ) {
            // already in finalization stage, just returned from longcall
            continue;
        }

        if ( session->currentCommand() ) {
            // smsc_log_debug(log_,"key=%s session=%p is locked, cmd=%u, skipped",
            // key.toString().c_str(), session, session->currentCommand() );
            ++notexpired;
            session = 0;
            continue;
        }

        // session is not locked

        if ( stopping_ ) {
            // we are stopping the session should be dropped to disk
            // keep = true;

        } else if ( keep ) {
            // was the session not accessed for too long time?

            const time_t newlastaccess = session->lastAccessTime();
            if ( lastaccess != newlastaccess ) {
                ++notexpired;
                session = 0;
                continue;
            }

            SCAGCommand* nextcmd = session->popCommand();
            if ( nextcmd ) {
                // a command has appeared on the queue
                // ++lockedSessions_;
                lockedSessions_->increment();
                setCommandSession( *nextcmd, session );
                session->setCurrentCommand( nextcmd->getSerial() );
                if ( carryNextCommand(*session,nextcmd,false) ) {
                    // some activity on this session
                    ++notexpired;
                    session = 0;
                    continue;
                }
            }

            const time_t newexpiration = session->expirationTime();
            if ( now+5 > newexpiration ) {
                // too short interval until expiration, so don't flush on disk
                ++notexpired;
                session = 0;
                continue;
            }

        } else {

            // expired session?
            const time_t newexpiration = session->expirationTime();

            // smsc_log_debug(log_, "expired key=%s session=%p", key.toString().c_str(), session );

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
        // ++lockedSessions_;
        lockedSessions_->increment();

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
                                         unsigned& sessionsLoadedCount,
                                         unsigned& sessionsLockedCount ) const
{
    MutexGuard mg(cacheLock_);
    sessionsCount = totalSessions_->getValue();
    sessionsLoadedCount = loadedSessions_->getValue();
    sessionsLockedCount = lockedSessions_->getValue();
}


bool SessionStoreImpl::uploadInitial( unsigned cnt )
{
    if ( ! initialkeys_.empty() ) {

        std::vector< SessionKey > keys;
        {
            unsigned count(std::min(cnt,unsigned(initialkeys_.size())));
            std::vector< DiskIndexStorage::storedkey_type >::iterator i = initialkeys_.begin() + count;
            keys.reserve( count );
            for ( std::vector< DiskIndexStorage::storedkey_type >::const_iterator j = initialkeys_.begin();
                  j != i;
                  ++j ) {
                keys.push_back( *j );
            }
            initialkeys_.erase( initialkeys_.begin(), i );
        }

        std::vector< SessionKey >::const_iterator i = keys.begin();
        SessionKey key;
        MemStorage::stored_type v = cache_->val2store(0);
        while ( true ) {

            time_t now = time(0);

            Session* session = cache_->store2val(v);
            if ( session ) {

                if ( disk_->get( key, cache_->store2ref(v) ) )
                {
                    // loaded
                    smsc_log_debug(log_, "uploaded session=%p/%s has expire=%d",
                                   session, key.toString().c_str(), int(session->expirationTime()-now));
                    // scag_plog_debug(pl,log_);
                    // session->print(pl);
                    expiration_->scheduleExpire( session->expirationTime(),
                                                 session->lastAccessTime(),
                                                 key );
                }
                cache_->dealloc(v);
            }

            if ( i == keys.end() ) break;

            key = *i;
            ++i;

            MutexGuard mg(cacheLock_);
            // if ( session ) {
            // --totalSessions_;
            // --lockedSessions_;
            // }
            if ( stopping_ ) return false;

            MemStorage::stored_type* vv = cache_->get( key );
            if ( vv ) {
                // already in cache
                v = cache_->val2store(0);
                continue;
            }
            v = cache_->val2store( allocator_->quietalloc(key) );
            // ++totalSessions_;
            // ++lockedSessions_;
        } // while loop over session keys
    } // if the list of keys is not empty
    return ! initialkeys_.empty();
}


/// the cache must be locked here
bool SessionStoreImpl::doSessionFinalization( Session& session, bool keep )
{
    if ( session.currentCommand() != 1 ) {
        smsc_log_error(log_, "logic error in session=%p/%s finalization, session->cmd=%u != 1",
                       &session, session.sessionKey().toString().c_str(), session.currentCommand() );
        ::abort();
    }
    // --lockedSessions_;
    lockedSessions_->increment(-1);

    SCAGCommand* nextcmd = session.popCommand();
    if ( nextcmd ) {
        // ++lockedSessions_;
        lockedSessions_->increment();
        setCommandSession( *nextcmd, &session );
        session.setCurrentCommand( nextcmd->getSerial() );
        // session has been already finalized, so clear it up
        if (!keep) session.clear();
        carryNextCommand( session, nextcmd, false );
        return false;
    } else {
        // no more commands, delete the session
        // --loadedSessions_;
        loadedSessions_->increment(-1);
        if (keep) {
            // NOTE: we leave a stub in cache as a flag that session has been processed in this run
            MemStorage::stored_type* v = cache_->get( session.sessionKey() );
            __require__( v );
            cache_->dealloc(*v);
            *v = cache_->val2store(0);
        } else {
            // --totalSessions_;
            totalSessions_->increment(-1);
            delete cache_->release( session.sessionKey() );
        }
        return true;
    }
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
            // --lockedSessions_;
            lockedSessions_->increment(-1);
            session.setCurrentCommand( 0 );
        } catch (...) {
        }
        if ( dolock ) cacheLock_.Unlock();
    }
    
    smsc_log_info( log_, "queue is stopped: session=%p/%s deletes %u commands",
                   &session, session.sessionKey().toString().c_str(), comlist.size() );
    for ( std::vector< SCAGCommand* >::iterator i = comlist.begin();
          i != comlist.end();
          ++i ) {
        queue_->pushCommand( *i, SCAGCommandQueue::MOVE );
    }
    return false;

} // carryNextCommand

} // namespace sessions
} // namespace scag2
