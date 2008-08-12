#ifndef _SCAG_SESSIONS_IMPL_SESSIONSTORE2_H
#define _SCAG_SESSIONS_IMPL_SESSIONSTORE2_H

#include "time.h"
#include <vector>

#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "scag/sessions/base/Session2.h"
#include "scag/sessions/base/SessionStore2.h"
#include "scag/util/storage/HashedMemoryCache.h"
#include "scag/util/storage/PageFileDiskStorage.h"
#include "scag/util/storage/RBTreeIndexStorage.h"
#include "scag/util/storage/StorageIface.h"
#include "scag/util/storage/CompositeDiskStorage.h"

namespace scag2 {
namespace sessions {

class SessionFinalizer;
class SessionExpirationQueue;


// create an instance of session store for given processing node
// \param fin -- session finalizer, method finalize() is invoked when session is expired;
// \param exq -- an instance of session expiration queue interface,
// \param queue -- an instance of scag command queue interface, the instance
//                 is used in method releaseSession to push the next command to the queue.
// static SessionStore* createSessionStore( SessionFinalizer&       fin,
//                                         SessionExpirationQueue& exq );

struct SessionAllocator {
    virtual ~SessionAllocator() {}
    virtual Session* alloc( const SessionKey& k ) { return new Session(k); }
};


class SessionStoreImpl : public SessionStore
{
public:
    typedef HashedMemoryCache< SessionKey, Session > MemStorage;
    typedef PageFileDiskStorage< SessionKey, Session > DiskDataStorage;
    typedef RBTreeIndexStorage< SessionKey, DiskDataStorage::index_type > DiskIndexStorage;
    typedef IndexedStorage< DiskIndexStorage, DiskDataStorage, smsc::core::synchronization::Mutex > EltDiskStorage;
    // typedef IndexedStorage< DiskIndexStorage, DiskDataStorage, SlowMutex > EltDiskStorage;
    typedef CompositeDiskStorage< EltDiskStorage > DiskStorage;

    /// create a storage
    SessionStoreImpl( SessionFinalizer& fin,
                      SessionExpirationQueue& exq,
                      SessionAllocator* a );

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

    virtual bool expireSessions( const std::vector< SessionKey >& expired );

    virtual void sessionFinalized( Session& s );

    virtual void getSessionsCount( unsigned& sessionsCount,
                                   unsigned& sessionsLockedCount ) const;

protected:

    // helper method, may be invoked from expireSessions or from sessionFinalized
    // NOTE: lock should be pre-locked.
    bool doSessionFinalization( Session& s );

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

    std::auto_ptr<DiskStorage>      disk_;
    std::auto_ptr<SessionAllocator> allocator_;

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

} // namespace sessions2
} // namespace scag

#endif /* !_SCAG_SESSIONS_SESSIONSTORE2_H */
