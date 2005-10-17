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
  ac.acLen=7;
//  ac.ac[0] = 0x06; //|00000110 |Tag                    |(UNIV P Obj Identifier)
//  ac.ac[1] = 0x07; //|00000111 |Length                 |7
  ac.ac[0] = 0x04; //|00000100 |Authority,Organization |ITU-T, Identified-organization
  ac.ac[1] = 0x00; //|00000000 |                       |ETSI
  ac.ac[2] = 0x00; //|00000000 |Domain                 |Mobile Domain
  ac.ac[3] = 0x01; //|00000001 |Mobile Subdomain       |GSM / UMTS Network
  ac.ac[4] = 0x15; //|00010101 |Common Component ID    |CAP 3 OE
  ac.ac[5] = 0x03; //|00000011 |CAP3 OE ID             |ACE
  ac.ac[6] = 0x3D; //|00111101 |Application Context    |CAP3-SMS
}

Session::Session(UCHAR_T userssn, UCHAR_T ownssn, const char* ownaddr, UCHAR_T remotessn, const char* remoteaddr)
    : logger(Logger::getInstance("smsc.inman.inap.Session"))
    , SSN( userssn )
    , state( IDLE )
    , lastDialogId( TCAP_DIALOG_MIN_ID )
{
    fillAddress(&ssfAddr,ownaddr, ownssn);
    fillAddress(&scfAddr,remoteaddr, remotessn);
    _ac_idx = id_ac_cap3_sms_AC;
    //fillAppContext(&ac);

    USHORT_T  result = EINSS7_I97TBindReq( SSN, MSG_USER_ID, TCAP_INSTANCE_ID, EINSS7_I97TCAP_WHITE_USER );
    if (result != 0 )
    throw runtime_error( format( "BindReq() failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

Session::Session(UCHAR_T ownssn, const char* ownaddr, UCHAR_T remotessn, const char* remoteaddr)
    : logger(Logger::getInstance("smsc.inman.inap.Session"))
    , SSN( ownssn )
    , state( IDLE )
    , lastDialogId( TCAP_DIALOG_MIN_ID )
{
    fillAddress(&ssfAddr,ownaddr, ownssn);
    fillAddress(&scfAddr,remoteaddr, remotessn);
    //ac = *smsc::ac::ACOID::OIDbyIdx(id_ac_cap3_sms_AC);
    _ac_idx = id_ac_cap3_sms_AC;

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
{
    fillAddress(&ssfAddr,ssf, ssn);
    fillAddress(&scfAddr,scf, ssn);
    //ac = *smsc::ac::ACOID::OIDbyIdx(id_ac_cap3_sms_AC);
    _ac_idx = id_ac_cap3_sms_AC;

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
// register dialog with session, it's ad hoc method
// require to set allocated dialogue id to dialog object
Dialog* Session::registerDialog(Dialog* pDlg)
{
  if (pDlg)
  {
    USHORT_T id = nextDialogId();
    pDlg->setId(id);
    smsc_log_debug(logger,"Open dialog (SSN=%d, Dialog id=%d)", SSN, id );
    dialogs.insert( DialogsMap_T::value_type( id, pDlg ) );

    for( ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++)
    {
         SessionListener* ptr = *it;
         ptr->onDialogBegin( pDlg );
    }
  }
  return pDlg;
}

//Dialog* Session::openDialog(USHORT_T id, const APP_CONTEXT_T& dac)
Dialog* Session::openDialog(USHORT_T id, unsigned dialog_ac_idx)
{
  if(id == 0) id = nextDialogId();
  smsc_log_debug(logger,"Open dialog (SSN=%d, Dialog id=%d)", SSN, id );
  Dialog* pDlg = new Dialog( this, id , dialog_ac_idx);
  dialogs.insert( DialogsMap_T::value_type( id, pDlg ) );

  for( ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++)
  {
       SessionListener* ptr = *it;
       ptr->onDialogBegin( pDlg );
  }

  return pDlg;
}

//deprecated
Dialog* Session::openDialog(USHORT_T id)
{
    if(id == 0) id = nextDialogId();
    smsc_log_debug(logger,"Open dialog (SSN=%d, Dialog id=%d)", SSN, id );
//    Dialog* pDlg = new Dialog( this, id , ac);
    Dialog* pDlg = new Dialog( this, id , _ac_idx);
    dialogs.insert( DialogsMap_T::value_type( id, pDlg ) );

  for( ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++)
  {
       SessionListener* ptr = *it;
       ptr->onDialogBegin( pDlg );
  }

    return pDlg;
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
} // namespace inmgr
} // namespace smsc
