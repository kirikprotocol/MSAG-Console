#ifndef SCAG_SESSIONS_MANAGER
#define SCAG_SESSIONS_MANAGER

#include <string>

#include <core/threads/Thread.hpp>
#include <scag/transport/SCAGCommand.h>

#include "Session.h"

namespace scag { namespace sessions 
{
    using namespace smsc::core::threads;
    using scag::transport::SCAGCommand;
    
    class SessionManager
    {
    public:

        static void Init(const std::string& dir);
        static SessionManager& Instance();

        virtual Session* getSession(const SCAGCommand& command)   = 0;
        virtual void releaseSession(const Session* session) = 0;
        virtual void closeSession  (const Session* session) = 0;

    protected:
    
        SessionManager() {};
        virtual ~SessionManager() = 0;

    private:
        
        SessionManager(const SessionManager& sm);
        SessionManager& operator=(const SessionManager& sm);
    };
}}

#endif // SCAG_SESSIONS_MANAGER
