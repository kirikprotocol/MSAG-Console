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

Session::Session(UCHAR_T ssn, const char* scfNum, const char* inmanNum)
    : logger(Logger::getInstance("smsc.inman.inap.Session"))
    , SSN( ssn )
    , state( IDLE )
    , lastDialogId( TCAP_DIALOG_MIN_ID )
{
  	fillAddress(&scfAddr,scfNum, ssn);
  	fillAddress(&inmanAddr,inmanNum, ssn);
  	fillAppContext(&ac);

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


TcapDialog* Session::openDialog(USHORT_T id)
{
    if(id == 0) id = nextDialogId();
    smsc_log_debug(logger,"Open dialog (SSN=%d, TcapDialog id=%d)", SSN, id );
    TcapDialog* pDlg = new TcapDialog( this, id );
    dialogs.insert( DialogsMap_T::value_type( id, pDlg ) );
    notify1<TcapDialog*>( &SessionListener::onDialogBegin, pDlg );
    return pDlg;
}

TcapDialog* Session::findDialog(USHORT_T id)
{
    DialogsMap_T::const_iterator it = dialogs.find( id );
    if( it == dialogs.end() )
    {
        return NULL;
    }
    return (*it).second;

}

void Session::closeDialog(TcapDialog* pDlg)
{
    if( !pDlg ) return;
    smsc_log_debug(logger,"Close dialog (SSN=%d, TcapDialog id=%d)", SSN, pDlg->did );
    dialogs.erase( pDlg->did );
    notify1<TcapDialog*>( &SessionListener::onDialogEnd, pDlg );
    delete pDlg;
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
