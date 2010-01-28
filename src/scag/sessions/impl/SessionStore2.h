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
#include "scag/counter/Counter.h"

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
    Session* quietalloc( const SessionKey& k ) { return new Session(k,true); }
};


class SessionStoreImpl : public SessionStore
{
public:
    typedef util::storage::HashedMemoryCache< SessionKey, Session > MemStorage;
    typedef util::storage::PageFileDiskStorage< SessionKey, Session > DiskDataStorage;
    typedef util::storage::RBTreeIndexStorage< SessionKey, DiskDataStorage::index_type, StoredSessionKey > DiskIndexStorage;
    typedef util::storage::IndexedStorage< DiskIndexStorage, DiskDataStorage, smsc::core::synchronization::Mutex > EltDiskStorage;
    // typedef IndexedStorage< DiskIndexStorage, DiskDataStorage, SlowMutex > EltDiskStorage;
    // typedef CompositeDiskStorage< EltDiskStorage > DiskStorage;
    typedef EltDiskStorage DiskStorage;

    /// create a storage
    SessionStoreImpl( SessionFinalizer& fin,
                      SessionExpirationQueue& exq,
                      SessionAllocator* a );

    virtual ~SessionStoreImpl();

    /// unlock storage, clear stop flag, etc.
    void init( unsigned eltNumber,
               SCAGCommandQueue& queue,
               const std::string& path = "sessions",
               unsigned indexgrowth = 10000,
               unsigned pagesize = 512,
               unsigned prealloc = 0,
               bool     dodiskio = true );

    void stop();

    ActiveSession fetchSession( const SessionKey&           key,
                                std::auto_ptr<SCAGCommand>& cmd,
                                bool                        create );

    // unsigned storedCommands() const;

    bool expireSessions( const std::vector< SessionKey >& expired,
                         const std::vector< std::pair<SessionKey,time_t> >& flush );

    void sessionFinalized( Session& s );

    void getSessionsCount( unsigned& sessionsCount,
                           unsigned& sessionsLoadedCount,
                           unsigned& sessionsLockedCount ) const;

    /// upload count initial sessions, return true if there are more sessions to upload
    bool uploadInitial( unsigned count );
    
    /// --- session store iface

    virtual void releaseSession( Session& s );

    virtual void moveLock( Session& s, SCAGCommand* cmd );

protected:

    // helper method, may be invoked from expireSessions or from sessionFinalized.
    // keep = true, if the session should not be cleaned.
    // NOTE: cachelock should be pre-acquired.
    bool doSessionFinalization( Session& s, bool keep );

    ActiveSession makeLockedSession( Session& s, SCAGCommand& c );

    bool carryNextCommand( Session& s, SCAGCommand* cmd, bool dolock );

private:
    // unsigned                    eltNumber_;  // the number of elementary storage

    SessionFinalizer*           fin_;
    SessionExpirationQueue*     expiration_;
    SCAGCommandQueue*           queue_;

    mutable EventMonitor        cacheLock_;
    bool                        stopping_;

    SessionAllocator*           allocator_;    // not owned
    std::auto_ptr<MemStorage>   cache_;
    std::auto_ptr<DiskStorage>  disk_;
    std::vector< DiskIndexStorage::storedkey_type > initialkeys_;   // the list of initial keys

    smsc::logger::Logger*       log_;
    
    bool                        diskio_;

    // statistics
    // unsigned                    totalSessions_;
    // unsigned                    loadedSessions_;
    // unsigned                    lockedSessions_;
    counter::CounterPtrAny      totalSessions_;
    counter::CounterPtrAny      loadedSessions_;
    counter::CounterPtrAny      lockedSessions_;
    unsigned                    storedCommands_;

    unsigned                    maxqueuesize_;
    unsigned                    maxcachesize_;
    unsigned                    maxcommands_;
};

} // namespace sessions2
} // namespace scag

#endif /* !_SCAG_SESSIONS_SESSIONSTORE2_H */
