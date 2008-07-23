#ifndef _SCAG_SESSIONS_SESSIONSTORE2_H
#define _SCAG_SESSIONS_SESSIONSTORE2_H

#include "Session2.h"
#include "SessionFinalizer.h"
#include "scag/config/ConfigView.h"

namespace scag {
namespace sessions2 {

    using namespace scag::transport2;

class SessionStore
{
public:
    virtual ~SessionStore() {}

    /// create an instance of session store for given processing node
    /// \param fin -- session finalizer, method finalize() is invoked when session is expired;
    /// \param queue -- an instance of scag command queue interface, the instance
    ///                 is used in method releaseSession to push the next command to the queue.
    static SessionStore* create( unsigned          node,
                                 SessionFinalizer& fin,
                                 SCAGCommandQueue& queue );

    /// initialization
    virtual void init( const scag::config::ConfigView& cfg ) = 0;

    /// stop the storage
    virtual void stop( bool wait = false ) = 0;

    /// fetch/create session for given command.
    /// NOTE: command ownership is taken.
    /// @return NULL if session is locked and push command to the end of session command list.
    /// otherwise, return valid session with current command set.
    virtual ActiveSession fetchSession( const SessionKey& key, SCAGCommand* cmd ) = 0;

    /// release the session.
    /// NOTE: this method is invoked automatically from ActiveSession dtor.
    virtual void releaseSession( Session& s, bool flush ) = 0;


    /// Total number of commands that are attached to sessions
    virtual unsigned storedCommands() const = 0;

protected:
    inline Session* getCommandSession( SCAGCommand& cmd ) const {
        return cmd.getSession();
    }
    inline void setCommandSession( SCAGCommand& cmd, Session* s ) const
    {
        cmd.setSession( s );
    }

};

} // namespace sessions2
} // namespace scag

#endif /* !_SCAG_SESSIONS_SESSIONSTORE2_H */
