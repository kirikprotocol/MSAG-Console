#include <memory>

#include "Session2.h"
#include "SessionStore2.h"
#include "SessionFinalizer.h"
#include "SessionExpirationQueue.h"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "scag/util/ThreadWrap.h"
#include "scag/util/storage/HashedMemoryCache.h"
#include "scag/util/storage/PageFileDiskStorage.h"
#include "scag/util/storage/RBTreeIndexStorage.h"
#include "scag/util/storage/StorageIface.h"
#include "scag/util/storage/CompositeDiskStorage.h"

namespace {

    using namespace scag::sessions2;
    using namespace scag::transport2;
    using namespace scag::util::storage;

    class UnlockGuard
    {
        UnlockGuard( const UnlockGuard& g );
    public:
        UnlockGuard( Mutex& m ) : m_(m) { m_.Unlock(); }
        ~UnlockGuard() { m_.Lock(); }
    private:
        mutable Mutex& m_;
    };


    /*
    // FIXME: temporary for tests SlowMutex
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


    typedef HashedMemoryCache< SessionKey, Session > MemStorage;
    typedef PageFileDiskStorage< SessionKey, Session > DiskDataStorage;
    typedef RBTreeIndexStorage< SessionKey, DiskDataStorage::index_type > DiskIndexStorage;
    typedef IndexedStorage< DiskIndexStorage, DiskDataStorage, smsc::core::synchronization::Mutex > EltDiskStorage;
    // typedef IndexedStorage< DiskIndexStorage, DiskDataStorage, SlowMutex > EltDiskStorage;
    typedef CompositeDiskStorage< EltDiskStorage > DiskStorage;

    struct Allocator {
        Session* alloc( const SessionKey& k ) { return new Session(k); }
    };


    class SessionStoreImpl : public SessionStore
    {
    public:
        /// create a storage
        SessionStoreImpl( SessionFinalizer& fin,
                          SessionExpirationQueue& exq,
                          Allocator* a );

        virtual ~SessionStoreImpl();

        /// unlock storage, clear stop flag, etc.
        virtual void init( unsigned nodeNumber,
                           SCAGCommandQueue& queue,
                           const std::string& path = "sessions",
                           const std::string& name = "sessions",
                           unsigned indexgrowth = 10000,
                           unsigned pagesize = 512,
                           unsigned prealloc = 0 );

        virtual void stop();

        virtual ActiveSession fetchSession( const SessionKey& key, SCAGCommand* cmd );

        virtual void releaseSession( Session& s, bool flush );

        virtual void moveLock( Session& s, SCAGCommand* cmd );

        virtual unsigned storedCommands() const;

        virtual void expireSessions( const std::vector< ExpireData >& expired,
                                     std::vector< ExpireData >& notyetexpired );

        virtual void getSessionsCount( unsigned& sessionsCount,
                                       unsigned& sessionsLockedCount ) const;

    protected:

        ActiveSession makeLockedSession( Session& s, SCAGCommand& c );

        bool carryNextCommand( Session& s, SCAGCommand* cmd, bool dolock );

    private:
        unsigned                    nodeNumber_; // for composite storage

        SessionFinalizer*           fin_;
        SessionExpirationQueue*     expiration_;
        SCAGCommandQueue*           queue_;

        // lock access to cache_, cache_->get(key)->state, stopping_ flag
        mutable EventMonitor        cacheLock_;
        std::auto_ptr<MemStorage>   cache_;
        bool                        stopping_;

        std::auto_ptr<DiskStorage>  disk_;
        std::auto_ptr<Allocator>    allocator_;

        // EventMonitor                expireMonitor_;
        // std::auto_ptr<ExpireSet>    expireSet_;

        smsc::logger::Logger*       log_;
        // std::auto_ptr<smsc::core::threads::Thread> expireThread_;

        // statstics
        unsigned                    totalSessions_;
        unsigned                    lockedSessions_;
        unsigned                    storedCommands_;

        unsigned                    maxqueuesize_;
        unsigned                    maxcachesize_;
        unsigned                    maxcommands_;
    };


    SessionStoreImpl::SessionStoreImpl( SessionFinalizer& fin,
                                        SessionExpirationQueue& exq,
                                        Allocator* a ) :
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

        // create a new memstorage, diskstorage
        // const std::string path = getString(cfg, "path", "sessions" );
        // const std::string name = getString(cfg, "name", "sessionStore" );
        // const int32_t indexgrowth = getInt(cfg, "indexgrowth", 100 );
        // const int32_t pagesize    = getInt(cfg, "pagesize", 512 );
        // const int32_t prealloc    = getInt(cfg, "prealloc", 0 );
        smsc_log_info(log_, "init path=%s, name=%s, indexgrowth=%d, pagesize=%d, prealloc=%d",
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
/*
        expireSet_.reset(new ExpireSet);
        expireThread_.reset( new scag::util::ThreadWrap<SessionStoreImpl>
                             ( *this, &SessionStoreImpl::expireThread ) );
        expireThread_->Start();
 */
    }


    /// stop processing
    void SessionStoreImpl::stop()
    {
        smsc_log_info(log_, "stop()");
        MutexGuard mg(cacheLock_);
        stopping_ = true;
    }
    /*
            printf( "session store: maxqueuesize=%u maxcachesize=%u maxstoredcmds=%u\n", maxqueuesize_, maxcachesize_, maxcommands_ );
            smsc_log_info( log_, "maxqueuesize=%u maxcachesize=%u maxstoredcmds=%u", maxqueuesize_, maxcachesize_, maxcommands_ );
            if ( ! wait ) return;

            // wait until there are locked sessions
            int passes = 0;
            do {

                bool haslocked = false;
                // check that all session have been freed
                for ( MemStorage::iterator_type i(cache_->begin()); i.next(); ) {
                    Session* s = cache_->store2val( i.value() );
                    SCAGCommand* cmd = s->currentCommand();
                    if ( cmd ) {
                        smsc_log_debug( log_, "storage is still locked: key=%s session=%p cmd=%p",
                                        i.key().toString().c_str(), s, cmd );
                        haslocked = true;
                        break;
                    }
                }
                if ( ! haslocked ) break;
                cacheLock_.wait(100);

            } while ( ++passes < 200 );
            if ( passes >= 200 ) {
                smsc_log_error( log_, "logic error in stop: dead lock?");
                ::abort();
            }
        }


        {   // notify expire thread, and wait for when it stopped
            MutexGuard mg(expireMonitor_);
            while ( expireSet_.get() && expireSet_->size() > 0 ) {
                expireMonitor_.notify();
                expireMonitor_.wait(100);
            }
            smsc_log_info( log_, "expire pool is destroyed" );
            expireSet_.reset(0);
            expireMonitor_.notify();
        }
        expireThread_->WaitFor();
    }
     */



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
            smsc_log_debug(log_,"releaseSession(key=%s,session=%p,cmd=%p,flush=%d)",
                           key.toString().c_str(), &session, cmd, flush?1:0 );
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
            expiration = session.expirationTime();

            if ( flush ) {

                UnlockGuard ug(cacheLock_);
                disk_->set( key, cache_->store2ref(*v) );
                smsc_log_debug(log_, "flushed key=%s session=%p", key.toString().c_str(), &session );

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


    void SessionStoreImpl::expireSessions( const std::vector< ExpireData >& expired,
                                           std::vector< ExpireData >& notyetexpired )
    {
        notyetexpired.clear();
        if ( expired.size() == 0 ) return;

        notyetexpired.reserve( expired.size() );
        smsc_log_debug( log_, "%u sessions to be expired", expired.size() );

        time_t now = time(0);
        Session* session = 0;
        std::vector< ExpireData >::const_iterator i = expired.begin();
        while ( true ) {

            if ( session ) {
                smsc_log_debug(log_, "finalizing key=%s session=%p", session->sessionKey().toString().c_str(), session );
                // it may take quite long time
                fin_->finalize( *session );
            }

            MutexGuard mg(cacheLock_);
            
            if ( session ) {

                // check if there are some more commands
                SCAGCommand* nextcmd = session->popCommand();
                session->setCurrentCommand( nextcmd );
                if ( nextcmd )
                    setCommandSession(*nextcmd, session);

                if ( carryNextCommand(*session,nextcmd,false) ) {
                    // cannot delete session, return it back to not expired
                    // FIXME: what about initialization ?
                    notyetexpired.push_back( ExpireData(session->expirationTime(),session->sessionKey()) );
                } else {
                    // no more commands
                    delete cache_->release( session->sessionKey() );
                }

            }
            

            if ( i == expired.end() ) break;

            const SessionKey& key = i->key;
            MemStorage::stored_type* v = cache_->get( key );
            if ( !v ) {
                // smsc_log_warn(log_,"key=%s to be expired is not found, sz=%u", key.toString().c_str(), curset.size() );
                continue;
            }

            session = cache_->store2val(*v);
            // smsc_log_debug(log_, "expired key=%s session=%p", key.toString().c_str(), session );
            const time_t newexpiration = session->expirationTime();

            if ( session->currentCommand() ) {
                smsc_log_debug(log_,"key=%s session=%p is not free, cmd=%p, skipped",
                               key.toString().c_str(), session, session->currentCommand() );
                notyetexpired.push_back( ExpireData(newexpiration,key) );
                session = 0;
                continue;
            }

            // check expiration time again, as it may be prolonged while we were waiting
            if ( ! stopping_ ) {
                if ( now < newexpiration ) {
                    smsc_log_debug(log_,"key=%s session=%p is not expired yet",
                                   key.toString().c_str(), session );
                    notyetexpired.push_back( ExpireData(newexpiration,key) );
                    session = 0;
                    continue;
                }
            }

            // lock session with some fictional command to prevent fetching
            // while finalizing
            session->setCurrentCommand( reinterpret_cast<SCAGCommand*>(-1) );

        } // while

        if ( notyetexpired.size() > 0 )
            smsc_log_debug( log_, "%u sessions are not yet expired", notyetexpired.size() );
    }


    void SessionStoreImpl::getSessionsCount( unsigned& sessionsCount,
                                             unsigned& sessionsLockedCount ) const
    {
        MutexGuard mg(cacheLock_);
        sessionsCount = totalSessions_;
        sessionsLockedCount = lockedSessions_;
    }


    // executed in a separate thread
    /*
    int SessionStoreImpl::expireThread()
    {
        const int tmo = 1000;
        MutexGuard mg(expireMonitor_);
        smsc_log_debug(log_,"node=%u expire thread started", nodeNumber_ );
        bool dostopping = false;
        while ( expireSet_.get() ) {

            // FIXME: should we calculate the next wake time?
            // the answer is yes, but we should take minimum
            // of the calculated time and some rather small timeout.
            int curtmo = tmo;
            if ( expireSet_->size() > 0 ) {
                int next = expireSet_->begin()->expiration - time(0);
                if ( next < 0 ) next = 0;
                if ( curtmo > next ) curtmo = next;
            }

            if ( dostopping ) curtmo = 100;
            if ( curtmo > 0 ) expireMonitor_.wait( curtmo );
            if ( ! expireSet_.get() ) break;

            const time_t curtime = time(0);

            ExpireSet curset;
            if ( dostopping ) {
                curset = *expireSet_;
                *expireSet_ = ExpireSet();
                smsc_log_debug(log_, "taking the whole expire set, sz=%u", curset.size() );
            } else {
                ExpireData d( curtime, SessionKey() );
                ExpireSet::iterator i = expireSet_->lower_bound(d);
                curset = ExpireSet( expireSet_->begin(), i );
                expireSet_->erase( expireSet_->begin(), i );
            }

            std::vector< ExpireData > newset;
            {
                UnlockGuard ug( expireMonitor_ );
                newset.reserve( curset.size() );

                MutexGuard cmg(cacheLock_);
                dostopping = stopping_;

                if ( curset.size() > 0 ) smsc_log_debug(log_, "%u sessions expired", curset.size() );

                for ( ExpireSet::iterator i = curset.begin();
                      i != curset.end();
                      ++i ) {

                    const SessionKey& key = i->key;
                    MemStorage::stored_type* v = cache_->get( key );
                    if ( !v ) {
                        // smsc_log_warn(log_,"key=%s to be expired is not found, sz=%u", key.toString().c_str(), curset.size() );
                        continue;
                    }

                    Session* session = cache_->store2val(*v);
                    // smsc_log_debug(log_, "expired key=%s session=%p", key.toString().c_str(), session );
                    const time_t newexpiration = session->expirationTime();

                    if ( session->currentCommand() ) {
                        smsc_log_debug(log_,"key=%s session=%p is not free, cmd=%p, skipped",
                                       key.toString().c_str(), session, session->currentCommand() );
                        newset.push_back( ExpireData(newexpiration,key) );
                        continue;
                    }

                    // check expiration time again, as it may be prolonged while we were waiting
                    if ( ! stopping_ ) {
                        if ( curtime < newexpiration ) {
                            smsc_log_debug(log_,"key=%s session=%p is not expired yet",
                                           key.toString().c_str(), session );
                            newset.push_back( ExpireData(newexpiration,key) );
                            continue;
                        }
                    }

                    smsc_log_debug(log_, "finalizing key=%s session=%p", key.toString().c_str(), session );
                    fin_->finalize( *session );
                    delete cache_->release( key );

                }

            } // lock expireMonitor_

            if ( newset.size() > 0 ) smsc_log_debug( log_, "%u sessions has not expired yet", newset.size() );
            if ( expireSet_.get() ) {
                for ( std::vector< ExpireData >::const_iterator i = newset.begin();
                      i != newset.end();
                      ++i ) {
                    expireSet_->insert(*i);
                }
            }

        } // while expireSet_ is alive

        smsc_log_info(log_,"expire thread is finishing" );
        return 0;

    } // expireThread
     */



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

} // namespace




namespace scag {
namespace sessions2 {

    SessionStore* SessionStore::create( SessionFinalizer& fin,
                                        SessionExpirationQueue& exq )
    {
        return new ::SessionStoreImpl( fin, exq, new Allocator );
    }

} // namespace sessions
} // namespace scag
