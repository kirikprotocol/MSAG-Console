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

        Session * getSession(const CSessionKey& sessionKey);

        void newSession(const CSessionKey& sessionKey);
        void deleteSession(const CSessionKey& sessionKey);
        void updateSession(Session* session);
        void loadExpireList(COperationsHash& opList);
    };
}}

#endif // SCAG_SESSIONS_STORE
