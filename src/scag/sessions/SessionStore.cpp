#include "SessionStore.h"

namespace scag { namespace sessions 
{

    void SessionStore::init(const std::string& dir)
    {
    
    }
    
    Session * SessionStore::getSession(const CSessionKey& sessionKey)
    {
        return new Session();
    }

    void SessionStore::newSession(const CSessionKey& sessionKey)
    {
    }
    
    void SessionStore::deleteSession(const CSessionKey& sessionKey)
    {
    }
    
    void SessionStore::updateSession(Session* session)
    {
    }
    
    void SessionStore::loadExpireList(COperationsHash& opList)
    {
    }
    
}}

