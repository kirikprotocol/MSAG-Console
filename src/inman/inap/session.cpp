static char const ident[] = "$Id$";

#include <assert.h>
#include "inman/common/util.hpp"
#include "inman/common/adrutil.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"

using smsc::cvtutil::packSCCPAddress;

#define MAX_ID_ATTEMPTS (maxId - minId)

namespace smsc  {
namespace inman {
namespace inap  {

/////////////////////////////////////////////////////////////////////////////////////
// TCAP Session (TCAP dialogs factory, one per SSN)
/////////////////////////////////////////////////////////////////////////////////////
SSNSession::SSNSession(UCHAR_T ownssn, USHORT_T user_id, Logger * uselog/* = NULL*/)
    : logger(uselog), SSN(ownssn), userId(user_id), state(ssnIdle)
    , lastDlgId(0), ac_idx(ACOID::id_ac_NOT_AN_OID), iType(ssnMultiRoute)
    , maxId(0), minId(0)
{
    locAddr.addrLen = rmtAddr.addrLen = 0;
    if (!uselog)
        logger = Logger::getInstance("smsc.inman.inap.Session");
}

void SSNSession::init(const char* own_addr, ACOID::DefinedOIDidx dialog_ac_idx,
                    const char* rmt_addr/* = NULL*/, UCHAR_T rmt_ssn/* = 0*/,
                    USHORT_T max_id/* = 2000*/, USHORT_T min_id/* = 1*/)
{
    ownAdr += own_addr;
    packSCCPAddress(&locAddr, own_addr, SSN);
    if (rmt_addr) {
        packSCCPAddress(&rmtAddr, rmt_addr, rmt_ssn);
        iType = ssnSingleRoute;
    } else if (rmt_ssn) {
        rmtAddr.addrLen = 1;
        rmtAddr.addr[0] = rmt_ssn;
        iType = ssnMultiAddress;
    }// else { rmtAddr.addrLen = 0; iType = ssnMultiRoute; }
    ac_idx = dialog_ac_idx;
    maxId = max_id;
    lastDlgId = minId = min_id;
}

SSNSession::~SSNSession()
{
    MutexGuard tmp(dlgGrd);
    for (DialogsLIST::iterator it = pool.begin(); it != pool.end(); it++) {
        delete *it;
    }
    pool.clear();
    if (!(dialogs.size() + pending.size()))
        return;
    smsc_log_debug(logger, "SSN[%u]: Closing %u dialogs ..",
                    (unsigned)SSN, dialogs.size() + pending.size());
    if (dialogs.size()) {
        for (DialogsMap_T::iterator it = dialogs.begin(); it != dialogs.end(); it++) {
            USHORT_T dId = (*it).first;
            Dialog* pDlg = (*it).second;
            if (!(pDlg->getState().value & TC_DLG_CLOSED_MASK))
                smsc_log_warn(logger,
                    "SSN[%u]: Dialog[%u](0x%x) is active, %u invokes pending",
                    (unsigned)SSN, dId, pDlg->getState().value, pDlg->pendingInvokes());
            delete (*it).second;
        }
        dialogs.clear();
    }
    if (pending.size()) {
        for (DlgTimesMap_T::iterator it = pending.begin(); it != pending.end(); it++) {
            USHORT_T dId = (*it).first;
            DlgTime dtm  = (*it).second;
            smsc_log_warn(logger, "SSN[%u]: Dialog[%u](0x%x) is not terminated, %u invokes pending",
                           (unsigned)SSN, dId, dtm.dlg->getState().value, dtm.dlg->pendingInvokes());
            delete dtm.dlg;
        }
        pending.clear();
    }
}

Dialog* SSNSession::openDialog(UCHAR_T rmt_ssn, const char* rmt_addr)
{
    SCCP_ADDRESS_T  sccp_addr;
    MutexGuard tmp(dlgGrd);
    packSCCPAddress(&sccp_addr, rmt_addr, rmt_ssn);
    return initDialog(sccp_addr);
}

Dialog* SSNSession::openDialog(const char* rmt_addr)
{
    assert(iType == ssnMultiAddress);
    return openDialog(rmtAddr.addr[0], rmt_addr);
}

Dialog* SSNSession::openDialog(void)
{
    assert(iType == ssnSingleRoute);
    MutexGuard tmp(dlgGrd);
    return initDialog(rmtAddr);
}

Dialog* SSNSession::findDialog(USHORT_T dId)
{
    MutexGuard tmp(dlgGrd);
    return locateDialog(dId);
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
        if (!(pDlg->getState().value & TC_DLG_CLOSED_MASK)) {
            DlgTime     dtm;
            gettimeofday(&dtm.tms, 0);
            dtm.dlg = pDlg;
            pending.insert(DlgTimesMap_T::value_type(dId, dtm));
            smsc_log_warn(logger,
                "SSN[%u]: Pushed aside unterminated dialog[%u](0x%x), %u invokes pending",
                (unsigned)SSN, dId, pDlg->getState().value, pDlg->pendingInvokes());
        } else {
            pool.push_back(pDlg);
            smsc_log_debug(logger, "SSN[%u]: Released terminated dialog[%u](0x%x)",
                (unsigned)SSN, dId, pDlg->getState().value);
        }
    }
}

void SSNSession::releaseDialogs(void)
{
    smsc_log_debug(logger, "SSN[%u]: Releasing %u dialogs ..", (unsigned)SSN, dialogs.size());
    MutexGuard tmp(dlgGrd);
    for (DialogsMap_T::iterator it = dialogs.begin(); it != dialogs.end(); it++) {
        Dialog* pDlg = (*it).second;
        USHORT_T dId = (*it).first;

        if (!(pDlg->getState().value & TC_DLG_CLOSED_MASK)) {
            DlgTime     dtm;
            gettimeofday(&dtm.tms, 0);
            dtm.dlg = pDlg;
            pending.insert(DlgTimesMap_T::value_type(dId, dtm));
            smsc_log_warn(logger, "SSN[%u]: Pushed aside unterminated dialog[%u], %u invokes pending",
                           (unsigned)SSN, dId, pDlg->pendingInvokes());
        } else {
            pool.push_back(pDlg);
            smsc_log_debug(logger, "SSN[%u]: Released terminated dialog[%u]",
                           (unsigned)SSN, dId);
        }
    }
    dialogs.clear();
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * NOTE: these methods are not the processing graph entries, so never lock Mutex,
 *       it's a caller responsibility to lock it !!!
 * ---------------------------------------------------------------------------------- */
Dialog* SSNSession::initDialog(const SCCP_ADDRESS_T & rmt_addr)
{
    if (state != ssnBound) {
        smsc_log_error(logger, "SSN[%u]: not bounded!", (unsigned)SSN);
        return NULL;
    }
    if (!pool.size())
        cleanUpDialogs();

    Dialog* pDlg = NULL;
    USHORT_T did = 0;
    if (!nextDialogId(did)) {
        smsc_log_fatal(logger, "SSN[%u]: Dialogs exhausted [%u of %u], active(%u), pending(%u)",
            (unsigned)SSN, dialogs.size() + pending.size(),
            maxId - minId, dialogs.size(), pending.size());
        if (logger->isDebugEnabled())
            dumpDialogs();
        return NULL;
    }

    if (pool.size()) {
        pDlg = *(pool.begin());
        pool.pop_front();
        pDlg->reset(did, &rmt_addr);
    } else
        pDlg = new Dialog(did, ac_idx, userId, locAddr, rmt_addr, logger);

    smsc_log_debug(logger, "SSN[%u]: Opening dialog[%u]", (unsigned)SSN, did);
    dialogs.insert(DialogsMap_T::value_type(did, pDlg));
    return pDlg;
}

bool SSNSession::nextDialogId(USHORT_T & dId)
{
    USHORT_T attempt = 0;
    do {
        dId = lastDlgId;
        if (++lastDlgId  > maxId)
            lastDlgId = minId;
    } while (locateDialog(dId) && ((++attempt) < MAX_ID_ATTEMPTS));

    return (attempt < MAX_ID_ATTEMPTS) ? true : false;
}

void SSNSession::cleanUpDialogs(void)
{
    DlgTimesMap_T::iterator it  = pending.begin();
    while (it != pending.end()) {
        DlgTime dtm = (*it).second;
        DlgTimesMap_T::iterator curr = it++;
        if ((dtm.dlg->getState().value & TC_DLG_CLOSED_MASK) != 0) {
            pool.push_back(dtm.dlg);
            pending.erase(curr);
        }
    }
}

Dialog* SSNSession::locateDialog(USHORT_T dId)
{
    Dialog* pDlg = NULL;
    DialogsMap_T::const_iterator it = dialogs.find(dId);
    if (it == dialogs.end()) {
        DlgTimesMap_T::iterator tit = pending.find(dId);
        pDlg = (tit == pending.end()) ? NULL : ((*tit).second).dlg;
    } else
        pDlg = (*it).second;
    return pDlg;
}

void SSNSession::dumpDialogs(void)
{
    std::string dump;
    format(dump, "SSN[%u]: Dialogs [%u of %u], ", (unsigned)SSN,
           dialogs.size() + pending.size(), maxId - minId);

    if (pending.size()) {
        format(dump, "pending(%u): ", pending.size());
        struct timeval ctm;
        gettimeofday(&ctm, 0);

        for (DlgTimesMap_T::const_iterator it = pending.begin(); it != pending.end(); it++) {
            DlgTime dtm = (*it).second;
            long tdif = ctm - dtm.tms;
            format(dump, "%u(0x%x):%lus ", (*it).first, dtm.dlg->getState().value, tdif);
        }
    }
    format(dump, "active(%u): ", dialogs.size());
    for (DialogsMap_T::const_iterator it = dialogs.begin(); it != dialogs.end(); it++) {
        Dialog* pDlg = (*it).second;
        format(dump, "%u(0x%x) ", (*it).first, pDlg->getState().value);
    }
    smsc_log_debug(logger, dump.c_str());
}

} // namespace inap
} // namespace inman
} // namespace smsc

