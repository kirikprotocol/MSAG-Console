#ifndef _SCAG_SESSIONS_SESSIONSTORE2_H
#define _SCAG_SESSIONS_SESSIONSTORE2_H

#include "time.h"
#include <vector>

// #include "Session2.h"
#include "scag/transport/SCAGCommand2.h"
#include "SessionKey.h"
// #include "SessionFinalizer.h"
// #include "SessionExpirationQueue.h"
#include "scag/config/ConfigView.h"

namespace scag2 {
namespace sessions {

    using namespace scag2::transport;
    class Session;
    class SessionFinalizer;
    class SessionExpirationQueue;


class SessionStore
{
public:

    struct ExpireData 
    {
        ExpireData( time_t expTime, const SessionKey& k ) : expiration(expTime), key(k) {}
        bool operator < ( const ExpireData& e ) const {
            return ( expiration < e.expiration || 
                     ( expiration == e.expiration && key < e.key ) );
        }
                
        bool operator == ( const ExpireData& e ) const {
            return ( expiration == e.expiration ) && ( key == e.key );
        }

    public:
        time_t      expiration;
        SessionKey  key;
    };

public:
    virtual ~SessionStore() {}

    /// create an instance of session store for given processing node
    /// \param fin -- session finalizer, method finalize() is invoked when session is expired;
    /// \param exq -- an instance of session expiration queue interface,
    /// \param queue -- an instance of scag command queue interface, the instance
    ///                 is used in method releaseSession to push the next command to the queue.
    static SessionStore* create( SessionFinalizer&       fin,
                                 SessionExpirationQueue& exq );

    /// initialization, clear stopping flag
    virtual void init( unsigned           nodeNumber,
                       SCAGCommandQueue&  cmdqueue,
                       const std::string& path = "sessions",
                       const std::string& name = "sessions",
                       unsigned indexgrowth = 10000,
                       unsigned pagesize = 512,
                       unsigned prealloc = 0 ) = 0;

    /// tell storage to begin stopping
    virtual void stop() = 0;

    /// fetch/create session for given command.
    /// NOTE: command ownership is taken.
    /// @return NULL if session is locked and push command to the end of session command list.
    /// otherwise, return valid session with current command set.
    virtual ActiveSession fetchSession( const SessionKey& key, SCAGCommand* cmd ) = 0;

    /// release session (return it to store and unlock for further use).
    /// \param flush (do flush on disk prior to releasing session lock).
    /// NOTE: this method is invoked automatically from ActiveSession dtor.
    virtual void releaseSession( Session& s, bool flush ) = 0;

    /// move lock on the session to the specified command.
    virtual void moveLock( Session& s, SCAGCommand* cmd ) = 0;

    /// Total number of commands that are attached to sessions
    virtual unsigned storedCommands() const = 0;

    /// expire sessions and fill the list of sessions that are not yet expired
    virtual void expireSessions( const std::vector< ExpireData >& expired,
                                 std::vector< ExpireData >& notyetexpired ) = 0;

    /// get current sessions count
    virtual void getSessionsCount( unsigned& sessionsCount,
                                   unsigned& sessionsLockedCount ) const = 0;

    /*
protected:
    inline Session* getCommandSession( SCAGCommand& cmd ) const {
        return cmd.getSession();
    }
     */
    inline void setCommandSession( SCAGCommand& cmd, Session* s ) const
    {
        cmd.setSession( s );
    }

};

} // namespace sessions2
} // namespace scag

#endif /* !_SCAG_SESSIONS_SESSIONSTORE2_H */
