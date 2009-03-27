#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/inap/dispatcher.hpp"
#include "inman/inap/ACRegistry.hpp"
#include "inman/inap/TCAPErrors.hpp"
using smsc::inman::inap::rc2Txt_TC_BindResult;
using smsc::inman::inap::TC_BindResult;

#include "inman/inap/TCCallbacks.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

/* ************************************************************************** *
 * class TCAPDispatcher implementation:
 * ************************************************************************** */
#define MAX_BIND_SECS   5       //maximum timeout on SSN bind request, units: secs
#define RECV_TIMEOUT    300     //message receiving timeout, units: millisecs
#define RECONNECT_TIMEOUT 3000  //SS7 stack reconnection timeout, units: millisecs

#define MAX_BIND_ATTEMPTS (MAX_BIND_SECS*1000/RECV_TIMEOUT)
#define MAX_UCONN_ATTEMPTS MAX_BIND_ATTEMPTS

TCAPDispatcher::TCAPDispatcher()
    : state(ss7None), _listening(false), logger(NULL), _logId("TCAPDsp")
{
    ApplicationContextRegistry::get();
    TCCbkLink::get(); //Link TC API Callbacks
#ifdef EIN_HD
    EINSS7CpMain_CpInit();
#endif /* EIN_HD */
}

TCAPDispatcher::~TCAPDispatcher()
{
    Stop(true);
    MutexGuard grd(_sync);
    disconnectCP(ss7None);
    TCCbkLink::get().DeInit();
    smsc_log_info(logger, "%s: SS7 stack disconnected", _logId);
}

ApplicationContextRegistryITF * TCAPDispatcher::acRegistry(void) const
{
    return &ApplicationContextRegistry::get();
}

    
//Initializes TCAPDispatcher and SS7 communication facility
//Returns true on success
bool TCAPDispatcher::Init(TCDsp_CFG & use_cfg, Logger * use_log/* = NULL*/)
{
    MutexGuard grd(_sync);
    _cfg = use_cfg;
    _cfg.mpUserId += USER01_ID - 1; //adjust USER_ID to PortSS7 units id
#if EIN_HD >= 101
    //check for TCAP BE instances ids
    SS7UnitsCFG::const_iterator cit = _cfg.ss7Units.find(TCAP_ID);
    if ((cit == _cfg.ss7Units.end()) || cit->second.instIds.empty())
        return false;
#else /* EIN_HD >= 101 */
    //set the single TCAP BE instance with id = 0
    SS7Unit_CFG tcUnit(TCAP_ID);
    tcUnit.instIds.insert(SS7UnitInstsMap::value_type(0, SS7UnitInstance(0)));
    _cfg.ss7Units[TCAP_ID] = tcUnit;
#endif /* EIN_HD >= 101 */

    logger = use_log ? use_log : Logger::getInstance("smsc.inman.inap");
    if (TCCbkLink::get().tcapDisp())
        return false;
    if (connectCP(TCAPDispatcherITF::ss7INITED) < 0)
        return false;
    TCCbkLink::get().Init(this, logger);
    return true;
}

//Returns true on successfull connection to TCAP unit of SS7 stack,
//starts TCAP messages(indications) listener
bool TCAPDispatcher::Start(void)
{
    if (isListening())
        return true;
    if (connectCP(ss7OPENED) < 0)
        return false;
    smsc_log_debug(logger, "%s: starting ..", _logId);
    Thread::Start();
    {
        MutexGuard grd(_sync);
        if (_listening)
            return true;
        _sync.wait(RECV_TIMEOUT);
        if (!_listening) {
            smsc_log_fatal(logger, "%s: unable to start", _logId);
            return false;
        }
    }
    return true;
}

//Stops TCAP messages(indications) listener, unbinds all SSNs,
//if do_wait is set sets ss7INITED state
void TCAPDispatcher::Stop(bool do_wait/* = false*/)
{
    {
        MutexGuard grd(_sync);
        if (_listening) {
            _listening = false;
            smsc_log_debug(logger, "%s: stopping MsgListener ..", _logId);
            _sync.notify();
        }
    }
    if (do_wait) {
        Thread::WaitFor();
        MutexGuard grd(_sync);
        if (!sessions.empty()) {
            do {
                SSNSession* pSession = sessions.begin()->second;
                _sync.Unlock();
                delete pSession;
                _sync.Lock();
                sessions.erase(sessions.begin());
            } while (!sessions.empty());
        }
        disconnectCP(ss7INITED);
    }
}

// -------------------------------------------------------------------------
// TCAP messages(indications) listener methods
// -------------------------------------------------------------------------
void TCAPDispatcher::onDisconnect(void)
{
    MutexGuard grd(_sync);
    disconnectCP(ss7INITED);
}

int TCAPDispatcher::Listen(void)
{
    unsigned bindTimer = 0;
    unsigned connTimer = 0;
    USHORT_T result = 0;

    _sync.Lock(); //waits here for connect()
    _listening = true;
    _sync.notify();
    while (_listening) {
        if (state == ss7CONNECTED) {
            if (connTimer >= MAX_UCONN_ATTEMPTS) {
            //there is at least one disconnected unit instance, try to connect it
                if (connectUnits())
                    bindSSNs(); //rebind SSNs to newly available instance
                connTimer = 0;
            }
        } else {
            if (connectCP(ss7CONNECTED) >= 0) {
                bindSSNs();
                bindTimer = connTimer = 0;
            } else {
                _sync.wait(RECONNECT_TIMEOUT);
                continue;
            }
        }
        _sync.Unlock();
        MSG_T msg;
        memset(&msg, 0, sizeof( MSG_T ));
        msg.receiver = _cfg.mpUserId;

        result = MsgRecvEvent(&msg, NULL, NULL, RECV_TIMEOUT);
        if (MSG_TIMEOUT != result) {
            if (!result) {
                if (msg.sender == TCAP_ID)
                    EINSS7_I97THandleInd(&msg); //calls callbacks.cpp::*()
                else
                    smsc_log_warn(logger, "%s: ignoring msg[%u] received from unit[%u], ",
                                    _logId, (unsigned)msg.primitive, (unsigned)msg.sender);
            }
            EINSS7CpReleaseMsgBuffer(&msg);
            if (result) {
                smsc_log_error(logger, "%s: MsgRecv() failed with code %d (%s)", _logId,
                               result, rc2Txt_SS7_CP(result));
                if ((MSG_BROKEN_CONNECTION == result) || (MSG_NOT_CONNECTED == result))
                    onDisconnect();
            }
        } else { //check for SSN bind state
            MutexGuard grd(_sync);
            if (disconnectedUnits())
                ++connTimer;
            if (unbindedSSNs())
                ++bindTimer;
            if (bindTimer >= MAX_BIND_ATTEMPTS) {
                smsc_log_error(logger, "%s: not all SSNs binded, reconnecting", _logId);
                disconnectCP(ss7INITED);
            }
        }
        _sync.Lock();
    }
    if (MSG_TIMEOUT == result)
        result = 0;
    unbindSSNs(); //unbind all SSNs
    disconnectCP(ss7OPENED);
    _sync.Unlock();
    return (int)result;
}

// Listener thread entry point
int TCAPDispatcher::Execute(void)
{
    int result = 0;
    try {
        smsc_log_info(logger, "%s: MsgListener thread started", _logId);
        result = Listen();
    } catch (const std::exception& exc) {
        smsc_log_fatal(logger, "%s: MsgListener thread got an exception: %s", _logId, exc.what() );
        result = -2;
    }
    smsc_log_debug(logger, "%s: MsgListener thread finished, reason %d", _logId, result);
    return result;
}

/* -------------------------------------------------------------------------- *
 * PRIVATE:
 * -------------------------------------------------------------------------- */
//Checks for unconnected TCAP BE instances and returns its total number
unsigned TCAPDispatcher::disconnectedUnits(void) const
{
    unsigned rval = 0;
    SS7UnitsCFG::const_iterator cit = _cfg.ss7Units.find(TCAP_ID);
    const SS7Unit_CFG & unitCfg = cit->second;

    for (SS7UnitInstsMap::const_iterator it = unitCfg.instIds.begin();
                                    it != unitCfg.instIds.end(); ++it) {
        if (it->second.connStatus != SS7UnitInstance::uconnOk)
            ++rval;
    }
    return rval;
}

//Connects currently disconnected TCAP BE instances.
//Returns true if new instances become available (connected)
bool TCAPDispatcher::connectUnits(void)
{
    unsigned disconnIni = disconnectedUnits();
    SS7UnitsCFG::iterator cit = _cfg.ss7Units.find(TCAP_ID);
    SS7Unit_CFG & unitCfg = cit->second;
    SS7UnitInstsMap::iterator it = unitCfg.instIds.begin();
#ifdef EIN_HD
    //set CONNECTED state if at least one instance is succesfully connected
    for (; (it != unitCfg.instIds.end())
            && (it->second.connStatus != SS7UnitInstance::uconnOk); ++it) {

        USHORT_T result = EINSS7CpMsgConnInst(_cfg.mpUserId, TCAP_ID, it->second.instId);
        if (result != 0) {
            smsc_log_error(logger, "%s: MsgConn(TCAP instId = %u) failed: %s (code %u)",
                        _logId, (unsigned)it->second.instId, rc2Txt_SS7_CP(result), result);
            it->second.connStatus = SS7UnitInstance::uconnError;
        } else {
            smsc_log_debug(logger, "%s: MsgConn(TCAP instId = %u) done",
                        _logId, (unsigned)it->second.instId);
            it->second.connStatus = SS7UnitInstance::uconnOk;
        }
    }
#else  /* EIN_HD */
    //single TCAP BE instance is used
    USHORT_T result = MsgConn(_cfg.mpUserId, TCAP_ID);
    if (result != 0) {
        smsc_log_fatal(logger, "%s: MsgConn(TCAP instId = %u) failed: %s (code %u)",
                       _logId, (unsigned)it->second.instId, rc2Txt_SS7_CP(result), result);
        it->second.connStatus = SS7UnitInstance::uconnError;
    }
    it->second.connStatus = SS7UnitInstance::uconnOk;
#endif /* EIN_HD */

    return (disconnectedUnits() < disconnIni);
}

//Disonnects all TCAP BE instances.
void TCAPDispatcher::disconnectUnits(void)
{
    SS7UnitsCFG::iterator cit = _cfg.ss7Units.find(TCAP_ID);
    SS7Unit_CFG & unitCfg = cit->second;
    SS7UnitInstsMap::iterator it = unitCfg.instIds.begin();
#ifdef EIN_HD
    for (; it != unitCfg.instIds.end(); ++it) {
        if (it->second.connStatus == SS7UnitInstance::uconnOk) {
            USHORT_T result = EINSS7CpMsgRelInst(_cfg.mpUserId, TCAP_ID, it->second.instId);
            if (result)
                smsc_log_error(logger, "%s: MsgRel(TCAP instId = %u) failed: %s (code %u)",
                            _logId, (unsigned)it->second.instId, rc2Txt_SS7_CP(result), result);
        }
        it->second.connStatus = SS7UnitInstance::uconnIdle;
    }
#else  /* EIN_HD */
    //single TCAP BE instance is used
    USHORT_T result = MsgRel(_cfg.mpUserId, TCAP_ID);
    if (result)
        smsc_log_error(logger, "%s: MsgRel(TCAP instId = %u) failed: %s (code %u)",
                    _logId, (unsigned)it->second.instId, rc2Txt_SS7_CP(result), result);
    it->second.connStatus = SS7UnitInstance::uconnIdle;
#endif /* EIN_HD */
    return;
}

unsigned TCAPDispatcher::unbindedSSNs(void) const
{
    unsigned rval = 0;
    for (SSNmap_T::const_iterator it = sessions.begin(); it != sessions.end(); ++it) {
        SSNSession* pSession = it->second;
        if (pSession->getState() < SSNBinding::ssnPartiallyBound)
            ++rval;
    }
    return rval;
}

//Returns true if at least one SSN BindReq() is successfull
//NOTE: actual BindReq() result is returned to confirmSSN()
bool TCAPDispatcher::bindSSN(SSNSession * p_session) const
{
    SS7UnitsCFG::const_iterator ucit = _cfg.ss7Units.find(TCAP_ID);
    const SS7Unit_CFG & unitCfg = ucit->second;


    UNITsStatus tcUnits;
    p_session->getUnitsStatus(tcUnits);

    bool rval = false;
    for (UNITsStatus::const_iterator cit = tcUnits.begin();
          cit != tcUnits.end() && (cit->bindStatus != UNITStatus::unitBound); ++cit) {
        const UNITStatus & unit = *cit;
        if (!unitCfg.instIds.isStatus(unit.instId, SS7UnitInstance::uconnOk))
            continue;
        if (unit.bindStatus == UNITStatus::unitError)
            p_session->ResetUnit(unit.instId);

        USHORT_T  result = EINSS7_I97TBindReq(p_session->getSSN(), _cfg.mpUserId,
                                              unit.instId, EINSS7_I97TCAP_WHITE_USER
#if EIN_HD >= 101
                                              , 0
#endif /* EIN_HD >= 101 */
                                              );
        if (result) {
            smsc_log_error(logger, "%s: BindReq(SSN=%u, userId=%u,"
                                   " tcInstId=%u) failed with code %u (%s)", _logId,
                            p_session->getSSN(), _cfg.mpUserId, (unsigned)unit.instId,
                            result, rc2Txt_TC_APIError(result));
             //no confirmation will come from EIN SS7, so rise signal!
            p_session->SignalUnit(unit.instId, UNITStatus::unitError);
        } else {
            smsc_log_debug(logger, "%s: BindReq(SSN=%u, userId=%u,"
                                   " tcInstId=%u) sent", _logId,
                           p_session->getSSN(), _cfg.mpUserId, (unsigned)unit.instId);
            rval = true;  //await confirmation
        }
    }
    return rval;
}

void TCAPDispatcher::bindSSNs(void) const
{
    for (SSNmap_T::const_iterator it = sessions.begin(); it != sessions.end(); ++it)
        bindSSN(it->second);
    return;
}

void TCAPDispatcher::unbindSSN(SSNSession * p_session) const
{
    UNITsStatus tcUnits;
    p_session->getUnitsStatus(tcUnits);

    for (UNITsStatus::const_iterator cit = tcUnits.begin();
                                        cit != tcUnits.end(); ++cit) {
        const UNITStatus & unit = *cit;
        if (unit.bindStatus == UNITStatus::unitBound) {
            USHORT_T  result = EINSS7_I97TUnBindReq(p_session->getSSN(),
                                                    _cfg.mpUserId, unit.instId);
            if (!result)
                smsc_log_debug(logger, "%s: UnBindReq(SSN=%u, userId=%u, tcInstId=%u)", _logId,
                               p_session->getSSN(), _cfg.mpUserId, (unsigned)unit.instId);
            else
                smsc_log_error(logger, "%s: UnBindReq(SSN=%u, userId=%u, tcInstId=%u)"
                                           " failed with code %u (%s)", _logId,
                                   p_session->getSSN(), _cfg.mpUserId, (unsigned)unit.instId,
                                   result, rc2Txt_TC_APIError(result));
        }
        p_session->ResetUnit(unit.instId);
    }
    return;
}

void TCAPDispatcher::unbindSSNs(void) const
{
    for (SSNmap_T::const_iterator it = sessions.begin(); it != sessions.end(); ++it)
        unbindSSN(it->second);
    return;
}


//Returns:  (-1) - failed to connect, 0 - already connected, 1 - successfully connected
int TCAPDispatcher::connectCP(SS7State_T upTo/* = ss7CONNECTED*/)
{
    if (state >= upTo)
        return 0;
    smsc_log_info(logger, "%s: connecting SS7 stack: %u -> %u ..", _logId, state, upTo);
    
    USHORT_T result;
    int    rval = 1;
    while ((rval > 0) && (state < upTo)) {
        switch (state) {
        case ss7None: {
#ifdef EIN_HD
            result = EINSS7CpRegisterMPOwner(_cfg.mpUserId);
            if (result != RETURN_OK) {
                smsc_log_fatal(logger, "%s: CpRegisterMPOwner(userId = %u)"
                                       " failed: %s (code %u)", _logId,
                                (unsigned)_cfg.mpUserId, rc2Txt_SS7_CP(result), result);
                rval = -1;
                break;
            }
            char * buf = new char[_cfg.rcpMgrAdr.length() + 2];
            strcpy(buf, _cfg.rcpMgrAdr.c_str());
            result = EINSS7CpRegisterRemoteCPMgmt(CP_MANAGER_ID, _cfg.rcpMgrInstId, buf);
            delete [] buf;
            if (result != RETURN_OK) {
                smsc_log_fatal(logger, "%s: CpRegisterRemoteCPMgmt() failed: %s (code %u)", _logId,
                                rc2Txt_SS7_CP(result), result);
                rval = -1;
                break;
            }
            state = ss7REGISTERED;
            smsc_log_info(logger, "%s: state REGISTERED (userId = %u, CpMgr: %s)", _logId,
                           (unsigned)_cfg.mpUserId, _cfg.rcpMgrAdr.c_str());
#else /* EIN_HD */
            state = ss7REGISTERED;
            smsc_log_info(logger, "%s: state REGISTERED (userId = %u)", _logId,
                            (unsigned)_cfg.mpUserId);
#endif /* EIN_HD */
        } break;

        case ss7REGISTERED: {
            //Initiation of the message handling and allocating memory,
            //reading cp.cnf, connecting to CP manager
            result = EINSS7CpMsgInitiate(_cfg.maxMsgNum, _cfg.appInstId, 0);
            if (result != 0) {
                smsc_log_fatal(logger, "%s: CpMsgInitiate(appInstanceId = %u)"
                                       " failed: %s (code %u)", _logId,
                               (unsigned)_cfg.appInstId, rc2Txt_SS7_CP(result), result);
                rval = -1;
            } else {
                state = ss7INITED;
                smsc_log_debug(logger, "%s: state INITED (appInstanceId = %u)",
                               _logId, (unsigned)_cfg.appInstId);
            }
        } break;

        case ss7INITED: {  //Opening of the input queue. //MsgOpen(userId)
            result = EINSS7CpMsgPortOpen(_cfg.mpUserId, TRUE);
            if (result != 0) {
                smsc_log_fatal(logger, "%s: CpMsgPortOpen(userId = %u) failed: %s (code %u)", _logId,
                                _cfg.mpUserId, rc2Txt_SS7_CP(result), result);
                rval = -1;
            } else {
                state = ss7OPENED;
                smsc_log_info(logger, "%s: state OPENED (userId = %u)", _logId, _cfg.mpUserId);
            }
        } break;

        case ss7OPENED: { //Connecting user to TCAP unit input queue.
            if (!connectUnits()) {
                rval = -1;
                smsc_log_fatal(logger, "%s: MsgConn(TCAP all instances) failed", _logId);
                break;
            }
            state = ss7CONNECTED;
            _sync.notify(); //awake MsgListener
            smsc_log_info(logger, "%s: state CONNECTED (userId = %u)", _logId, _cfg.mpUserId);
        } break;

        default:;
        } /* eosw */
    }
    return rval;
}

void TCAPDispatcher::disconnectCP(SS7State_T downTo/* = ss7None*/)
{
    if (state > downTo)
        smsc_log_info(logger, "%s: disconnecting SS7 stack: %u -> %u ..", _logId, state, downTo);
    while (state > downTo) {
        switch (state) {
        case ss7CONNECTED: { //Releasing of connections to other users.
            unbindSSNs();
            disconnectUnits();
            smsc_log_info(logger, "%s: state OPENED", _logId);
            state = ss7OPENED;
        } break;

        case ss7OPENED: { //Closing of the input queue.
            USHORT_T result;
            if ((result = MsgClose(_cfg.mpUserId)) != 0)
                smsc_log_error(logger, "%s: MsgClose(userId = %u) failed: %s (code %u)",
                               _logId, (unsigned)_cfg.mpUserId, 
                               rc2Txt_SS7_CP(result), result);
            smsc_log_info(logger, "%s: state INITED", _logId);
            state = ss7INITED;
        } break;

        case ss7INITED: {
            MsgExit();
            state = ss7REGISTERED;
            smsc_log_debug(logger, "%s: state REGISTERED", _logId);
        } break;

        case ss7REGISTERED: {
            state = ss7None;
            smsc_log_debug(logger, "%s: state NONE", _logId);
        } break;
        //case ss7None:
        default:;
        } /* eosw */
    }
    return;
}

/* ************************************************************************* *
 * TCAP sessions factory methods
 * ************************************************************************* */
//Binds SSN to all known TCAP BE instances and initializes SSNSession (TCAP dialogs factory)
SSNSession* TCAPDispatcher::openSSN(UCHAR_T ssn_id, USHORT_T max_dlg_id/* = 2000 */,
                                    Logger * uselog/* = NULL*/)
{
    SSNSession* pSession = NULL;
    {
        MutexGuard tmp(_sync);
        if (!(pSession = lookUpSSN(ssn_id))) {
            pSession = new SSNSession(this->acRegistry() , ssn_id, _cfg.mpUserId,
                                      _cfg.ss7Units[TCAP_ID].instIds,
                                      max_dlg_id, uselog ? uselog : logger);
            sessions.insert(SSNmap_T::value_type(ssn_id, pSession));
        } else {
            pSession->incMaxDlgs(max_dlg_id);
            smsc_log_debug(logger, "%s: SSN[%u] already inited, state: %u", _logId,
                           ssn_id, pSession->getState());
        }
    }
    if (pSession->getState() != SSNBinding::ssnBound) {
        if (!bindSSN(pSession) || pSession->Wait(RECV_TIMEOUT)) //wait for confirmation
            return (pSession->getState() >= SSNBinding::ssnPartiallyBound) ? pSession : 0;
    }
    return pSession;
}

SSNSession* TCAPDispatcher::findSession(UCHAR_T ssn) const
{
    MutexGuard tmp(_sync);
    return lookUpSSN(ssn);
}

void TCAPDispatcher::confirmSSN(uint8_t ssn, uint8_t tc_inst_id, uint8_t bindResult)
{
    SSNSession* pSession = findSession(ssn);
    if (!pSession) {
        smsc_log_warn(logger, "%s: BindConf for invalid/inactive SSN[%u]", _logId, (unsigned)ssn);
        return;
    }
    UNITStatus::BindStatus_e rval = UNITStatus::unitError;

    if ((bindResult == TC_BindResult::bindOk)
        || (bindResult == TC_BindResult::ssnInUse)) {
        rval = UNITStatus::unitBound;
    } else {
        smsc_log_error(logger, "%s: SSN[%u] BindConf(tcInstId = %u) failed: '%s' (code 0x%X)",
                       _logId, (unsigned)ssn, (unsigned)tc_inst_id,
                       rc2Txt_TC_BindResult(bindResult), bindResult);
    }
    if (!pSession->SignalUnit(tc_inst_id, rval))
        smsc_log_warn(logger, "%s: SSN[%u] BindConf() for invalid instance = %u",
                      _logId, (unsigned)ssn, (unsigned)tc_inst_id);
    return;
}


} // namespace inap
} // namespace inmgr
} // namespace smsc

