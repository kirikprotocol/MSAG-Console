#include "SessionStore.h"

namespace scag { namespace sessions 
{

    void SessionStore::init(const std::string& dir,SessionLoadCallback cb,void* data)
    {
    
    }
    
    SessionPtr SessionStore::getSession(const CSessionKey& sessionKey)
    {
        SessionPtr session;
        session = new Session(sessionKey);
        return session;
    }

    SessionPtr SessionStore::newSession(const CSessionKey& sessionKey)
    {
        SessionPtr session;
        session = new Session(sessionKey);
        return session;
    }
    
    void SessionStore::deleteSession(const CSessionKey& sessionKey)
    {
    }
    
    void SessionStore::updateSession(SessionPtr session)
    {
    }
    
   
}}

