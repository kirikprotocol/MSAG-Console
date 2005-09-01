static char const ident[] = "$Id$";
#include <assert.h>
#include <string>
#include <stdexcept>

#include "session.hpp"
#include "factory.hpp"
#include "inman/common/util.hpp"

using std::map;
using std::runtime_error;
using std::pair;
using smsc::inman::common::format;
using smsc::inman::common::getReturnCodeDescription;

namespace smsc  {
namespace inman {
namespace inap  {

/////////////////////////////////////////////////////////////////////////////////////
// Static variables
/////////////////////////////////////////////////////////////////////////////////////

Mutex Factory::instanceLock;

/////////////////////////////////////////////////////////////////////////////////////
// IN sessions factory
/////////////////////////////////////////////////////////////////////////////////////

Factory::Factory() : state( IDLE )
{
}

Factory::~Factory()
{
    closeAllSessions();
    closeConnection();
}

void Factory::openConnection()
{
    USHORT_T result;
    while( state != CONNECTED )
    {
      switch( state )
      {
        case IDLE:
            result = MsgInit(MSG_INIT_MAX_ENTRIES);
            if (result != 0)
            {
                throw runtime_error( format("MsgInit Failed with code %d (%s)", result, getReturnCodeDescription(result)));
            }
            state = INITED;
            smsc_log_debug(inapLogger,"MsgInit done.");
            break;

        case INITED:
            result = MsgOpen(MSG_USER_ID);
            if (result != 0)
            {
				throw runtime_error( format("MsgOpen failed with code %d (%s)", result, getReturnCodeDescription(result)));
            }
            state = OPENED;
            smsc_log_debug(inapLogger,"MsgOpen done.");
            break;

       case OPENED:
            result = MsgConn(MSG_USER_ID, TCAP_ID);
            if (result != 0)
            {
				throw runtime_error( format("MsgConn failed with code %d (%s)", result, getReturnCodeDescription(result)) );
            }
            state = CONNECTED;
            smsc_log_debug(inapLogger,"MsgConn done.");
            break;
      }
   }
}

void Factory::closeConnection()
{
    USHORT_T result;

    while( state != IDLE )
    {
      switch( state )
      {
        case CONNECTED:
        {
            result = MsgRel(MSG_USER_ID,TCAP_ID);
            if( result != 0 )
            {
                smsc_log_error(inapLogger, "MsgRel(%d,%d) failed with code %d (%s)", MSG_USER_ID, TCAP_ID, result, getReturnCodeDescription(result));
            }
            else
            {
                smsc_log_debug(inapLogger,"MsgRel done.");
            }
            state = OPENED;
            break;
        }
        case OPENED:
        {
            result = MsgClose(MSG_USER_ID);
            if( result != 0 )
            {
                smsc_log_error(inapLogger, "MsgClose(%d) failed with code %d (%s)", MSG_USER_ID, result ,getReturnCodeDescription(result));
            }
            else
            {
                smsc_log_debug(inapLogger,"MsgClose done.");
            }
            state = INITED;
            break;
        }
        case INITED:
        {
            MsgExit();
            smsc_log_debug(inapLogger,"MsgExit done.");
            state = IDLE;
            break;
        }
     }
   }
}

Factory* Factory::getInstance()
{
    MutexGuard guard( instanceLock );
    static Factory instance;
    return &instance;
}

Session* Factory::openSession(UCHAR_T SSN, const char* szSCFNumber, const char* szINmanNumber,
    											 const char* szSMSCHost, int nSMSCPort)
{
    MutexGuard guard( sessionsLock );

    if( state != CONNECTED ) openConnection();

    if( state != CONNECTED ) return NULL;

    smsc_log_debug(inapLogger,"Open IN session (SSN=%d, SCF=%s, IN=%s, SMSC=%s:%d)", 
    		SSN, szSCFNumber, szINmanNumber, szSMSCHost, nSMSCPort );

	Session* pSession;

    if( sessions.find( SSN ) != sessions.end() )
    {
        throw runtime_error( format("IN session with SSN=%d already opened", SSN) );
    }

	pSession = new Session( SSN, szSCFNumber, szINmanNumber, szSMSCHost, nSMSCPort );

    pSession->Start();

    if( pSession->started.Wait( 1000 ) )
    {
        delete pSession;
        throw runtime_error( format( "Can't start session (SSN=%d)", SSN ) );
    }

    sessions.insert( SessionsMap_T::value_type( SSN, pSession ) );
    return pSession;
}

Session* Factory::findSession(UCHAR_T SSN)
{
    MutexGuard guard( sessionsLock );

    SessionsMap_T::const_iterator it = sessions.find( SSN );
    if(  it == sessions.end() )
    {
        return NULL;
    }
    return (*it).second;
}

void Factory::closeSession(Session* pSession)
{
    MutexGuard guard( sessionsLock );

    if( !pSession ) return;
    smsc_log_debug(inapLogger,"Close IN session (SSN=%d)", pSession->SSN );

    if( sessions.find( pSession->SSN ) == sessions.end() )
    {
        smsc_log_error(inapLogger,"IN session with SSN=%d not exists", pSession->SSN );
    }

    pSession->running = FALSE;

    if( pSession->stopped.Wait( MSG_RECV_TIMEOUT * 3 ) )
    {
        smsc_log_error(inapLogger,"Can't stop IN session (SSN=%d)", pSession->SSN );
    }
    sessions.erase( pSession->SSN );
    delete pSession;
}

void Factory::closeAllSessions()
{
    smsc_log_debug(inapLogger,"Close all IN sessions");

    SessionsMap_T snapshot;

    /* synchronized */
    {
        MutexGuard guard( sessionsLock );
        snapshot = sessions;
    }

    for( SessionsMap_T::iterator it = snapshot.begin(); it != snapshot.end(); it++ )
    {
        closeSession( (*it).second );
    }
}

}
}
}
