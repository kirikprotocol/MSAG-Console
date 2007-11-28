#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "util/vformat.hpp"
using smsc::util::format;

#include "inman/common/adrutil.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/inap/dispatcher.hpp"
using smsc::cvtutil::packSCCPAddress;
using smsc::cvtutil::unpackSCCP2SSN_GT;

#define MAX_ID_ATTEMPTS (maxId - minId)

namespace smsc  {
namespace inman {
namespace inap  {

/* ************************************************************************** *
 * class TCSessionAC implementation:
 * ************************************************************************** */
TCSessionAC::TCSessionAC(USHORT_T uid, SSNSession * owner, UCHAR_T fake_ssn,
                const TonNpiAddress & own_addr, ACOID::DefinedOIDidx dlg_ac_idx)
    : tcUID(uid), _owner(owner), ac_idx(dlg_ac_idx), ownAdr(own_addr)
{
    ownAdr.fixISDN();
    senderSsn = _owner->getSSN();
    packSCCPAddress(&locAddr, ownAdr.getSignals(), fake_ssn ? fake_ssn : senderSsn);
}

TCSessionAC::~TCSessionAC(void)
{
    MutexGuard  tmp(dlgGrd);
    if (pool.size()) {
        TCDialogsLIST::iterator it = pool.begin();
        for (; it != pool.end(); it++)
            delete (*it);
        pool.clear();
    }
}
//NOTE: it's important that mkSignature doesn't rely on addresses .Ton.Npi
void TCSessionAC::mkSignature(std::string & sid, UCHAR_T own_ssn, const TonNpiAddress & onpi,
                             ACOID::DefinedOIDidx dlg_ac_idx,
                             UCHAR_T rmt_ssn, const TonNpiAddress * rnpi)
{
    char rssn[8]; // sizeof("255")
    if (rmt_ssn)
        snprintf(rssn, sizeof(rssn), "%u", rmt_ssn);
    else {
        rssn[0] = '*'; rssn[1] = 0;
    }
    format(sid, "%u:%s[%u]->%s:%s", own_ssn, onpi.getSignals(), dlg_ac_idx, rssn,
           rnpi ? rnpi->getSignals() : "*");
}


void TCSessionAC::release(void)
{
    MutexGuard  tmp(dlgGrd);
    _owner->closeTCSession(this);
    _owner = NULL;
}


void TCSessionAC::releaseDialogs(void)
{
    _owner->releaseDialogs(&sign);
}

void TCSessionAC::releaseDialog(Dialog* pDlg)
{
    _owner->releaseDialog(pDlg, &sign);
}

Dialog * TCSessionAC::initDialog(const SCCP_ADDRESS_T & rmt_addr)
{
    USHORT_T dId = 0;
    if ((_owner->getState() != smsc::inman::inap::ssnBound)
        || !_owner->getDialogId(dId))
        return NULL; //todo: throw ?

    Dialog * pDlg = NULL;
    {
        MutexGuard  tmp(dlgGrd);
        if (pool.size()) {
            pDlg = *(pool.begin());
            pool.pop_front();
        } else
            pDlg = new Dialog(sign, dId, _owner->getMsgUserId(), ac_idx,
                              locAddr, senderSsn, NULL);
    }
    pDlg->reset(dId, &rmt_addr);
    _owner->markDialog(pDlg);
    return pDlg;
}

void TCSessionAC::toPool(Dialog * p_dlg)
{
    MutexGuard  tmp(dlgGrd);
    pool.push_back(p_dlg);
}
/* ************************************************************************** *
 * class TCSessionMR implementation:
 * ************************************************************************** */
TCSessionMR::TCSessionMR(USHORT_T uid, SSNSession * owner, UCHAR_T fake_ssn,
                const TonNpiAddress & own_addr, ACOID::DefinedOIDidx dlg_ac_idx)
    : TCSessionAC(uid, owner, fake_ssn, own_addr, dlg_ac_idx)
{
    mkSignature(sign, owner->getSSN(), own_addr, dlg_ac_idx, 0, NULL);
}

Dialog* TCSessionMR::openDialog(UCHAR_T rmt_ssn, const char* rmt_addr)
{
    TonNpiAddress   rnpi;
    SCCP_ADDRESS_T  rmtAddr;

    if (!rnpi.fromText(rmt_addr) || !rnpi.fixISDN())
        return NULL;

    packSCCPAddress(&rmtAddr, rnpi.getSignals(), rmt_ssn);
    return initDialog(rmtAddr);
}

/* ************************************************************************** *
 * class TCSessionMA implementation:
 * ************************************************************************** */
TCSessionMA::TCSessionMA(USHORT_T uid, SSNSession * owner, UCHAR_T fake_ssn,
                const TonNpiAddress & own_addr, ACOID::DefinedOIDidx dlg_ac_idx,
                UCHAR_T rmt_ssn)
    : TCSessionAC(uid, owner, fake_ssn, own_addr, dlg_ac_idx)
    , rmtSSN(rmt_ssn)
{
    mkSignature(sign, owner->getSSN(), own_addr, dlg_ac_idx, rmtSSN, NULL);
}

Dialog* TCSessionMA::openDialog(const char* rmt_addr)
{
    TonNpiAddress   rnpi;
    if (!rnpi.fromText(rmt_addr) || !rnpi.fixISDN())
        return NULL;

    return openDialog(rnpi);
}

Dialog* TCSessionMA::openDialog(const TonNpiAddress & rnpi)
{
    if ((rnpi.numPlanInd != NUMBERING_ISDN)
        || (rnpi.typeOfNumber > ToN_INTERNATIONAL))
        return NULL; //todo: throw

    SCCP_ADDRESS_T  rmtAddr;
    packSCCPAddress(&rmtAddr, rnpi.getSignals(), rmtSSN);
    return initDialog(rmtAddr);
}

/* ************************************************************************** *
 * class TCSessionSR implementation:
 * ************************************************************************** */
TCSessionSR::TCSessionSR(USHORT_T uid, SSNSession * owner, UCHAR_T fake_ssn,
                const TonNpiAddress & own_addr, ACOID::DefinedOIDidx dlg_ac_idx,
                UCHAR_T rmt_ssn, const TonNpiAddress & rmt_addr)
    : TCSessionAC(uid, owner, fake_ssn, own_addr, dlg_ac_idx)
    , rmtNpi(rmt_addr)
{
    mkSignature(sign, owner->getSSN(), own_addr, dlg_ac_idx, rmt_ssn, &rmtNpi);
    packSCCPAddress(&rmtAddr, rmtNpi.getSignals(), rmt_ssn);
}

Dialog* TCSessionSR::openDialog(void)
{
    if (!rmtNpi.fixISDN())
        return NULL;
    return initDialog(rmtAddr);
}

/* ************************************************************************** *
 * class SSNSession implementation (TCAP dialogs/sessions factory):
 * ************************************************************************** */
SSNSession::SSNSession(UCHAR_T ownssn, USHORT_T user_id, USHORT_T max_dlg_id/* = 2000*/,
                       USHORT_T min_dlg_id/* = 1*/, Logger * uselog/* = NULL*/)
    : logger(uselog), _SSN(ownssn), msgUserId(user_id), state(ssnIdle)
    , maxId(max_dlg_id), lastTCSUId(0)
{
    if (!min_dlg_id)
        min_dlg_id++;
    lastDlgId = minId = min_dlg_id;
    if (maxId < minId)
        maxId = minId;

    if (!uselog)
        logger = Logger::getInstance("smsc.inman.inap.SSN");
}

SSNSession::~SSNSession()
{
    MutexGuard tmp(mutex);
    if (tcSessions.size()) { //kill TC sessions
        for (TCSessionsMAP::iterator sit = tcSessions.begin();
                                    sit != tcSessions.end(); sit++)
            delete (*sit).second;
    }
    if (deadSess.size()) {
        for (TCSessionsLIST::iterator sit = deadSess.begin();
                                    sit != deadSess.end(); sit++)
            delete (*sit);
    }
    cleanUpDialogs();

    if (!(dialogs.size() + pending.size()))
        return;

    smsc_log_debug(logger, "SSN[%u]: Closing %u dialogs ..",
                    (unsigned)_SSN, dialogs.size() + pending.size());
    if (dialogs.size()) {
        for (DialogsMAP::iterator it = dialogs.begin(); (it != dialogs.end()) && (*it).second; it++) {
            USHORT_T dId = (*it).first;
            Dialog* pDlg = (*it).second;
            unsigned invNum = 0;
            if (!pDlg->isFinished(&invNum))
                smsc_log_warn(logger,
                    "SSN[%u]: Dialog[0x%X](0x%x) is active, %u invokes pending",
                    (unsigned)_SSN, dId, pDlg->getState().value, invNum);
            delete (*it).second;
        }
        dialogs.clear();
    }
    if (pending.size()) {
        for (DlgTimesMAP::iterator it = pending.begin(); it != pending.end(); it++) {
            USHORT_T dId = (*it).first;
            DlgTime dtm  = (*it).second;
            smsc_log_warn(logger, "SSN[%u]: Dialog[0x%X](0x%x) is active, %u invokes pending",
                           (unsigned)_SSN, dId, dtm.dlg->getState().value, dtm.dlg->pendingInvokes());
            delete dtm.dlg;
        }
        pending.clear();
    }
}

TCSessionMR * SSNSession::newMRsession(const char* own_addr, ACOID::DefinedOIDidx dlg_ac_idx,
                                       UCHAR_T fake_ssn/* = 0*/)
{
    TonNpiAddress   onpi;
    if (!onpi.fromText(own_addr))
        return NULL;
    return newMRsession(onpi, dlg_ac_idx, fake_ssn);
}

TCSessionMR * SSNSession::newMRsession(const TonNpiAddress & onpi, ACOID::DefinedOIDidx dlg_ac_idx,
                                       UCHAR_T fake_ssn/* = 0*/)
{
    TCSessionMR *   pSess = NULL;

    if ((onpi.numPlanInd != NUMBERING_ISDN) || (onpi.typeOfNumber > ToN_INTERNATIONAL))
        return NULL;
//    onpi.typeOfNumber = ToN_INTERNATIONAL; //correct isdn unknown
    {
        MutexGuard tmp(mutex);
        std::string sid;
        TCSessionAC::mkSignature(sid, _SSN, onpi, dlg_ac_idx, 0, NULL);

        TCSessionsMAP::iterator it = tcSessions.find(sid);
        if (it != tcSessions.end())
            pSess = static_cast<TCSessionMR*>((*it).second);
        else {
            pSess = new TCSessionMR(++lastTCSUId, this, fake_ssn, onpi, dlg_ac_idx);
            tcSessions.insert(TCSessionsMAP::value_type(pSess->Signature(), pSess));
        }
    }
    return pSess;
}


TCSessionMA * SSNSession::newMAsession(const char* own_addr, ACOID::DefinedOIDidx dlg_ac_idx,
                            UCHAR_T rmt_ssn, UCHAR_T fake_ssn/* = 0*/)
{
    TonNpiAddress   onpi;
    if (!onpi.fromText(own_addr) || !onpi.fixISDN())
        return NULL;
    return newMAsession(onpi, dlg_ac_idx, rmt_ssn, fake_ssn);
}

TCSessionMA * SSNSession::newMAsession(const TonNpiAddress & onpi, ACOID::DefinedOIDidx dlg_ac_idx,
                            UCHAR_T rmt_ssn, UCHAR_T fake_ssn/* = 0*/)
{
    TCSessionMA *   pSess = NULL;

    if ((onpi.numPlanInd != NUMBERING_ISDN) || (onpi.typeOfNumber > ToN_INTERNATIONAL))
        return NULL;
//    onpi.typeOfNumber = ToN_INTERNATIONAL; //correct isdn unknown
    {
        MutexGuard tmp(mutex);
        std::string sid;
        TCSessionAC::mkSignature(sid, _SSN, onpi, dlg_ac_idx, rmt_ssn, NULL);

        TCSessionsMAP::iterator it = tcSessions.find(sid);
        if (it != tcSessions.end())
            pSess = static_cast<TCSessionMA*>((*it).second);
        else {
            pSess = new TCSessionMA(++lastTCSUId, this, fake_ssn, onpi, dlg_ac_idx, rmt_ssn);
            tcSessions.insert(TCSessionsMAP::value_type(pSess->Signature(), pSess));
        }
    }
    return pSess;
}


TCSessionSR * SSNSession::newSRsession(const char* own_addr, ACOID::DefinedOIDidx dlg_ac_idx,
                                UCHAR_T rmt_ssn, const char* rmt_addr, UCHAR_T fake_ssn/* = 0*/)
{
    TonNpiAddress   onpi, rnpi;
    if (!onpi.fromText(own_addr) || !onpi.fixISDN())
        return NULL;
    if (!rnpi.fromText(own_addr) || !rnpi.fixISDN())
        return NULL;
    return newSRsession(onpi, dlg_ac_idx, rmt_ssn, rnpi, fake_ssn);
}

TCSessionSR * SSNSession::newSRsession(const TonNpiAddress & onpi, ACOID::DefinedOIDidx dlg_ac_idx,
                                UCHAR_T rmt_ssn, const TonNpiAddress & rnpi, UCHAR_T fake_ssn/* = 0*/)
{
    TCSessionSR *   pSess = NULL;

    if ((onpi.numPlanInd != NUMBERING_ISDN) || (onpi.typeOfNumber > ToN_INTERNATIONAL))
        return NULL;
    if ((rnpi.numPlanInd != NUMBERING_ISDN) || (rnpi.typeOfNumber > ToN_INTERNATIONAL))
        return NULL;

    {
        MutexGuard tmp(mutex);
        std::string sid;
        TCSessionAC::mkSignature(sid, _SSN, onpi, dlg_ac_idx, rmt_ssn, &rnpi);

        TCSessionsMAP::iterator it = tcSessions.find(sid);
        if (it != tcSessions.end())
            pSess = static_cast<TCSessionSR*>((*it).second);
        else {
            pSess = new TCSessionSR(++lastTCSUId, this, fake_ssn, onpi, dlg_ac_idx, rmt_ssn, rnpi);
            tcSessions.insert(TCSessionsMAP::value_type(pSess->Signature(), pSess));
        }
    }
    return pSess;
}


void SSNSession::closeTCSession(TCSessionAC * p_sess)
{
    MutexGuard tmp(mutex);
    TCSessionsMAP::iterator sit = tcSessions.find(p_sess->Signature());
    if (sit != tcSessions.end()) {
        tcSessions.erase(sit);
        deadSess.push_back(p_sess);
    } else
        smsc_log_error(logger, "SSN[%u]: TSess[%u] is not registered!",
                       (unsigned)_SSN, p_sess->getUID());
}


Dialog* SSNSession::findDialog(USHORT_T dId)
{
    MutexGuard tmp(mutex);
    return locateDialog(dId);
}

void SSNSession::releaseDialog(USHORT_T dId)
{
    MutexGuard tmp(mutex);
    DialogsMAP::iterator it = dialogs.find(dId);
    if (it == dialogs.end())
        return;

    Dialog* pDlg = (*it).second;
    dialogs.erase(it);
    if (pDlg) {
        unsigned invNum = 0;
        if (!pDlg->isFinished(&invNum)) {
            DlgTime     dtm;
            gettimeofday(&dtm.tms, 0);
            dtm.dlg = pDlg;
            pending.insert(DlgTimesMAP::value_type(dId, dtm));
            if (invNum)
                smsc_log_warn(logger,
                    "SSN[%u]: Put aside Dialog[0x%X](0x%x), %u invokes pending",
                    (unsigned)_SSN, dId, pDlg->getState().value, invNum);
            else
                smsc_log_debug(logger, "SSN[%u]: Put aside Dialog[0x%X](0x%x)",
                    (unsigned)_SSN, dId, pDlg->getState().value);
        } else
            dischargeDlg(pDlg);
    }
    return;
}

void SSNSession::releaseDialog(Dialog* pDlg, const TCSessionSUID * tc_suid/* = 0*/)
{
    if (!pDlg)
        return;
    MutexGuard tmp(mutex);
    USHORT_T dId = pDlg->getId();
    DialogsMAP::iterator it = dialogs.find(dId);
    if ((it == dialogs.end()) || (pDlg != (*it).second)) {
        smsc_log_error(logger, "SSN[%u]: Unregistered/illegal Dialog[0x%X]",
                       (unsigned)_SSN, dId);
        return;
    }
    dialogs.erase(it);
    unsigned invNum = 0;
    if (!pDlg->isFinished(&invNum)) {
        DlgTime     dtm;
        gettimeofday(&dtm.tms, 0);
        dtm.dlg = pDlg;
        pending.insert(DlgTimesMAP::value_type(dId, dtm));
        if (invNum)
            smsc_log_warn(logger,
                "SSN[%u]: Put aside Dialog[0x%X](0x%x), %u invokes pending",
                (unsigned)_SSN, dId, pDlg->getState().value, invNum);
        else
            smsc_log_debug(logger, "SSN[%u]: Put aside Dialog[0x%X](0x%x)",
                (unsigned)_SSN, dId, pDlg->getState().value);
    } else {
        dischargeDlg(pDlg, tc_suid);
    }
    return;
}

void SSNSession::releaseDialogs(const TCSessionSUID * tc_suid/* = 0*/)
{
    MutexGuard tmp(mutex);
    if (!tc_suid)
        smsc_log_debug(logger, "SSN[%u]: Releasing %u dialogs ..", (unsigned)_SSN, dialogs.size());
    else
        smsc_log_debug(logger, "SSN[%u]: Releasing %s dialogs ..", (unsigned)_SSN, tc_suid->c_str());

    for (DialogsMAP::iterator it = dialogs.begin(); (it != dialogs.end()) && (*it).second; it++) {
        Dialog* pDlg = (*it).second;
        USHORT_T dId = (*it).first;

        unsigned invNum = 0;
        if (!pDlg->isFinished(&invNum)) {
            DlgTime     dtm;
            gettimeofday(&dtm.tms, 0);
            dtm.dlg = pDlg;
            pending.insert(DlgTimesMAP::value_type(dId, dtm));
            if (invNum)
                smsc_log_warn(logger,
                    "SSN[%u]: Put aside Dialog[0x%X](0x%x), %u invokes pending",
                    (unsigned)_SSN, dId, pDlg->getState().value, invNum);
            else
                smsc_log_debug(logger, "SSN[%u]: Put aside Dialog[0x%X](0x%x)",
                    (unsigned)_SSN, dId, pDlg->getState().value);
        } else {
            dischargeDlg(pDlg, tc_suid);
        }
    }
    dialogs.clear();
}

bool SSNSession::getDialogId(USHORT_T & dId)
{
    MutexGuard tmp(mutex);
    cleanUpDialogs();

    if (state != ssnBound) {
        smsc_log_error(logger, "SSN[%u]: not bounded!", (unsigned)_SSN);
        return false;
    }
    if (!nextDialogId(dId)) {
        smsc_log_fatal(logger, "SSN[%u]: Dialogs exhausted [%u of %u], active(%u), pending(%u)",
            (unsigned)_SSN, dialogs.size() + pending.size(),
            maxId - minId, dialogs.size(), pending.size());
        if (logger->isDebugEnabled())
            dumpDialogs();
        return false;
    }
    return true;
}

void SSNSession::markDialog(Dialog * p_dlg)
{
    MutexGuard tmp(mutex);
    dialogs.insert(DialogsMAP::value_type(p_dlg->getId(), p_dlg));
}

void SSNSession::noticeInd(USHORT_T dlg_id, USHORT_T rel_id, UCHAR_T reportCause)
{
    MutexGuard tmp(mutex);
    TNoticeParms    parms(rel_id, reportCause);
    ntcdDlgs.insert(NoticedDLGs::value_type(dlg_id, parms));
    DialogsMAP::iterator it = dialogs.find(dlg_id);
    if (it != dialogs.end())
        smsc_log_error(logger, "SSN[%u]: SS7 assigned active dlgId(0x%x) to Notice Dialog!", dlg_id);
    else //reserve dlg_id
        dialogs.insert(DialogsMAP::value_type(dlg_id, NULL));
}

bool SSNSession::noticeParms(USHORT_T dlg_id, TNoticeParms & parms)
{
    MutexGuard tmp(mutex);
    NoticedDLGs::iterator it = ntcdDlgs.find(dlg_id);
    if (it != ntcdDlgs.end()) {
        parms = (*it).second;
        ntcdDlgs.erase(it);
        return true;
    }
    return false;
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * NOTE: these methods are not the processing graph entries, so never lock Mutex,
 *       it's a caller responsibility to lock it !!!
 * ---------------------------------------------------------------------------------- */
void SSNSession::dischargeDlg(Dialog * pDlg, const TCSessionSUID * tc_suid/* = 0*/)
{
    if (tc_suid && (*tc_suid == pDlg->getSUId())) {
        TCSessionsMAP::iterator sit = tcSessions.find(*tc_suid);
        if (sit != tcSessions.end()) {
            TCSessionAC * sess = (*sit).second;
            smsc_log_debug(logger, "SSN[%u]: Discharged terminated Dialog[0x%X], SUId: %s",
                            (unsigned)_SSN, pDlg->getId(), tc_suid->c_str());
            sess->toPool(pDlg);
            return;
        }
    }
    smsc_log_debug(logger, "SSN[%u]: Released terminated Dialog[0x%X]",
                       (unsigned)_SSN, pDlg->getId());
    delete pDlg;
    return;
}

void SSNSession::cleanUpDialogs(void)
{
    DlgTimesMAP::iterator it  = pending.begin();
    while (it != pending.end()) {
        DlgTime dtm = (*it).second;
        DlgTimesMAP::iterator curr = it++;
        if (dtm.dlg->isFinished()) {
            pending.erase(curr);
            dischargeDlg(dtm.dlg, &(dtm.dlg->getSUId()));
        }
    }
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


Dialog* SSNSession::locateDialog(USHORT_T dId)
{
    Dialog* pDlg = NULL;
    DialogsMAP::const_iterator it = dialogs.find(dId);
    if (it == dialogs.end()) {
        DlgTimesMAP::iterator tit = pending.find(dId);
        pDlg = (tit == pending.end()) ? NULL : ((*tit).second).dlg;
    } else
        pDlg = (*it).second;
    return pDlg;
}

void SSNSession::dumpDialogs(void)
{
    std::string dump;
    format(dump, "SSN[%u]: Dialogs [%u of %u], ", (unsigned)_SSN,
           dialogs.size() + pending.size(), maxId - minId);

    if (pending.size()) {
        format(dump, "pending(%u): ", pending.size());
        struct timeval ctm;
        gettimeofday(&ctm, 0);

        for (DlgTimesMAP::const_iterator it = pending.begin(); it != pending.end(); it++) {
            DlgTime dtm = (*it).second;
            long tdif = ctm - dtm.tms;
            format(dump, "%u(0x%x):%lus ", (*it).first, dtm.dlg->getState().value, tdif);
        }
    }
    format(dump, "active(%u): ", dialogs.size());
    for (DialogsMAP::const_iterator it = dialogs.begin(); (it != dialogs.end()) && (*it).second; it++) {
        Dialog* pDlg = (*it).second;
        format(dump, "%u(0x%x) ", (*it).first, pDlg->getState().value);
    }
    smsc_log_debug(logger, dump.c_str());
}

} // namespace inap
} // namespace inman
} // namespace smsc

