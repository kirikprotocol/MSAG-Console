static char const ident[] = "$Id$";
#include <assert.h>

#include "session.hpp"
#include "dialog.hpp"
#include "factory.hpp"
#include "inman/common/util.hpp"
#include "inman/comp/comfactory.hpp"

using std::map;
using std::pair;
using smsc::inman::inap::fillAddress;


namespace smsc  {
namespace inman {
namespace inap  {

/////////////////////////////////////////////////////////////////////////////////////
// IN session
/////////////////////////////////////////////////////////////////////////////////////

Session::Session(UCHAR_T ssn)
    : Thread()
    , SSN( ssn )
    , state( IDLE )
    , lastDialogId( TCAP_DIALOG_MIN_ID )
{
}

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

Session::Session(UCHAR_T ssn, const char* scfNum, const char* inmanNum)
    : Thread()
    , SSN( ssn )
    , state( IDLE )
    , lastDialogId( TCAP_DIALOG_MIN_ID )
{
  fillAddress(&scfAddr,scfNum, ssn);
  fillAddress(&inmanAddr,inmanNum,ssn);
  fillAppContext(&ac);
}

Session::~Session()
{
    closeAllDialogs();
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

int Session::Execute()
{
  running = TRUE;
  started.SignalAll();
  smsc_log_debug(inapLogger,"Session (%d) is started", SSN );

  USHORT_T  result = EINSS7_I97TBindReq( SSN, MSG_USER_ID, TCAP_INSTANCE_ID, EINSS7_I97TCAP_WHITE_USER );

  if (result != 0)
  {
    smsc_log_error(inapLogger, "EINSS7_I97TBindReq() failed with code %d(%s)", result,getTcapReasonDescription(result));
    goto stop;
  }

  MSG_T msg;
  while(running)
  {
      if( ERROR == state )
      {
          smsc_log_debug(inapLogger, "Exit because ERROR state");
          running = FALSE;
          break;
      }

      msg.receiver = MSG_USER_ID;

      result = EINSS7CpMsgRecv_r( &msg,  MSG_RECV_TIMEOUT );

      if( result == MSG_TIMEOUT )
      {
          continue;
      }
      if (result != RETURN_OK )
      {
          smsc_log_error(inapLogger, "MsgRecv failed with code %d(%s)",  result,getReturnCodeDescription(result));
          running = FALSE;
          break;
      }
      else
      {
          //smsc_log_debug(inapLogger, "MSG: sender 0x%X, receiver 0x%X, Primitive 0x%X",  msg.sender, msg.receiver, msg.primitive );
          EINSS7_I97THandleInd(&msg);
      }

      EINSS7CpReleaseMsgBuffer(&msg);
  }

  result = EINSS7_I97TUnBindReq( SSN, MSG_USER_ID, TCAP_INSTANCE_ID );
  if (result != 0)
  {
      smsc_log_error(inapLogger, "EINSS7_I97TUnBindReq() failed with code %d(%s)", result,getTcapReasonDescription(result));
      goto stop;
  }

stop:
  smsc_log_debug(inapLogger,"Session (%d) is stopped", SSN);
  stopped.SignalAll();
  return result;
}


} // namespace inap
} // namespace inmgr
} // namespace smsc
