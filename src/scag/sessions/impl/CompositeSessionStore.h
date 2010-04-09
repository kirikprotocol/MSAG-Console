#ifndef _SCAG_SESSIONS_IMPL_COMPOSITESESSIONSTORE_H
#define _SCAG_SESSIONS_IMPL_COMPOSITESESSIONSTORE_H

#include <vector>
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "SessionStore2.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag2 {
namespace sessions {

using namespace exceptions;

class CompositeSessionStore
{
private:
    typedef SessionStoreImpl Storage;

public:
    CompositeSessionStore( smsc::logger::Logger*   thelog,
                           SessionFinalizer&       fin,
                           SessionExpirationQueue& exq,
                           SessionAllocator*       a );

    ~CompositeSessionStore();

    void init( unsigned nodeNumber,
               SCAGCommandQueue& queue,
               const std::vector<std::string>& paths,
               unsigned indexgrowth = 10000,
               unsigned pagesize = 512,
               unsigned prealloc = 0,
               unsigned initialCount = 10,
               unsigned initialTime = 100,
               bool     dodiskio = true );

    /// start stop the initial upload thread
    void start();
    void stop();
    
    ActiveSession fetchSession( const SessionKey&           key,
                                std::auto_ptr<SCAGCommand>& cmd,
                                bool                        create );

    // unsigned storedCommands() const;

    bool expireSessions( std::vector< std::pair<SessionKey,time_t> >& expired,
                         const std::vector< std::pair<SessionKey,time_t> >& flush );

    void sessionFinalized( Session& s );

    void getSessionsCount( unsigned& sessionsCount,
                           unsigned& sessionsLoadedCount,
                           unsigned& sessionsLockedCount ) const;

private:
    void clear();

    Storage* store( const SessionKey& key ) const /* throw (SCAGException) */;

    struct InitialThread : public smsc::core::threads::Thread
    {
        InitialThread( CompositeSessionStore& s ) : store_(s) {}

        virtual int Execute();
        
    private:
        CompositeSessionStore& store_;
    };

private:
    bool                    stopped_;
    smsc::logger::Logger*   log_;
    SessionFinalizer*       fin_;
    SessionExpirationQueue* expiration_;
    SessionAllocator*       allocator_;

    // SCAGCommandQueue*       queue_;
    // unsigned                nodeNumber_;
    std::vector< Storage* > storages_;   // owned

    InitialThread*          initialThread_;
    unsigned                initialChunk_;
    unsigned                initialTime_;

    smsc::core::synchronization::EventMonitor stopLock_;

    counter::CounterPtrAny  totalSessions_;
    counter::CounterPtrAny  loadedSessions_;
    counter::CounterPtrAny  lockedSessions_;
    counter::CounterPtrAny  storedCommands_;
};

} // namespace sessions
} // namespace scag2

#endif /* ! _SCAG_SESSIONS_IMPL_COMPOSITESESSIONSTORE_H */
