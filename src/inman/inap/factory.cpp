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
using smsc::inman::common::dump;
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
	connect();
}

Factory::~Factory()
{
    closeAllSessions();
    disconnect();
}

void Factory::connect()
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
            smsc_log_debug(logger,"state: INITED");
            break;

        case INITED:
            result = MsgOpen(MSG_USER_ID);
            if (result != 0)
            {
				throw runtime_error( format("MsgOpen failed with code %d (%s)", result, getReturnCodeDescription(result)));
            }
            state = OPENED;
            smsc_log_debug(logger,"state: OPENED");
            break;

       case OPENED:
            result = MsgConn(MSG_USER_ID, TCAP_ID);
            if (result != 0)
            {
				throw runtime_error( format("MsgConn failed with code %d (%s)", result, getReturnCodeDescription(result)) );
            }
            state = CONNECTED;
            smsc_log_debug(logger,"state: CONNECTED");
            break;
      }
   }
}

void Factory::disconnect()
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
            smsc_log_debug(logger,"state: OPENED");
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
            smsc_log_debug(logger,"state: INITED");
            state = INITED;
            break;
        }
        case INITED:
        {
            MsgExit();
            smsc_log_debug(logger,"state: IDLE");
            state = IDLE;
            break;
        }
     }
   }
}

Session* Factory::openSession(UCHAR_T SSN, const char* szSSF, const char* szSCF)
{
    if( state != CONNECTED )
    {
    	throw runtime_error( format("Invalid factory state (%d)", state) );
	}

    smsc_log_debug(logger,"Open session (SSN=%d, SCF=%s, IN=%s)", SSN, szSSF, szSCF);

    if( sessions.find( SSN ) != sessions.end() )
    {
        throw runtime_error( format("Session with SSN=%d already opened", SSN) );
    }

	Session* pSession = new Session( SSN, szSSF, szSCF );

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
    assert( pSession );

    smsc_log_debug(logger,"Close session (SSN=%d)", pSession->SSN );

    if( sessions.find( pSession->SSN ) == sessions.end() )
    {
        smsc_log_error(logger, "Session with SSN=%d not exists", pSession->SSN );
    }

    sessions.erase( pSession->SSN );
    delete pSession;
}

void Factory::closeAllSessions()
{
    smsc_log_debug(logger,"Close all sessions");

    for( SessionsMap_T::iterator it = sessions.begin(); it != sessions.end(); it++ )
    {
        closeSession( (*it).second );
    }
}

Factory* Factory::getInstance()
{
	static Factory instance;
	return &instance;
}

}
}
}
