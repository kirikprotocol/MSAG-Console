static char const ident[] = "$Id$";

#include "ss7tmc.h"

#include <errno.h>
#include <string.h> //for strerrno()
#include <assert.h>
#include <stdexcept>
#include <algorithm>

#include "session.hpp"
#include "dialog.hpp"
#include "factory.hpp"
#include "inman/common/util.hpp"
#include "inman/comp/comfactory.hpp"

using std::max;
using std::map;
using std::pair;
using std::runtime_error;

using smsc::inman::common::getReturnCodeDescription;
using smsc::inman::common::getTcapReasonDescription;
using smsc::inman::common::fillAddress;
using smsc::inman::common::format;
using smsc::inman::common::dump;

namespace smsc  {
namespace inman {
namespace inap  {

/////////////////////////////////////////////////////////////////////////////////////
// IN session
/////////////////////////////////////////////////////////////////////////////////////

static const int SOCKET_TIMEOUT = 1000;

static void fillAppContext(APP_CONTEXT_T* p_ac)
{
  APP_CONTEXT_T& ac = *p_ac;
  ac.acLen=9;
  ac.ac[0] = 0x06; //|00000110 |Tag                    |(UNIV P Obj Identifier)
  ac.ac[0] = 0x07; //|00000111 |Length                 |7
  ac.ac[0] = 0x04; //|00000100 |Authority,Organization |ITU-T, Identified-organization
  ac.ac[0] = 0x00; //|00000000 |                       |ETSI
  ac.ac[0] = 0x00; //|00000000 |Domain                 |Mobile Domain
  ac.ac[0] = 0x01; //|00000001 |Mobile Subdomain       |GSM / UMTS Network
  ac.ac[0] = 0x15; //|00010101 |Common Component ID    |CAP 3 OE
  ac.ac[0] = 0x03; //|00000011 |CAP3 OE ID             |ACE
  ac.ac[0] = 0x3D; //|00111101 |Application Context    |CAP3-SMS
}

Session::Session(UCHAR_T ssn, const char* scfNum, const char* inmanNum, const char* host, int port)
    : Thread()
    , SSN( ssn )
    , state( IDLE )
    , lastDialogId( TCAP_DIALOG_MIN_ID )
{
  	fillAddress(&scfAddr,scfNum, ssn);
  	fillAddress(&inmanAddr,inmanNum, ssn);
  	fillAppContext(&ac);

  	if( socket.Init( host, port, SOCKET_TIMEOUT ) != 0 )
  		throw runtime_error( format("Can't init socket. Host: %s:%d (error: %d, '%s')", 
  								host, port, errno, strerror( errno) ) );
}

Session::~Session()
{
    closeAllDialogs();
    socket.Close();
}

UCHAR_T Session::getSSN() const
{
    return SSN;
}

Session::State_T Session::getState() const
{
    return state;
}

void Session::setState(State_T newState)
{
    MutexGuard guard( lock );
    state = newState;
}

USHORT_T Session::nextDialogId()
{
	USHORT_T id = lastDialogId;
    if( ++lastDialogId  > TCAP_DIALOG_MAX_ID )  
    {
    	lastDialogId = TCAP_DIALOG_MIN_ID;
    }
    return id;
}


TcapDialog* Session::openDialog(USHORT_T id)
{
    MutexGuard guard( lock );
    if(id == 0) id = nextDialogId();
    smsc_log_debug(inapLogger,"Open dialog (SSN=%d, TcapDialog id=%d)", SSN, id );
    TcapDialog* pDlg = new TcapDialog( this, id );
    dialogs.insert( DialogsMap_T::value_type( id, pDlg ) );
    notify1<TcapDialog*>( &SessionListener::onDialogBegin, pDlg );
    return pDlg;
}

TcapDialog* Session::findDialog(USHORT_T id)
{
    MutexGuard guard( lock );
    DialogsMap_T::const_iterator it = dialogs.find( id );
    if( it == dialogs.end() )
    {
        return NULL;
    }
    return (*it).second;

}

void Session::closeDialog(TcapDialog* pDlg)
{
    MutexGuard guard( lock );
    if( !pDlg ) return;
    smsc_log_debug(inapLogger,"Close dialog (SSN=%d, TcapDialog id=%d)", SSN, pDlg->did );
    dialogs.erase( pDlg->did );
    notify1<TcapDialog*>( &SessionListener::onDialogEnd, pDlg );
    delete pDlg;
}

void Session::closeAllDialogs()
{
    smsc_log_debug(inapLogger,"Close all dialogs");
    DialogsMap_T snapshot;

    /* synchronized */
    {
        MutexGuard guard( lock );
        snapshot = dialogs;
    }

    for( DialogsMap_T::iterator it = snapshot.begin(); it != snapshot.end(); it++ )
    {
        closeDialog( (*it).second );
    }
}

void Session::run()
{
  smsc_log_debug(inapLogger,"Session (%d) is started", SSN );

  if( socket.Connect() != 0 )
  		throw runtime_error( format("Can't connect to SMSC (error: %d, '%s')", errno, strerror( errno ) ) );


  USHORT_T  result = EINSS7_I97TBindReq( SSN, MSG_USER_ID, TCAP_INSTANCE_ID, EINSS7_I97TCAP_WHITE_USER );

  if (result != 0 )
	throw runtime_error( format( "BindReq() failed with code %d (%s)", result,getTcapReasonDescription(result)));

  MSG_T msg;

  fd_set activeDescriptors;
  FD_ZERO(&activeDescriptors);

  SOCKET ss7socket  = EINSS7CpMsgObtainSocket(MSG_USER_ID, TCAP_ID);
  SOCKET smscsocket = socket.getSocket();

  FD_SET(ss7socket,  &activeDescriptors); /* Add ss7socket to the empty set*/
  FD_SET(smscsocket, &activeDescriptors); /* Add SMSC socket */

  struct timeval tv;
  tv.tv_sec 	= 0;
  tv.tv_usec 	= SOCKET_TIMEOUT * 1000;

  running = TRUE;

  while(running)
  {
      if( ERROR == state )
      {
          smsc_log_debug(inapLogger, "Exit because ERROR state");
          running = FALSE;
          break;
      }

		/* Use select to wait for input on any of the sockets */
		if( select( max( ss7socket, smscsocket) +1, &activeDescriptors, 0, 0, &tv ) > 0 )
		{
			if(FD_ISSET(ss7socket, &activeDescriptors)) /* Take care of SS7 message */
			{
				msg.receiver = MSG_USER_ID;

				result = EINSS7CpMsgRecv_r(&msg, SOCKET_TIMEOUT);

    			if (result != 0 )
    			{
	    			EINSS7CpReleaseMsgBuffer(&msg);
          			throw runtime_error( format( "MsgRecv failed with code %d (%s)", result, getReturnCodeDescription(result)) );
          		}

          		EINSS7_I97THandleInd(&msg);
      			EINSS7CpReleaseMsgBuffer(&msg);
			}
			else
			if(FD_ISSET(socket.getSocket(), &activeDescriptors)) /* read from SMSC socket */
			{
			unsigned char buffer[1024];
			int n = socket.Read( (char*)buffer, sizeof( buffer ) );
			if( n < 0 )
				throw runtime_error( format("Socket::Read failed with code %d (%s)", errno, strerror( errno )));

			smsc_log_debug(inapLogger, "Socket::Read( %s )", dump( n, buffer, true ).c_str() );
		}
		else 
        {
        	smsc_log_error(inapLogger, "Unknown select event");
        }
  	}
  	else
  	{
  		smsc_log_debug(inapLogger, "Select timeout...");
  	}
  }

  result = EINSS7_I97TUnBindReq( SSN, MSG_USER_ID, TCAP_INSTANCE_ID );

  if (result != 0)
      throw runtime_error( format("UnBindReq() failed with code %d (%s)", result, getTcapReasonDescription(result)));
}

int Session::Execute()
{
	int result = 0;
  	started.SignalAll();
	try
	{
      	smsc_log_debug(tcapLogger, "Session thread started");
		run();
	}
	catch(const std::exception& error)
	{
      	smsc_log_error(tcapLogger, "Error in session thread: %s", error.what() );
      	result = 1;
	}
	smsc_log_debug(tcapLogger, "Session thread finished");
  	stopped.SignalAll();
  	return result;
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
