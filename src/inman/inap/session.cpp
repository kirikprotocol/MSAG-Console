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
// TCAP Session (TCAP dialogs factory, one per SSN)
/////////////////////////////////////////////////////////////////////////////////////
SSNSession::SSNSession(UCHAR_T ownssn, Logger * uselog/* = NULL*/)
    : logger(uselog), SSN(ownssn), state(ssnIdle)
    , lastDlgId(TCAP_DIALOG_MIN_ID), ac_idx(0)
{
    locAddr.addrLen = rmtAddr.addrLen = 0;
    if (!uselog)
        logger = Logger::getInstance("smsc.inman.inap.Session");
}

void SSNSession::init(const char* own_addr, /*UCHAR_T rmt_ssn,*/
                    const char* rmt_addr, unsigned dialog_ac_idx)
{
    fillAddress(&locAddr, own_addr, SSN);
    fillAddress(&rmtAddr, rmt_addr, SSN /*rmt_ssn*/);
    ac_idx = dialog_ac_idx;
}

SSNSession::~SSNSession()
{
    releaseDialogs();
    MutexGuard tmp(dlgGrd);
    for (DialogsLIST::iterator it = pool.begin(); it != pool.end(); it++) {
        delete *it;
    }
}

USHORT_T SSNSession::nextDialogId(void)
{
    USHORT_T id = lastDlgId;
    if (++lastDlgId  > TCAP_DIALOG_MAX_ID)
        lastDlgId = TCAP_DIALOG_MIN_ID;
    return id;
}

void SSNSession::cleanUpDialogs(void)
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

Dialog* SSNSession::openDialog(void)
{
    if (state != ssnBound) {
        smsc_log_error(logger, "SSN[%u]: not bounded!");
        return NULL;
    }
    if (!pool.size())
        cleanUpDialogs();

    Dialog* pDlg = NULL;
    USHORT_T did = nextDialogId();

    MutexGuard tmp(dlgGrd);
    if (pool.size()) {
        pDlg = *(pool.begin());
        pool.pop_front();
        pDlg->reset(did);
    } else
        pDlg = new Dialog(did, ac_idx, locAddr, rmtAddr, logger);

    smsc_log_debug(logger, "SSN[%u]: Opening dialog[%u]", (unsigned)SSN, did);
    dialogs.insert(DialogsMap_T::value_type(did, pDlg));
    return pDlg;
}


Dialog* SSNSession::findDialog(USHORT_T dId)
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

void SSNSession::releaseDialog(Dialog* pDlg)
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

void SSNSession::releaseDialogs(void)
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

} // namespace inap
} // namespace inman
} // namespace smsc

