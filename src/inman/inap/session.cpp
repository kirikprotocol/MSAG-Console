static char const ident[] = "$Id$";

#include <errno.h>
#include <string.h> //for strerrno()
#include <assert.h>
#include <stdexcept>
#include <algorithm>

#include "inman/inap/inss7util.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/common/util.hpp"

using std::max;
using std::map;
using std::pair;
using std::runtime_error;

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


Session::Session(UCHAR_T userssn, UCHAR_T ownssn, const char* ownaddr, UCHAR_T remotessn, const char* remoteaddr)
    : logger(Logger::getInstance("smsc.inman.inap.Session"))
    , SSN( userssn )
    , state( IDLE )
    , lastDialogId( TCAP_DIALOG_MIN_ID )
    , _ac_idx (0)
{
    fillAddress(&ssfAddr,ownaddr, ownssn);
    fillAddress(&scfAddr,remoteaddr, remotessn);

    USHORT_T  result = EINSS7_I97TBindReq( SSN, MSG_USER_ID, TCAP_INSTANCE_ID, EINSS7_I97TCAP_WHITE_USER );
    if (result != 0 )
    throw runtime_error( format( "BindReq() failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

Session::Session(UCHAR_T ownssn, const char* ownaddr, UCHAR_T remotessn, const char* remoteaddr)
    : logger(Logger::getInstance("smsc.inman.inap.Session"))
    , SSN( ownssn )
    , state( IDLE )
    , lastDialogId( TCAP_DIALOG_MIN_ID )
    , _ac_idx (0)
{
    fillAddress(&ssfAddr,ownaddr, ownssn);
    fillAddress(&scfAddr,remoteaddr, remotessn);

    USHORT_T  result = EINSS7_I97TBindReq( SSN, MSG_USER_ID, TCAP_INSTANCE_ID, EINSS7_I97TCAP_WHITE_USER );
    if (result != 0 )
    throw runtime_error( format( "BindReq() failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

//deprecated
Session::Session(UCHAR_T ssn, const char* ssf, const char* scf)
    : logger(Logger::getInstance("smsc.inman.inap.Session"))
    , SSN( ssn )
    , state( IDLE )
    , lastDialogId( TCAP_DIALOG_MIN_ID )
    , _ac_idx (0)
{
    fillAddress(&ssfAddr,ssf, ssn);
    fillAddress(&scfAddr,scf, ssn);

    USHORT_T  result = EINSS7_I97TBindReq( SSN, MSG_USER_ID, TCAP_INSTANCE_ID, EINSS7_I97TCAP_WHITE_USER );
    if (result != 0 )
    throw runtime_error( format( "BindReq() failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

Session::~Session()
{
    USHORT_T result = EINSS7_I97TUnBindReq( SSN, MSG_USER_ID, TCAP_INSTANCE_ID );
    if (result != 0)
    {
      smsc_log_error( logger, format("UnBindReq() failed with code %d (%s)", result, getTcapReasonDescription(result)));
    }

    closeAllDialogs();
}

void    Session::setDialogsAC(const unsigned dialog_ac_idx)
{
    _ac_idx = dialog_ac_idx;
}

UCHAR_T Session::getSSN() const
{
    return SSN;
}

Session::SessionState Session::getState() const
{
    return state;
}

void Session::setState(SessionState newState)
{
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

//protected:
Dialog* Session::registerDialog(Dialog* pDlg, USHORT_T id)
{
    assert(pDlg);
    pDlg->setId(id);
    smsc_log_debug(logger,"Open dialog (SSN=%d, Dialog id=%d)", SSN, id );
    dialogs.insert( DialogsMap_T::value_type( id, pDlg ) );

    for( ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++)
    {
         SessionListener* ptr = *it;
         ptr->onDialogBegin( pDlg );
    }
    return pDlg;
}

// register dialog in session, it's ad hoc method for using
// Session functionality for Dialog successors.
// NOTE: forcedly sets dialogId
Dialog* Session::registerDialog(Dialog* pDlg)
{
    return registerDialog(pDlg, nextDialogId());
}


Dialog* Session::openDialog(unsigned dialog_ac_idx)
{
    USHORT_T id = nextDialogId();
    Dialog* pDlg = new Dialog(this, id, dialog_ac_idx);
    return registerDialog(pDlg, id);
}

Dialog* Session::openDialog()
{
    assert(_ac_idx);
    return openDialog(_ac_idx);
}

Dialog* Session::findDialog(USHORT_T id)
{
    DialogsMap_T::const_iterator it = dialogs.find( id );
    if( it == dialogs.end() )
    {
        return NULL;
    }
    return (*it).second;

}

void Session::closeDialog(Dialog* pDlg)
{
    if( !pDlg ) return;
    smsc_log_debug(logger,"Close dialog (SSN=%d, Dialog id=%d)", SSN, pDlg->did );

  for( ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++)
  {
       SessionListener* ptr = *it;
       ptr->onDialogEnd( pDlg );
  }
}

void Session::closeAllDialogs()
{
    smsc_log_debug(logger,"Close all dialogs");

    for( DialogsMap_T::iterator it = dialogs.begin(); it != dialogs.end(); it++ )
    {
        closeDialog( (*it).second );
    }
}

} // namespace inap
} // namespace inman
} // namespace smsc

