#include "SessionStore.h"

namespace scag { namespace sessions 
{

    SessionPtr currSession;

    void SessionStore::init(const std::string& dir,SessionLoadCallback cb,void* data)
    {
    
    }
    
    SessionPtr SessionStore::getSession(const CSessionKey& sessionKey)
    {
        return currSession;
    }

    SessionPtr SessionStore::newSession(const CSessionKey& sessionKey)
    {
        currSession = new Session(sessionKey);

        return currSession;
    }
    
    void SessionStore::deleteSession(const CSessionKey& sessionKey)
    {
    }
    
    void SessionStore::updateSession(SessionPtr session)
    {
    }
    
   
}}

