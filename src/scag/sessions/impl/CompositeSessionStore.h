#ifndef _SCAG_SESSIONS_IMPL_COMPOSITESESSIONSTORE_H
#define _SCAG_SESSIONS_IMPL_COMPOSITESESSIONSTORE_H

#include <vector>
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
               bool     dodiskio = true );

    void stop();
    
    ActiveSession fetchSession( const SessionKey&           key,
                                std::auto_ptr<SCAGCommand>& cmd,
                                bool                        create );

    unsigned storedCommands() const;

    bool expireSessions( const std::vector< SessionKey >& expired,
                         const std::vector< std::pair<SessionKey,time_t> >& flush );

    void sessionFinalized( Session& s );

    void getSessionsCount( unsigned& sessionsCount,
                           unsigned& sessionsLockedCount ) const;

private:
    void clear();

    Storage* store( const SessionKey& key ) const throw (SCAGException);
    
private:
    bool                    stopped_;
    smsc::logger::Logger*   log_;
    SessionFinalizer*       fin_;
    SessionExpirationQueue* expiration_;
    SessionAllocator*       allocator_;

    // SCAGCommandQueue*       queue_;
    // unsigned                nodeNumber_;
    std::vector< Storage* > storages_;   // owned
};

} // namespace sessions
} // namespace scag2

#endif /* ! _SCAG_SESSIONS_IMPL_COMPOSITESESSIONSTORE_H */
