static char const ident[] = "$Id$";

#include <assert.h>

#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/common/util.hpp"

using smsc::inman::common::fillAddress;

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
    releaseDialogs();
    MutexGuard tmp(dlgGrd);
    for (DialogsLIST::iterator it = pool.begin(); it != pool.end(); it++) {
        delete *it;
    }
}

void    Session::setDialogsAC(const unsigned dialog_ac_idx)
{
    _ac_idx = dialog_ac_idx;
}

void  Session::getRoute(SCCP_ADDRESS_T & ownAddr, SCCP_ADDRESS_T & rmtAddr) const
{
    ownAddr = ssfAddr;
    rmtAddr = scfAddr;
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

void Session::cleanUpDialogs(void)
{
    MutexGuard tmp(dlgGrd);
    DialogsMap_T::iterator it  = pending.begin();
    while (it != pending.end()) {
        Dialog* pDlg = (*it).second;
        DialogsMap_T::iterator curr = it++;
        if (!pDlg->pendingInvokes()) {
            pool.push_back(pDlg);
            pending.erase(curr);
        }
    }
}

Dialog* Session::openDialog(unsigned dialog_ac_idx)
{
    if (state != Session::BOUND) {
        smsc_log_error(logger, "SSN[%u]: not bounded!");
        return NULL;
    }
    Dialog* pDlg = NULL;
    USHORT_T id = nextDialogId();

    if (!pool.size())
        cleanUpDialogs();

    MutexGuard tmp(dlgGrd);
    if (pool.size()) {
        pDlg = *(pool.begin());
        pool.pop_front();
        pDlg->reset(id, dialog_ac_idx); 
    } else
        pDlg = new Dialog(this, id, dialog_ac_idx);

    smsc_log_debug(logger, "SSN[%u]: Opening dialog[%u]", (unsigned)SSN, id);
    dialogs.insert(DialogsMap_T::value_type(id, pDlg));
    return pDlg;
}

Dialog* Session::openDialog(void)
{
    assert(_ac_idx);
    return openDialog(_ac_idx);
}

Dialog* Session::findDialog(USHORT_T dId)
{
    Dialog* pDlg = NULL;
    MutexGuard tmp(dlgGrd);
    DialogsMap_T::const_iterator it = dialogs.find(dId);
    if (it == dialogs.end()) {
        it = pending.find(dId);
        pDlg = (it == pending.end()) ? NULL : (*it).second;
    } else
        pDlg = (*it).second;
    return pDlg;
}

void Session::releaseDialog(Dialog* pDlg)
{
    if (!pDlg)
        return;
    MutexGuard tmp(dlgGrd);
    USHORT_T dId = pDlg->getId();
    DialogsMap_T::iterator it = dialogs.find(dId);
    if ((it == dialogs.end()) || (pDlg != (*it).second)) {
        smsc_log_error(logger, "SSN[%u]: Unregistered/illegal dialog[%u]", (unsigned)SSN, dId);
    } else {
        dialogs.erase(it);
        unsigned inv = pDlg->pendingInvokes();
        if (inv) {
            pending.insert(DialogsMap_T::value_type(dId, pDlg));
            smsc_log_debug(logger, "SSN[%u]: Released dialog[%u], %u invokes pending",
                           (unsigned)SSN, dId, inv);
        } else {
            pool.push_back(pDlg);
            smsc_log_debug(logger, "SSN[%u]: Released dialog[%u]",
                           (unsigned)SSN, dId);
        }
    }
}

void Session::releaseDialogs(void)
{
    smsc_log_debug(logger, "SSN[%u]: Releasing all dialogs ..", (unsigned)SSN);
    MutexGuard tmp(dlgGrd);
    for (DialogsMap_T::iterator it = dialogs.begin(); it != dialogs.end(); it++) {
        Dialog* pDlg = (*it).second;
        USHORT_T dId = (*it).first;
        unsigned inv = pDlg->pendingInvokes();
        if (inv) {
            pending.insert(DialogsMap_T::value_type(dId, pDlg));
            smsc_log_debug(logger, "SSN[%u]: Released dialog[%u], %u invokes pending",
                           (unsigned)SSN, dId, inv);
        } else {
            pool.push_back(pDlg);
            smsc_log_debug(logger, "SSN[%u]: Released dialog[%u]",
                           (unsigned)SSN, dId);
        }
    }
    dialogs.clear();
}

// register dialog in session, it's ad hoc method for using
// Session functionality for Dialog successors.
// NOTE: forcedly sets dialogId
//Dialog* Session::registerDialog(Dialog* pDlg)
//{
//    assert(pDlg);
//    USHORT_T id = nextDialogId();
//    smsc_log_debug(logger, "SSN[%u]: Opening dialog id=%u", (unsigned)SSN, id);
//    dlgGrd.Lock();
//    pDlg->setId(id);
//    dialogs.insert( DialogsMap_T::value_type( id, pDlg ) );
//    dlgGrd.Unlock();
//    return pDlg;
//}


} // namespace inap
} // namespace inman
} // namespace smsc

