#ifndef SCAG_SESSIONS_MANAGER2
#define SCAG_SESSIONS_MANAGER2

#include <string>
#include <core/threads/Thread.hpp>
#include "Session2.h"
#include "SessionStore2.h"

namespace scag {

namespace config2 {
    class SessionManagerConfig;
} // namespace config

namespace sessions2 {

using namespace smsc::core::threads;
using scag::transport2::SCAGCommand;
using scag::transport2::SCAGCommandQueue;

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
                          const scag::config2::SessionManagerConfig& config,
                          SCAGCommandQueue& cmdqueue );
        static SessionManager& Instance();

        // virtual SessionPtr newSession(CSessionKey& key) = 0;
        // virtual bool getSession(const CSessionKey& key, SessionPtr& session, SCAGCommand& cmd) = 0;
        // virtual void releaseSession(SessionPtr session)     = 0;
        virtual ActiveSession getSession( const SessionKey& key,
                                          SCAGCommand*      cmd ) = 0;

//        virtual void closeSession  (SessionPtr session)     = 0;
        virtual void getSessionsCount( uint32_t& sessionsCount,
                                       uint32_t& sessionsLockedCount ) = 0;

    protected:
        // place session to expiration queue
        // virtual void expireSession( const SessionKey& key,
        // time_t            expirationTime ) = 0;


        ActiveSession makeActiveSession( Session* session ) const;
    };
}}

#endif // SCAG_SESSIONS_MANAGER
