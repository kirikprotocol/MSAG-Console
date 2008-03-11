#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/inap/dispatcher.hpp"
#include "inman/inap/ACRegistry.hpp"
#include "inman/inap/TCAPErrors.hpp"
using smsc::inman::inap::rc2Txt_TC_BindResult;

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

TCAPDispatcher::TCAPDispatcher()
    : state(ss7None), _listening(false), logger(NULL), _logId("TCAPDsp")
{
    ApplicationContextRegistry::get();
    TCCbkLink::get(); //Link TC API Callbacks
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
    _cfg.userId += 39; //adjust USER_ID to PortSS7 units id
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
    USHORT_T result = 0;

    _sync.Lock(); //waits here for connect()
    _listening = true;
    _sync.notify();
    while (_listening) {
        if (state != ss7CONNECTED) {
            if (connectCP(ss7CONNECTED) >= 0) {
                bindSSNs();
                bindTimer = 0;
            } else {
                _sync.wait(RECONNECT_TIMEOUT);
                continue;
            }
        }
        _sync.Unlock();
        MSG_T msg;
        memset(&msg, 0, sizeof( MSG_T ));
        msg.receiver = _cfg.userId;

        result = MsgRecvEvent(&msg, NULL, NULL, RECV_TIMEOUT);
        if (MSG_TIMEOUT != result) {
            if (!result)
                EINSS7_I97THandleInd(&msg); //calls callbacks.cpp::*()
            EINSS7CpReleaseMsgBuffer(&msg);
            if (result) {
                smsc_log_error(logger, "%s: MsgRecv() failed with code %d (%s)", _logId,
                               result, rc2Txt_SS7_CP(result));
                if ((MSG_BROKEN_CONNECTION == result) || (MSG_NOT_CONNECTED == result))
                    onDisconnect();
            }
        } else { //check for SSN bind state
            MutexGuard grd(_sync);
            if (unbindedSSNs())
                bindTimer++;
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
unsigned TCAPDispatcher::unbindedSSNs(void)
{
    unsigned rval = 0;
    for (SSNmap_T::iterator it = sessions.begin(); it != sessions.end(); ++it) {
        SSNSession* pSession = (*it).second;
        if (pSession->getState() != smsc::inman::inap::ssnBound)
            rval++;
    }
    return rval;
}

bool TCAPDispatcher::bindSSN(UCHAR_T ssn)
{
    //listener thread will call confirmSession()
    smsc_log_debug(logger, "%s: BindReq(SSN=%u, userId=%u) ..", _logId, ssn, _cfg.userId);
    USHORT_T  result = EINSS7_I97TBindReq(ssn, _cfg.userId, TCAP_INSTANCE_ID,
                                          EINSS7_I97TCAP_WHITE_USER);
    if (result) {
        smsc_log_error(logger, "%s: BindReq(SSN=%u) failed with code %u (%s)", _logId,
                        ssn, result, rc2Txt_TC_APIError(result));
        return false;
    }
    return true;
}

void TCAPDispatcher::unbindSSNs(void)
{
    for (SSNmap_T::iterator it = sessions.begin(); it != sessions.end(); ++it) {
        UCHAR_T ssn = (*it).first;
        SSNSession* pSession = (*it).second;
        EINSS7_I97TUnBindReq(ssn, _cfg.userId, TCAP_INSTANCE_ID);
        pSession->setState(smsc::inman::inap::ssnIdle);
    }
    return;
}

void TCAPDispatcher::bindSSNs(void)
{
    for (SSNmap_T::iterator it = sessions.begin(); it != sessions.end(); ++it) {
        UCHAR_T ssn = (*it).first;
        SSNSession* pSession = (*it).second;
        //TBindReq() will call confirmSession()
        smsc_log_debug(logger, "%s: BindReq(SSN=%u, userId=%u) ..", _logId, ssn, _cfg.userId);
        USHORT_T  result = EINSS7_I97TBindReq(ssn, _cfg.userId, TCAP_INSTANCE_ID,
                                                   EINSS7_I97TCAP_WHITE_USER);
        if (result) {
            smsc_log_error(logger, "%s: BindReq(SSN=%u) failed with code %u (%s)", _logId,
                            ssn, result, rc2Txt_TC_APIError(result));
            pSession->setState(smsc::inman::inap::ssnError);
        } else
            pSession->setState(smsc::inman::inap::ssnIdle);
    }
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
            //Initiation of the message handling and allocating memory,
            //reading cp.cnf, connecting to CP manager
            result = EINSS7CpMsgInitNoSig(_cfg.maxMsgNum);
            if (result != 0) {
                smsc_log_fatal(logger, "%s: CpMsgInitNoSig() failed: %s (code %u)", _logId,
                                     rc2Txt_SS7_CP(result), result);
                rval = -1;
            } else {
                state = ss7INITED;
                smsc_log_debug(logger, "%s: state INITED", _logId);
            }
        } break;

        case ss7INITED: {  //Opening of the input queue. //MsgOpen(userId)
            result = EINSS7CpMsgPortOpen(_cfg.userId, TRUE);
            if (result != 0) {
                smsc_log_fatal(logger, "%s: CpMsgPortOpen(userId = %u) failed: %s (code %u)", _logId,
                                _cfg.userId, rc2Txt_SS7_CP(result), result);
                rval = -1;
            } else {
                state = ss7OPENED;
                smsc_log_debug(logger, "%s: state OPENED (userId = %u)", _logId, _cfg.userId);
            }
        } break;

        case ss7OPENED: { //Connecting user to TCAP unit input queue.
            result = MsgConn(_cfg.userId, TCAP_ID);
            if (result != 0) {
                smsc_log_fatal(logger, "%s: MsgConn() failed: %s (code %u)", _logId,
                             rc2Txt_SS7_CP(result), result);
                rval = -1;
            } else {
                state = ss7CONNECTED;
                _sync.notify(); //awake MsgListener
                smsc_log_debug(logger, "%s: state CONNECTED (userId = %u)", _logId, _cfg.userId);
                return 1;
            }
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
            USHORT_T result;
            unbindSSNs();
            if ((result = MsgRel(_cfg.userId, TCAP_ID)) != 0)
                smsc_log_error(logger, "%s: MsgRel(%d,%d) failed: %s (code %d)", _logId,
                               _cfg.userId, TCAP_ID, rc2Txt_SS7_CP(result), result);
            smsc_log_debug(logger, "%s: state OPENED", _logId);
            state = ss7OPENED;
        } break;

        case ss7OPENED: { //Closing of the input queue.
            USHORT_T result;
            if ((result = MsgClose(_cfg.userId)) != 0)
                smsc_log_error(logger, "%s: MsgClose(%d) failed: %s (code %d)", _logId,
                               _cfg.userId, rc2Txt_SS7_CP(result), result);
            smsc_log_debug(logger, "%s: state INITED", _logId);
            state = ss7INITED;
        } break;

        case ss7INITED: {
            MsgExit();
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
//Binds SSN and initializes SSNSession (TCAP dialogs factory)
SSNSession* TCAPDispatcher::openSSN(UCHAR_T ssn_id, USHORT_T max_dlg_id, USHORT_T min_dlg_id,
                                    Logger * uselog/* = NULL*/)
{
    SSNSession* pSession = NULL;
    {
        MutexGuard tmp(_sync);
        if (!(pSession = lookUpSSN(ssn_id))) {
            pSession = new SSNSession(this, ssn_id, _cfg.userId, max_dlg_id, min_dlg_id,
                                      uselog ? uselog : logger);
            sessions.insert(SSNmap_T::value_type(ssn_id, pSession));
        } else {
            pSession->incMaxDlgs(max_dlg_id);
            smsc_log_debug(logger, "%s: SSN[%u] already inited, state: %u", _logId,
                           ssn_id, pSession->getState());
        }
    }
    if (pSession->getState() != smsc::inman::inap::ssnBound) {
        if (!bindSSN(ssn_id) || pSession->Wait(RECV_TIMEOUT)) //wait for confirmation
            return NULL;
    }
    return pSession;
}

SSNSession* TCAPDispatcher::findSession(UCHAR_T ssn) const
{
    MutexGuard tmp(_sync);
    return lookUpSSN(ssn);
}

bool TCAPDispatcher::confirmSSN(UCHAR_T ssn, UCHAR_T bindResult)
{
    bool    rval = false;
    SSNSession* pSession = findSession(ssn);
    if (!pSession) {
        smsc_log_warn(logger, "%s: BindConf for invalid/inactive SSN[%u]", _logId, ssn);
        return false;
    }

    if ((bindResult == EINSS7_I97TCAP_BIND_OK)
        || (bindResult == EINSS7_I97TCAP_BIND_SSN_IN_USE)) {
        pSession->setState(smsc::inman::inap::ssnBound);
        rval = true;
    } else {
        smsc_log_error(logger, "%s: SSN[%u] BindReq failed: '%s' (code 0x%X)", _logId,
                           (unsigned)ssn, rc2Txt_TC_BindResult(bindResult), bindResult);
        pSession->setState(smsc::inman::inap::ssnError);
    }
    pSession->Signal();
    return rval;
}


} // namespace inap
} // namespace inmgr
} // namespace smsc

