#ifndef SCAG_SESSIONS_MANAGER
#define SCAG_SESSIONS_MANAGER

#include <string>
#include <core/threads/Thread.hpp>
#include "Session.h"
#include "scag/sessions/SessionStore.h"

namespace scag { namespace sessions 
{
    using namespace smsc::core::threads;
    using scag::transport::SCAGCommand;
    using scag::sessions::SessionPtr;


    class SessionManager 
    {
        SessionManager(const SessionManager& sm);
        SessionManager& operator=(const SessionManager& sm);
    protected:
        virtual ~SessionManager() {};
        SessionManager() {};
    public:
        static const time_t DEFAULT_EXPIRE_INTERVAL;
        static void Init(const SessionManagerConfig& config);
        static SessionManager& Instance();


        virtual SessionPtr newSession(CSessionKey& key) = 0;
        virtual bool getSession(const CSessionKey& key, SessionPtr& session, SCAGCommand& cmd) = 0;
        virtual void releaseSession(SessionPtr session)     = 0;
//        virtual void closeSession  (SessionPtr session)     = 0;
        virtual uint32_t getSessionsCount() = 0;
    };
}}

#endif // SCAG_SESSIONS_MANAGER
