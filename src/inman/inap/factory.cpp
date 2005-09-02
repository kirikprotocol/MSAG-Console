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
// IN sessions factory
/////////////////////////////////////////////////////////////////////////////////////

Factory::Factory() 
	: logger(Logger::getInstance("smsc.inman.inap.Factory"))
	, state( IDLE )
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
            smsc_log_debug(logger,"MsgInit done.");
            break;

        case INITED:
            result = MsgOpen(MSG_USER_ID);
            if (result != 0)
            {
				throw runtime_error( format("MsgOpen failed with code %d (%s)", result, getReturnCodeDescription(result)));
            }
            state = OPENED;
            smsc_log_debug(logger,"MsgOpen done.");
            break;

       case OPENED:
            result = MsgConn(MSG_USER_ID, TCAP_ID);
            if (result != 0)
            {
				throw runtime_error( format("MsgConn failed with code %d (%s)", result, getReturnCodeDescription(result)) );
            }
            state = CONNECTED;
            smsc_log_debug(logger,"MsgConn done.");
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
                smsc_log_error(logger, "MsgRel(%d,%d) failed with code %d (%s)", MSG_USER_ID, TCAP_ID, result, getReturnCodeDescription(result));
            }
            else
            {
                smsc_log_debug(logger,"MsgRel done.");
            }
            state = OPENED;
            break;
        }
        case OPENED:
        {
            result = MsgClose(MSG_USER_ID);
            if( result != 0 )
            {
                smsc_log_error(logger, "MsgClose(%d) failed with code %d (%s)", MSG_USER_ID, result ,getReturnCodeDescription(result));
            }
            else
            {
                smsc_log_debug(logger,"MsgClose done.");
            }
            state = INITED;
            break;
        }
        case INITED:
        {
            MsgExit();
            smsc_log_debug(logger,"MsgExit done.");
            state = IDLE;
            break;
        }
     }
   }
}


SOCKET	Factory::getHandle()
{
	return EINSS7CpMsgObtainSocket(MSG_USER_ID, TCAP_ID);
}

void	Factory::process(Dispatcher*)
{
		MSG_T msg;

		msg.receiver = MSG_USER_ID;

		UCHAR_T result = EINSS7CpMsgRecv_r(&msg, 0);

		if (result != 0 )
		{
	    		EINSS7CpReleaseMsgBuffer(&msg);
          		throw runtime_error( format( "MsgRecv failed with code %d (%s)", result, getReturnCodeDescription(result)) );
        }

        EINSS7_I97THandleInd(&msg);
      	EINSS7CpReleaseMsgBuffer(&msg);
}

Session* Factory::openSession(UCHAR_T SSN, const char* szSCFNumber, const char* szINmanNumber)
{
    if( state != CONNECTED ) openConnection();

    if( state != CONNECTED ) return NULL;

    smsc_log_debug(logger,"Open IN session (SSN=%d, SCF=%s, IN=%s)", SSN, szSCFNumber, szINmanNumber);

	Session* pSession;

    if( sessions.find( SSN ) != sessions.end() )
    {
        throw runtime_error( format("IN session with SSN=%d already opened", SSN) );
    }

	pSession = new Session( SSN, szSCFNumber, szINmanNumber );

    sessions.insert( SessionsMap_T::value_type( SSN, pSession ) );

    return pSession;
}

Session* Factory::findSession(UCHAR_T SSN)
{
    SessionsMap_T::const_iterator it = sessions.find( SSN );
    if(  it == sessions.end() )
    {
        return NULL;
    }
    return (*it).second;
}

void Factory::closeSession(Session* pSession)
{
    if( !pSession ) return;
    smsc_log_debug(logger,"Close IN session (SSN=%d)", pSession->SSN );

    if( sessions.find( pSession->SSN ) == sessions.end() )
    {
        smsc_log_error(logger, "IN session with SSN=%d not exists", pSession->SSN );
    }

    sessions.erase( pSession->SSN );
    delete pSession;
}

void Factory::closeAllSessions()
{
    smsc_log_debug(logger,"Close all IN sessions");

    for( SessionsMap_T::iterator it = sessions.begin(); it != sessions.end(); it++ )
    {
        closeSession( (*it).second );
    }
}

}
}
}
