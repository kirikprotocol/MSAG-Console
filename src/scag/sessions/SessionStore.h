#ifndef SCAG_SESSIONS_STORE
#define SCAG_SESSIONS_STORE

#include <inttypes.h>
#include <string>

#include "Session.h"

namespace scag { namespace sessions 
{
    class SessionStore
    {
    public:

        SessionStore() {};
        ~SessionStore() {};

        void init(const std::string& dir);

        SessionGuard getSession(const CSessionKey& sessionKey);
        void deleteSesion(const CSessionKey& sessionKey);
        void updateSesion(Session* session);
    };
}}

#endif // SCAG_SESSIONS_STORE
