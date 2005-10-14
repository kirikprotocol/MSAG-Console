#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_FACTORY__
#define __SMSC_INMAN_INAP_FACTORY__

#include <map>

#include "ss7cp.h"
#include "i97tcapapi.h"

#include "dispatcher.hpp"
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

namespace smsc {
namespace inman {
namespace inap {

static const USHORT_T MSG_USER_ID = USER03_ID;
static const UCHAR_T TCAP_INSTANCE_ID = 0;

class Session;

class Factory
{
public:
    typedef enum { IDLE, INITED, OPENED, CONNECTED } State_T;

public:
    Factory();
    virtual   ~Factory();

    void       connect();
    void       disconnect();

    virtual    Session* openSession(UCHAR_T ssn, const char* szSCFNumber, const char* szINmanNumber);
    virtual    Session* openSession(UCHAR_T    ownssn, const char*    ownaddr,
                                    UCHAR_T remotessn, const char* remoteaddr);
    virtual    Session* openSession(UCHAR_T   userssn,
                                    UCHAR_T    ownssn, const char*    ownaddr,
                                    UCHAR_T remotessn, const char* remoteaddr);

    virtual    Session* findSession(UCHAR_T ssn);
    virtual    void     closeSession(Session*);
    virtual    void     closeAllSessions();

    static     Factory* getInstance();
protected:

    typedef std::map<UCHAR_T, Session*> SessionsMap_T;


    SessionsMap_T     sessions;
    State_T           state;
    Logger*       logger;

};

}
}
}

#endif
