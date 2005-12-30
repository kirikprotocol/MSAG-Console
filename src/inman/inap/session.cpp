static char const ident[] = "$Id$";

#include <assert.h>

#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/common/util.hpp"


using smsc::inman::common::fillAddress;
//using smsc::inman::common::format;

namespace smsc  {
namespace inman {
namespace inap  {

/////////////////////////////////////////////////////////////////////////////////////
// TCAP Session (dialogs factory)
/////////////////////////////////////////////////////////////////////////////////////

Session::Session(UCHAR_T ownssn, const char* ownaddr, UCHAR_T remotessn, const char* remoteaddr, Logger *uselog/* = NULL*/)
    : logger(uselog), SSN(ownssn), state( IDLE )
    , lastDialogId( TCAP_DIALOG_MIN_ID ), _ac_idx (0)
{
    if (!uselog)
        logger = Logger::getInstance("smsc.inman.inap.Session");
    fillAddress(&ssfAddr, ownaddr, ownssn);
    fillAddress(&scfAddr, remoteaddr, remotessn);
}

Session::Session(UCHAR_T ssn, const char* ssf, const char* scf, Logger *uselog/* = NULL*/)
{
    Session::Session(ssn, ssf, ssn, scf, uselog);
}

Session::~Session()
{
    closeAllDialogs();
}

void    Session::setDialogsAC(const unsigned dialog_ac_idx)
{
    _ac_idx = dialog_ac_idx;
}

UCHAR_T Session::getSSN(void) const
{
    return SSN;
}

Session::SessionState Session::getState(void) const
{
    return state;
}

void Session::setState(SessionState newState)
{
    state = newState;
}

USHORT_T Session::nextDialogId(void)
{
    USHORT_T id = lastDialogId;
    if ( ++lastDialogId  > TCAP_DIALOG_MAX_ID )
        lastDialogId = TCAP_DIALOG_MIN_ID;

    return id;
}

//protected:
Dialog* Session::registerDialog(Dialog* pDlg, USHORT_T id)
{
    assert(pDlg);
    pDlg->setId(id);
    smsc_log_debug(logger, "SSN[%u]: Opening dialog id=%u", (unsigned)SSN, id);
    dialogs.insert( DialogsMap_T::value_type( id, pDlg ) );

    for ( ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
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
        return NULL;
    return (*it).second;
}

void Session::closeDialog(Dialog* pDlg)
{
    if (!pDlg)
        return;
    for( ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
        SessionListener* ptr = *it;
        ptr->onDialogEnd(pDlg);
    }
    dialogs.erase(pDlg->getId());
    smsc_log_debug(logger, "SSN[%u]: Closed dialog id=%u", (unsigned)SSN, pDlg->getId());
}

void Session::closeAllDialogs()
{
    smsc_log_debug(logger, "SSN[%u]: Closing all dialogs ..", (unsigned)SSN);
    for( DialogsMap_T::iterator it = dialogs.begin(); it != dialogs.end(); it++ )
        closeDialog((*it).second);
}

} // namespace inap
} // namespace inman
} // namespace smsc

