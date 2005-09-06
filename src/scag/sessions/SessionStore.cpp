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

    void SessionStore::newSesion(const CSessionKey& sessionKey)
    {
    }
    
    void SessionStore::deleteSesion(const CSessionKey& sessionKey)
    {
    }
    
    void SessionStore::updateSesion(Session* session)
    {
    }
    
    void SessionStore::loadExpireList(COperationsList& opList)
    {
    }
    
}}

