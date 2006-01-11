#ifndef SCAG_SESSIONS_STORE
#define SCAG_SESSIONS_STORE

#include <inttypes.h>
#include <string>

#include "Session.h"
#include "core/buffers/RefPtr.hpp"
#include "core/synchronization/Mutex.hpp"

namespace scag { namespace sessions
{
    typedef smsc::core::buffers::RefPtr<Session,smsc::core::synchronization::Mutex> SessionPtr;

    typedef void (*SessionLoadCallback)(void*,Session*);

    class SessionStore
    {
    public:

        SessionStore() {};
        ~SessionStore() {};

        void init(const std::string& dir,SessionLoadCallback cb,void* data);

        SessionPtr getSession(const CSessionKey& sessionKey);

        SessionPtr newSession(const CSessionKey& sessionKey);
        void deleteSession(const CSessionKey& sessionKey);
        void updateSession(SessionPtr session);
    };
}}

#endif // SCAG_SESSIONS_STORE
