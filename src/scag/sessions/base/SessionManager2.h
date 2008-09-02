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

    // should be invoked once
    // static void setInstance( SessionManager* mgr );

    template < class T >
        inline ActiveSession getSession( const SessionKey& key,
                                         std::auto_ptr< T >& cmd,
                                         bool create = true )
        {
            std::auto_ptr<SCAGCommand> kmd(cmd.release());
            ActiveSession a(fetchSession(key, kmd, create));
            cmd.reset( static_cast< T* >(kmd.release()) );
            return a;
        }

    /// get some statistics
    virtual void getSessionsCount( uint32_t& sessionsCount,
                                   uint32_t& sessionsLockedCount ) = 0;

    virtual ~SessionManager();

protected:

    SessionManager();

    /// get session via ActiveSession.
    /// It may have several outcomes:
    /// 1. for any arguments, if retval.get() != 0 then session is fetched.
    ///    cmd.get() should be not zero on return;
    /// 2. retval.get() == 0, cmd.get() == 0 -- session is locked by another command.
    ///    The cmd is taken into an internal session queue;
    /// 3. retval.get() == 0, cmd.get() != 0  (this case may be when create=false).
    ///    The session is not found.
    virtual ActiveSession fetchSession( const SessionKey&           key,
                                        std::auto_ptr<SCAGCommand>& cmd,
                                        bool                        create = true ) = 0;

private:
    SessionManager( const SessionManager& );
    SessionManager& operator = ( const SessionManager& );

};

} // namespace sessions
} // namespace scag2

#endif /* !_SCAG_SESSIONS_BASE_SESSIONPROVIDER_H */
