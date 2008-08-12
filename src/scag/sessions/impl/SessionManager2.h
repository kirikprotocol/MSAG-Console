#ifndef SCAG_SESSIONS_MANAGER2
#define SCAG_SESSIONS_MANAGER2

#include <string>
#include <core/threads/Thread.hpp>
#include "Session2.h"
#include "SessionStore2.h"

namespace scag2 {

namespace config {
    class SessionManagerConfig;
} // namespace config

namespace sessions {

using namespace smsc::core::threads;
using transport::SCAGCommand;
using transport::SCAGCommandQueue;

    class SessionManager
    {
        SessionManager(const SessionManager& sm);
        SessionManager& operator=(const SessionManager& sm);
    protected:
        virtual ~SessionManager() {};
        SessionManager() {};
    public:
        static const time_t DEFAULT_EXPIRE_INTERVAL;
        static void Init( unsigned nodeNumber,
                          const scag2::config::SessionManagerConfig& config,
                          SCAGCommandQueue& cmdqueue );
        static SessionManager& Instance();

        // virtual SessionPtr newSession(CSessionKey& key) = 0;
        // virtual bool getSession(const CSessionKey& key, SessionPtr& session, SCAGCommand& cmd) = 0;
        // virtual void releaseSession(SessionPtr session)     = 0;
        /// NOTE: T must be a subclass of SCAGCommand
        template < class T > 
            inline ActiveSession getSession( const SessionKey& key,
                                             std::auto_ptr<T>& cmd )
        {
            ActiveSession a(fetchSession(key, cmd.get()));
            if ( ! a.get() ) cmd.release();
            return a;
        }

//        virtual void closeSession  (SessionPtr session)     = 0;
        virtual void getSessionsCount( uint32_t& sessionsCount,
                                       uint32_t& sessionsLockedCount ) = 0;

    protected:

        /// get session via ActiveSession.
        /// if unsuccessful (i.e. retval.get() == 0), then cmd is taken
        /// into session internal command queue.
        virtual ActiveSession fetchSession( const SessionKey& key,
                                            SCAGCommand*      cmd ) = 0;

        // place session to expiration queue
        // virtual void expireSession( const SessionKey& key,
        // time_t            expirationTime ) = 0;


        // ActiveSession makeActiveSession( Session* session ) const;
    };
}}

#endif // SCAG_SESSIONS_MANAGER
