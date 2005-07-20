// $Id$
#include <assert.h>

#include "session.hpp"
#include "dialog.hpp"
#include "factory.hpp"
#include "util.hpp"

using std::map;
using std::pair;

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

Dialog* Session::openDialog()
{
    MutexGuard guard( lock );
    smsc_log_debug(inapLogger,"Open dialog (SSN=%d, Dialog id=%d)", SSN, lastDialogId );
    Dialog* pDlg = new Dialog( this, lastDialogId );
    dialogs.insert( DialogsMap_T::value_type( lastDialogId, pDlg ) );
    if( ++lastDialogId  > TCAP_DIALOG_MAX_ID )  lastDialogId = TCAP_DIALOG_MIN_ID;
    return pDlg;
}

Dialog* Session::findDialog(USHORT_T id)
{
    MutexGuard guard( lock );
    DialogsMap_T::const_iterator it = dialogs.find( id );
    if( it == dialogs.end() )
    {
        return NULL;
    }
    return (*it).second;

}

void Session::closeDialog(Dialog* pDlg)
{
    MutexGuard guard( lock );
    if( !pDlg ) return;
    smsc_log_debug(inapLogger,"Close dialog (SSN=%d, Dialog id=%d)", SSN, pDlg->id );
    dialogs.erase( pDlg->id );
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

    USHORT_T  result = E94InapBindReq(SSN, MSG_USER_ID, TRUE);
    if (result != 0)
    {
        smsc_log_error(inapLogger, "E94InapBindReq failed with code %d(%s)", result,getReturnCodeDescription(result));
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
            smsc_log_debug(inapLogger, "MSG: sender 0x%X, receiver 0x%X, Primitive 0x%X",  msg.sender, msg.receiver, msg.primitive );
            E94InapHandleInd(&msg);
        }

        EINSS7CpReleaseMsgBuffer(&msg);
    }

    result = E94InapUnBindReq( SSN );
    if (result != 0)
    {
        smsc_log_error(inapLogger, "E94InapUnBindReq failed with code %d(%s)", result,getReturnCodeDescription(result));
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
