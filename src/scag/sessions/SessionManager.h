#ifndef SCAG_SESSIONS_MANAGER
#define SCAG_SESSIONS_MANAGER

#include <string>
#include <core/threads/Thread.hpp>
#include "Session.h"
#include "scag/config/sessionman/SessionManagerConfig.h"

namespace scag { namespace sessions 
{
    using namespace smsc::core::threads;
    using scag::transport::SCAGCommand;
    using scag::config::SessionManagerConfig;
    

    class SessionManager 
    {
        SessionManager(const SessionManager& sm);
        SessionManager& operator=(const SessionManager& sm);
    protected:
        virtual ~SessionManager() {};
        SessionManager() {};
    public:

        static void Init(const SessionManagerConfig& config);
        static SessionManager& Instance();


        virtual Session* newSession(CSessionKey& key) = 0;
        virtual Session* getSession(const CSessionKey& key) = 0;
        virtual void releaseSession(Session* session)     = 0;
        virtual void closeSession  (const Session* session)     = 0;
        
    };
}}

#endif // SCAG_SESSIONS_MANAGER
