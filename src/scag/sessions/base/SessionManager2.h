#ifndef _SCAG_SESSIONS_BASE_SESSIONMANAGER_H
#define _SCAG_SESSIONS_BASE_SESSIONMANAGER_H

#include <memory>
#include "Session2.h"

namespace scag2 {
namespace sessions {

using namespace util;

/// this is an interface for providing sessions for different clients.
/// SessionManagerImpl will inherit from this one.
class SessionManager
{
public:
    static SessionManager& Instance();

    /// should be invoked once
    static void setInstance( SessionManager* mgr );

    template < class T >
        inline ActiveSession getSession( const SessionKey& key,
                                         std::auto_ptr< T >& cmd )
        {
            ActiveSession a(fetchSession(key, cmd.get()));
            if ( ! a.get() ) cmd.release();
            return a;
        }

    /// get some statistics
    virtual void getSessionsCount( uint32_t& sessionsCount,
                                   uint32_t& sessionsLockedCount ) = 0;

    virtual ~SessionManager();

protected:

    SessionManager();

    /// get session via ActiveSession.
    /// if unsuccessful (i.e. retval.get() == 0), then cmd is taken
    /// into session internal command queue.
    virtual ActiveSession fetchSession( const SessionKey& key,
                                        SCAGCommand*      cmd ) = 0;

private:
    SessionManager( const SessionManager& );
    SessionManager& operator = ( const SessionManager& );

};

} // namespace sessions
} // namespace scag2

#endif /* !_SCAG_SESSIONS_BASE_SESSIONPROVIDER_H */
