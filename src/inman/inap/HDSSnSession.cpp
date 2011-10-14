#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/vformat.hpp"
using smsc::util::format;

#include "inman/common/TimeOps.hpp"
//#include "inman/common/adrutil.hpp"
//using smsc::cvtutil::packSCCPAddress;
//using smsc::cvtutil::unpackSCCP2SSN_GT;

#include "inman/inap/HDSSnSession.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/inap/dispatcher.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

//compute masks for TCDlgState in order to avoid care on endianness.
TCDlgStateMASK   _TCDlgStateMasks;

/* ************************************************************************** *
 * class TCSessionAC implementation:
 * ************************************************************************** */
TCSessionAC::TCSessionAC(uint16_t uid, SSNSession * owner, uint8_t fake_ssn,
                const TonNpiAddress & own_addr, const ROSComponentsFactory * use_fact)
    : _owner(owner), tcUID(uid), ownAdr(own_addr), acFab(use_fact)
{
    ownAdr.fixISDN();
    senderSsn = _owner->getSSN();
    packSCCPAddress(locAddr, ownAdr.getSignals(), fake_ssn ? fake_ssn : senderSsn);
}

TCSessionAC::~TCSessionAC(void)
{
    MutexGuard  tmp(dlgGrd);
    if (pool.size()) {
        TCDialogsLIST::iterator it = pool.begin();
        for (; it != pool.end(); ++it)
            delete (*it);
        pool.clear();
    }
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

Dialog * TCSessionAC::initDialog(const SCCP_ADDRESS_OCTS & rmt_addr,
                                 Logger * use_log/* = NULL*/)
{
    TCDialogID dId;
    if (!_owner->getDialogId(dId))
        return NULL;
    Dialog * pDlg = NULL;
    {
        MutexGuard  tmp(dlgGrd);
        if (!pool.empty()) {
            pDlg = *(pool.begin());
            pool.pop_front();
        } else
            pDlg = new Dialog(sign, dId, _owner->getMsgUserId(),
                              acFab, locAddr, senderSsn, use_log);
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
 * class TCSessionMA implementation:
 * ************************************************************************** */
TCSessionMA::TCSessionMA(uint16_t uid, SSNSession * owner, uint8_t fake_ssn,
                const TonNpiAddress & own_addr, const ROSComponentsFactory * use_fact,
                uint8_t rmt_ssn)
    : TCSessionAC(uid, owner, fake_ssn, own_addr, use_fact)
    , rmtSSN(rmt_ssn)
{
    sign = owner->mkSignature(own_addr, use_fact->acOID(), rmt_ssn, NULL);
}

Dialog* TCSessionMA::openDialog(const char* rmt_addr, Logger * use_log/* = NULL*/)
{
    TonNpiAddress   rnpi;
    if (!rnpi.fromText(rmt_addr) || !rnpi.fixISDN())
        return NULL;

    return openDialog(rnpi, use_log);
}

Dialog* TCSessionMA::openDialog(const TonNpiAddress & rnpi, Logger * use_log/* = NULL*/)
{
    if ((rnpi.numPlanInd != TonNpiAddress::npiISDNTele_e164)
        || (rnpi.typeOfNumber > TonNpiAddress::tonInternational))
        return NULL; //todo: throw

    SCCP_ADDRESS_OCTS  rmtAddr;
    packSCCPAddress(rmtAddr, rnpi.getSignals(), rmtSSN);
    return initDialog(rmtAddr, use_log);
}

/* ************************************************************************** *
 * class TCSessionSR implementation:
 * ************************************************************************** */
TCSessionSR::TCSessionSR(uint16_t uid, SSNSession * owner, uint8_t fake_ssn,
                const TonNpiAddress & own_addr, const ROSComponentsFactory * use_fact,
                uint8_t rmt_ssn, const TonNpiAddress & rmt_addr)
    : TCSessionAC(uid, owner, fake_ssn, own_addr, use_fact)
    , rmtNpi(rmt_addr)
{
    sign = owner->mkSignature(own_addr, use_fact->acOID(), rmt_ssn, &rmtNpi);
    packSCCPAddress(rmtAddr, rmtNpi.getSignals(), rmt_ssn);
}

Dialog* TCSessionSR::openDialog(Logger * use_log/* = NULL*/)
{
    if (!rmtNpi.fixISDN())
        return NULL;
    return initDialog(rmtAddr, use_log);
}

/* ************************************************************************** *
 * class SSNSession implementation (TCAP dialogs/sessions factory):
 * ************************************************************************** */
SSNSession::SSNSession(const ApplicationContextRegistryITF * use_acReg, uint8_t ssn_id,
                       uint16_t user_id, const SS7UnitInstsMap & tcap_inst_ids,
                       uint16_t max_dlg_id/* = 2000*/, Logger * uselog/* = NULL*/)
    : _stopping(false), _SSN(ssn_id), msgUserId(user_id)
    , _cfg(tcap_inst_ids, max_dlg_id), lastTCSUId(0)
    , acReg(use_acReg), logger(uselog)
{
    if (!uselog)
        logger = Logger::getInstance("smsc.inman.inap.SSN");
}

SSNSession::~SSNSession()
{
    MutexGuard tmp(_sync);
    _stopping = true;
    cleanUpDialogs(); //adjusts tcSessions pools
    if (!tcSessions.empty()) { //kill TC sessions
        for (TCSessionsMAP::iterator sit = tcSessions.begin();
                                    sit != tcSessions.end(); ++sit)
            delete sit->second;
    }
    if (!deadSess.empty()) {
        for (TCSessionsLIST::iterator sit = deadSess.begin();
                                    sit != deadSess.end(); ++sit)
            delete (*sit);
    }

    if (dialogs.empty() && pending.empty())
        return;

    smsc_log_debug(logger, "SSN[%u]: Closing %u dialogs ..",
                    (unsigned)_SSN, dialogs.size() + pending.size());
    if (!dialogs.empty()) {
        for (DialogsMAP::iterator it = dialogs.begin();
                                (it != dialogs.end()) && it->second; ++it) {
            Dialog* pDlg = it->second;
            unsigned invNum = 0;
            if (!pDlg->isFinished(&invNum)) {
                smsc_log_warn(logger,
                    "SSN[%u]: %s is active, %u invokes pending, state {%s}",
                    (unsigned)_SSN, pDlg->idStr(), invNum, pDlg->getState().Print().c_str());
            }
            delete it->second;
        }
        dialogs.clear();
    }
    if (!pending.empty()) {
        for (DlgTimesMAP::iterator it = pending.begin(); it != pending.end(); ++it) {
            DlgTime dtm  = it->second;
            smsc_log_warn(logger, "SSN[%u]: %s is pending, %u invokes pending, state {%s}",
                          (unsigned)_SSN, dtm.dlg->idStr(), dtm.dlg->pendingInvokes(),
                          dtm.dlg->getState().Print().c_str());
            delete dtm.dlg;
        }
        pending.clear();
    }
}

std::string SSNSession::mkSignature(const TonNpiAddress & onpi, const EncodedOID & dlg_ac,
                            uint8_t rmt_ssn, const TonNpiAddress * rnpi) const
{
    char rssn[8]; // sizeof("255")
    if (rmt_ssn)
        snprintf(rssn, sizeof(rssn), "%u", rmt_ssn);
    else {
        rssn[0] = '*'; rssn[1] = 0;
    }
    return format("%u:%s{%s}->%s:%s", _SSN, onpi.getSignals(), dlg_ac.nick().c_str(), rssn,
           rnpi ? rnpi->getSignals() : "*");
}

TCSessionMA * SSNSession::newMAsession(const char* own_addr,
                                       const EncodedOID & dlg_ac,
                                       uint8_t rmt_ssn, uint8_t fake_ssn/* = 0*/)
{
    TonNpiAddress   onpi;
    if (!onpi.fromText(own_addr) || !onpi.fixISDN())
        return NULL;
    return newMAsession(onpi, dlg_ac, rmt_ssn, fake_ssn);
}

TCSessionMA * SSNSession::newMAsession(const TonNpiAddress & onpi,
                                       const EncodedOID & dlg_ac,
                                       uint8_t rmt_ssn, uint8_t fake_ssn/* = 0*/)
{
    TCSessionMA *   pSess = NULL;

    if ((onpi.numPlanInd != TonNpiAddress::npiISDNTele_e164) || (onpi.typeOfNumber > TonNpiAddress::tonInternational))
        return NULL;
    const ROSComponentsFactory * acFab = acReg->getFactory(dlg_ac);
    if (acFab) {
//        onpi.typeOfNumber = TonNpiAddress::tonInternational; //correct isdn unknown
        MutexGuard tmp(_sync);
        std::string sid = mkSignature(onpi, acFab->acOID(), rmt_ssn, NULL);
        TCSessionsMAP::iterator it = tcSessions.find(sid);
        if (it != tcSessions.end())
            pSess = static_cast<TCSessionMA*>(it->second);
        else {
            pSess = new TCSessionMA(++lastTCSUId, this, fake_ssn, onpi, acFab, rmt_ssn);
            tcSessions.insert(TCSessionsMAP::value_type(pSess->Signature(), pSess));
        }
    }
    return pSess;
}


TCSessionSR * 
    SSNSession::newSRsession(const char* own_addr,
                             const EncodedOID & dlg_ac, uint8_t rmt_ssn,
                             const char* rmt_addr, uint8_t fake_ssn/* = 0*/)
{
    TonNpiAddress   onpi, rnpi;
    if (!onpi.fromText(own_addr) || !onpi.fixISDN())
        return NULL;
    if (!rnpi.fromText(own_addr) || !rnpi.fixISDN())
        return NULL;
    return newSRsession(onpi, dlg_ac, rmt_ssn, rnpi, fake_ssn);
}

TCSessionSR * 
    SSNSession::newSRsession(const TonNpiAddress & onpi,
                             const EncodedOID & dlg_ac, uint8_t rmt_ssn,
                             const TonNpiAddress & rnpi, uint8_t fake_ssn/* = 0*/)
{
    TCSessionSR *   pSess = NULL;

    if ((onpi.numPlanInd != TonNpiAddress::npiISDNTele_e164) || (onpi.typeOfNumber > TonNpiAddress::tonInternational))
        return NULL;
    if ((rnpi.numPlanInd != TonNpiAddress::npiISDNTele_e164) || (rnpi.typeOfNumber > TonNpiAddress::tonInternational))
        return NULL;

    const ROSComponentsFactory * acFab = acReg->getFactory(dlg_ac);
    if (acFab) {
        MutexGuard tmp(_sync);
        std::string sid = mkSignature(onpi, acFab->acOID(), rmt_ssn, &rnpi);
        TCSessionsMAP::iterator it = tcSessions.find(sid);
        if (it != tcSessions.end())
            pSess = static_cast<TCSessionSR*>((*it).second);
        else {
            pSess = new TCSessionSR(++lastTCSUId, this, fake_ssn, onpi, acFab, rmt_ssn, rnpi);
            tcSessions.insert(TCSessionsMAP::value_type(pSess->Signature(), pSess));
        }
    }
    return pSess;
}


void SSNSession::closeTCSession(TCSessionAC * p_sess)
{
    MutexGuard tmp(_sync);
    TCSessionsMAP::iterator sit = tcSessions.find(p_sess->Signature());
    if (sit != tcSessions.end()) {
        tcSessions.erase(sit);
        deadSess.push_back(p_sess);
    } else
        smsc_log_error(logger, "SSN[%u]: TCSession[%u] is not registered!",
                       (unsigned)_SSN, p_sess->getUID());
}


Dialog* SSNSession::findDialog(const TCDialogID & dId) const
{
    MutexGuard tmp(_sync);
    return locateDialog(dId);
}

void SSNSession::releaseDialog(const TCDialogID & dId)
{
    MutexGuard tmp(_sync);
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
                    "SSN[%u]: Put aside %s, %u invokes pending, state {%s}",
                    (unsigned)_SSN, pDlg->idStr(), invNum, pDlg->getState().Print().c_str());
            else
                smsc_log_debug(logger, "SSN[%u]: Put aside %s, state {%s}",
                    (unsigned)_SSN, pDlg->idStr(), pDlg->getState().Print().c_str());
        } else
            dischargeDlg(pDlg);
    }
    return;
}

void SSNSession::releaseDialog(Dialog* pDlg, const TCSessionSUID * tc_suid/* = 0*/)
{
    if (!pDlg)
        return;
    MutexGuard tmp(_sync);
    TCDialogID dId = pDlg->getId();
    DialogsMAP::iterator it = dialogs.find(dId);
    if ((it == dialogs.end()) || (pDlg != (*it).second)) {
        smsc_log_error(logger, "SSN[%u]: Unregistered/illegal %s",
                       (unsigned)_SSN, pDlg->idStr());
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
                "SSN[%u]: Put aside %s, %u invokes pending, state {%s}",
                (unsigned)_SSN, pDlg->idStr(), invNum, pDlg->getState().Print().c_str());
        else
            smsc_log_debug(logger, "SSN[%u]: Put aside %s, state {%s}",
                (unsigned)_SSN, pDlg->idStr(), pDlg->getState().Print().c_str());
    } else {
        dischargeDlg(pDlg, tc_suid);
    }
    return;
}

void SSNSession::releaseDialogs(const TCSessionSUID * tc_suid/* = 0*/)
{
    MutexGuard tmp(_sync);
    if (!tc_suid)
        smsc_log_debug(logger, "SSN[%u]: Releasing %u dialogs ..", (unsigned)_SSN, dialogs.size());
    else
        smsc_log_debug(logger, "SSN[%u]: Releasing %s dialogs ..", (unsigned)_SSN, tc_suid->c_str());

    for (DialogsMAP::iterator it = dialogs.begin();
                                (it != dialogs.end()) && it->second; ++it) {
        Dialog* pDlg = it->second;
        TCDialogID dId = it->first;

        unsigned invNum = 0;
        if (!pDlg->isFinished(&invNum)) {
            DlgTime     dtm;
            gettimeofday(&dtm.tms, 0);
            dtm.dlg = pDlg;
            pending.insert(DlgTimesMAP::value_type(dId, dtm));
            if (invNum)
                smsc_log_warn(logger,
                    "SSN[%u]: Put aside %s, %u invokes pending, state {%s}",
                    (unsigned)_SSN, pDlg->idStr(), invNum, pDlg->getState().Print().c_str());
            else
                smsc_log_debug(logger, "SSN[%u]: Put aside %s, state {%s}",
                    (unsigned)_SSN, pDlg->idStr(), pDlg->getState().Print().c_str());
        } else {
            dischargeDlg(pDlg, tc_suid);
        }
    }
    dialogs.clear();
}

bool SSNSession::getDialogId(TCDialogID & dId)
{
    MutexGuard tmp(_sync);
    if (_stopping)
        return false;
    cleanUpDialogs();

    if (_cfg.getStatus() < SSNBinding::ssnPartiallyBound) {
        smsc_log_error(logger, "SSN[%u]: not bounded!", (unsigned)_SSN);
        return false;
    }
    if (!nextDialogId(dId)) {
        smsc_log_fatal(logger, "SSN[%u]: Dialogs exhausted, active(%u), pending(%u)",
            (unsigned)_SSN, dialogs.size() + pending.size(), dialogs.size(), pending.size());
        if (logger->isDebugEnabled())
            dumpDialogs();
        return false;
    }
    return true;
}

void SSNSession::markDialog(Dialog * p_dlg)
{
    MutexGuard tmp(_sync);
    dialogs.insert(DialogsMAP::value_type(p_dlg->getId(), p_dlg));
}

void SSNSession::noticeInd(const TCDialogID & dlg_id, const TCDialogID & rel_id, uint8_t reportCause)
{
    MutexGuard tmp(_sync);
    TNoticeParms    parms(rel_id, reportCause);
    ntcdDlgs.insert(NoticedDLGs::value_type(dlg_id, parms));
    DialogsMAP::iterator it = dialogs.find(dlg_id);
    if (it != dialogs.end())
        smsc_log_error(logger, "SSN[%u]: SS7 assigned active Dialog[%u:%Xh) to Notice Dialog!",
                       dlg_id.tcInstId, dlg_id.dlgId);
    else //reserve dlg_id
        dialogs.insert(DialogsMAP::value_type(dlg_id, NULL));
}

bool SSNSession::noticeParms(const TCDialogID & dlg_id, TNoticeParms & parms)
{
    MutexGuard tmp(_sync);
    NoticedDLGs::iterator it = ntcdDlgs.find(dlg_id);
    if (it != ntcdDlgs.end()) {
        parms = it->second;
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
    UNITBinding * ubnd = _cfg.getUnit(pDlg->getId().tcInstId);
    if (ubnd)
        --(ubnd->_numOfDlgs);

    if (tc_suid && (*tc_suid == pDlg->getSUId())) {
        TCSessionsMAP::iterator sit = tcSessions.find(*tc_suid);
        if (sit != tcSessions.end()) {
            TCSessionAC * sess = sit->second;
            smsc_log_debug(logger, "SSN[%u]: Discharged terminated %s, SUId: %s",
                            (unsigned)_SSN, pDlg->idStr(), tc_suid->c_str());
            sess->toPool(pDlg);
            return;
        }
    }
    smsc_log_debug(logger, "SSN[%u]: Released terminated %s",
                       (unsigned)_SSN, pDlg->idStr());
    delete pDlg;
    return;
}

void SSNSession::cleanUpDialogs(void)
{
    DlgTimesMAP::iterator it  = pending.begin();
    while (it != pending.end()) {
        DlgTime dtm = it->second;
        DlgTimesMAP::iterator curr = it++;
        if (dtm.dlg->isFinished()) {
            pending.erase(curr);
            dischargeDlg(dtm.dlg, &(dtm.dlg->getSUId()));
        }
    }
}

bool SSNSession::nextDialogId(TCDialogID & dId)
{
    UNITBinding * ubnd = _cfg.getUnitForDialog();
    if (!ubnd)
        return false;

    uint16_t attempt = 0, maxIdAtt = (ubnd->_maxId - ubnd->_numOfDlgs);
    dId.tcInstId = ubnd->getUnit()._instId;
    do {
        if ((++(ubnd->_lastDlgId) > ubnd->_maxId) || !ubnd->_lastDlgId)
            ubnd->_lastDlgId = 1; //minId
        dId.dlgId = ubnd->_lastDlgId;
    } while (locateDialog(dId) && ((++attempt) < maxIdAtt));

    if (attempt < maxIdAtt) { //dlgId is allocated
        ++(ubnd->_numOfDlgs);
        return true;
    }
    return false;
}


Dialog* SSNSession::locateDialog(const TCDialogID & dId) const
{
    Dialog* pDlg = NULL;
    DialogsMAP::const_iterator it = dialogs.find(dId);
    if (it == dialogs.end()) {
        DlgTimesMAP::const_iterator tit = pending.find(dId);
        pDlg = (tit == pending.end()) ? NULL : tit->second.dlg;
    } else
        pDlg = it->second;
    return pDlg;
}

void SSNSession::dumpDialogs(void) const
{
    std::string dump;
    format(dump, "SSN[%u]: Dialogs [%u of %u], ", (unsigned)_SSN,
           (unsigned)(dialogs.size() + pending.size()), _cfg.maxDlgNum());

    if (!pending.empty()) {
        format(dump, "pending(%u): ", (unsigned)pending.size());
        struct timeval ctm;
        gettimeofday(&ctm, 0);

        for (DlgTimesMAP::const_iterator it = pending.begin(); it != pending.end(); ++it) {
            TCDialogID   dId = it->first;
            DlgTime     dtm = it->second;
            long tdif = ctm - dtm.tms;
            format(dump, "[%u:%Xh]:%lus ", (unsigned)dId.dlgId, (unsigned)dId.tcInstId,
                   dtm.dlg->getState().value.mask, tdif);
        }
    }
    format(dump, "active(%u): ", dialogs.size());
    for (DialogsMAP::const_iterator it = dialogs.begin();
                                (it != dialogs.end()) && it->second; ++it) {
        TCDialogID   dId = it->first;
        Dialog *    pDlg = it->second;
        format(dump, "[%u:%Xh] ", (unsigned)dId.dlgId, (unsigned)dId.tcInstId,
               pDlg->getState().value.mask);
    }
    smsc_log_debug(logger, dump.c_str());
}

} // namespace inap
} // namespace inman
} // namespace smsc

