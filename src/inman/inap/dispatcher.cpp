static char const ident[] = "$Id$";

#include "inman/inap/dispatcher.hpp"
#include "inman/common/util.hpp"

using smsc::inman::common::format;

namespace smsc  {
namespace inman {
namespace inap  {

//assign this before calling TCAPDispatcher::getInstance()
Logger * _EINSS7_logger_DFLT = NULL;

/* ************************************************************************** *
 * class TCAPDispatcher implementation:
 * ************************************************************************** */
static const USHORT_T MSG_INIT_MAX_ENTRIES = 512;

#define MAX_BIND_SECS   5       //maximum timeout on SSN bind request, units: secs
#define RECV_TIMEOUT    400     //message receiving timeout, units: millisecs
#define RECONNECT_TIMEOUT 3000  //SS7 stack reconnection timeout, units: millisecs

#define MAX_BIND_ATTEMPTS (MAX_BIND_SECS*1000/RECV_TIMEOUT)

TCAPDispatcher::TCAPDispatcher()
    : logger(_EINSS7_logger_DFLT), state(ss7None), userId(0), _status(dspStopped)
{
    if (!_EINSS7_logger_DFLT)
        _EINSS7_logger_DFLT = logger = Logger::getInstance("smsc.inman.inap");

    //Initiation of the message handling and allocating memory. (cp.cnf is red here)
    USHORT_T result = EINSS7CpMsgInitNoSig(MSG_INIT_MAX_ENTRIES);
    if (result != 0) {
        smsc_log_fatal(logger, "TCAPDsp: CpMsgInitNoSig() failed: %s (code %u)",
                             getReturnCodeDescription(result), result);
    } else {
        state = ss7INITED;
        smsc_log_debug(logger, "TCAPDsp: state INITED");
    }
}

TCAPDispatcher* TCAPDispatcher::getInstance()
{
    static TCAPDispatcher instance;
    return &instance;
}


TCAPDispatcher::~TCAPDispatcher()
{
    if (state > ss7None)
        disconnect();
    for (SSNmap_T::iterator it = sessions.begin(); it != sessions.end(); it++) {
        SSNSession* pSession = (*it).second;
        delete pSession;
    }
}

bool TCAPDispatcher::reconnect(UCHAR_T toSsn)
{
    if (!connect(userId, toSsn)) { //starts MsgListener if necessary
        disconnectCP(ss7None);
        return connectCP(ss7CONNECTED) < 0 ? false : true;
    }
    return true;
}

bool TCAPDispatcher::connect(USHORT_T user_id, UCHAR_T toSsn)
{
    MutexGuard grd(_mutex);
    userId = user_id; //assign here, to make possible future reconnect attempts
    if (!thread) {
        Start(); //starts Listen() that waits for _mutex
        if (lstEvent.Wait(RECV_TIMEOUT)) {
            smsc_log_error(logger, "TCAPDsp: unable to start MsgListener thread");
            return false;
        }
    }
    if (connectCP(ss7CONNECTED) < 0)
        return false;
    return bindSSN(toSsn);
}

void TCAPDispatcher::disconnect(void)
{
    unbindSSNs(); //unbind all SSNs
    if (_status != dspStopped) //stop message listener
        Stop();
    WaitFor();
    thread = 0;

    MutexGuard grd(_mutex);
    if (state > ss7None)
        disconnectCP(ss7None);
    smsc_log_info(logger, "TCAPDsp: SS7 stack disconnection complete");
}

/* ************************************************************************* *
 * TCAP/SCCP message listener methods
 * ************************************************************************* */
void TCAPDispatcher::Stop(void)
{
    _mutex.Lock();
    _status = dspStopped;
    _mutex.Unlock();
    WaitFor();
    thread = 0;
}

void TCAPDispatcher::onDisconnect(void)
{
    MutexGuard grd(_mutex);
    disconnectCP(ss7INITED);
}

int TCAPDispatcher::Listen(void)
{
    unsigned bindTimer = 0;
    USHORT_T result = 0;

    _mutex.Lock(); //waits here for connect()
    _status = dspListening;
    while (_status != dspStopped) {
        if (state != ss7CONNECTED) {
            _mutex.wait(RECONNECT_TIMEOUT);
            if (connectCP(ss7CONNECTED) >= 0) {
                bindSSNs();
                bindTimer = 0;
            }
            continue;
        }
        _mutex.Unlock();
        MSG_T msg;
        memset(&msg, 0, sizeof( MSG_T ));
        msg.receiver = userId;

        result = MsgRecvEvent(&msg, NULL, NULL, RECV_TIMEOUT);
        if (MSG_TIMEOUT != result) {
            if (!result)
                EINSS7_I97THandleInd(&msg); //calls callbacks.cpp::*()
            EINSS7CpReleaseMsgBuffer(&msg);
            if (result) {
                smsc_log_error(logger, "TCAPDsp: MsgRecv() failed with code %d (%s)",
                               result, getReturnCodeDescription(result));
                if ((MSG_BROKEN_CONNECTION == result) || (MSG_NOT_CONNECTED == result))
                    onDisconnect();
            }
        } else { //check for SSN bind state
            if (unbindedSSNs())
                bindTimer++;
            if (bindTimer >= MAX_BIND_ATTEMPTS) {
                smsc_log_error(logger, "TCAPDsp: not all SSNs binded, reconnecting");
                disconnectCP(ss7INITED);
            }
        }
        _mutex.Lock();
    }
    _mutex.Unlock();
    if (MSG_TIMEOUT == result)
        result = 0;
    return (int)result;
}

// Listener thread entry point
int TCAPDispatcher::Execute(void)
{
    int result = 0;
    try {
        lstEvent.Signal();
        smsc_log_info(logger, "TCAPDsp: MsgListener thread started");
        result = Listen();
    } catch (const std::exception& exc) {
        smsc_log_fatal(logger, "TCAPDsp: MsgListener thread got an exception: %s", exc.what() );
        result = -2;
    }
    smsc_log_debug(logger, "TCAPDsp: MsgListener thread finished, reason %d", result);
    return result;
}

/* -------------------------------------------------------------------------- *
 * PRIVATE:
 * -------------------------------------------------------------------------- */
unsigned TCAPDispatcher::unbindedSSNs(void)
{
    unsigned rval = 0;
    for (SSNmap_T::iterator it = sessions.begin(); it != sessions.end(); it++) {
        SSNSession* pSession = (*it).second;
        if (pSession->getState() != SSNSession::ssnBound)
            rval++;
    }
    return rval;
}

bool TCAPDispatcher::bindSSN(UCHAR_T ssn)
{
    SSNSession* pSession;
    SSNmap_T::iterator it = sessions.find(ssn);

    if (it == sessions.end()) {
        pSession = new SSNSession(ssn, userId, logger);
        sessions.insert(SSNmap_T::value_type(ssn, pSession));
    } else
        pSession = (*it).second;
    //TBindReq() will call confirmSession()
    smsc_log_debug(logger, "TCAPDsp: BindReq(SSN=%u, userId=%u) ..", ssn, userId);
    USHORT_T  result = EINSS7_I97TBindReq(ssn, userId, TCAP_INSTANCE_ID,
                                               EINSS7_I97TCAP_WHITE_USER);
    if (result) {
        smsc_log_error(logger, "TCAPDsp: BindReq(SSN=%u) failed with code %u (%s)",
                        ssn, result, getTcapReasonDescription(result));
        pSession->setState(SSNSession::ssnError);
        return false;
    }
    pSession->setState(SSNSession::ssnIdle);
    return true;
}

void TCAPDispatcher::unbindSSNs(void)
{
    for (SSNmap_T::iterator it = sessions.begin(); it != sessions.end(); it++) {
        UCHAR_T ssn = (*it).first;
        SSNSession* pSession = (*it).second;
        EINSS7_I97TUnBindReq(ssn, userId, TCAP_INSTANCE_ID);
        pSession->setState(SSNSession::ssnIdle);
    }
    return;
}

void TCAPDispatcher::bindSSNs(void)
{
    for (SSNmap_T::iterator it = sessions.begin(); it != sessions.end(); it++) {
        UCHAR_T ssn = (*it).first;
        SSNSession* pSession = (*it).second;
        //TBindReq() will call confirmSession()
        smsc_log_debug(logger, "TCAPDsp: BindReq(SSN=%u, userId=%u) ..", ssn, userId);
        USHORT_T  result = EINSS7_I97TBindReq(ssn, userId, TCAP_INSTANCE_ID,
                                                   EINSS7_I97TCAP_WHITE_USER);
        if (result) {
            smsc_log_error(logger, "TCAPDsp: BindReq(SSN=%u) failed with code %u (%s)",
                            ssn, result, getTcapReasonDescription(result));
            pSession->setState(SSNSession::ssnError);
        } else
            pSession->setState(SSNSession::ssnIdle);
    }
    return;
}

//Returns:  (-1) - failed to connect, 0 - already connected, 1 - successfully connected
int TCAPDispatcher::connectCP(SS7State_T upTo/* = ss7CONNECTED*/)
{
    smsc_log_info(logger, "TCAPDsp: connecting SS7 stack: %u -> %u ..", state, upTo);
    if (state >= upTo)
        return 0;
    
    USHORT_T result;
    int    rval = 1;
    while ((rval > 0) && (state < upTo)) {
        switch (state) {
        case ss7None: { //reading cp.cnf, connecting to CP manager
            result = EINSS7CpMsgInitNoSig(MSG_INIT_MAX_ENTRIES);
            if (result != 0) {
                smsc_log_fatal(logger, "TCAPDsp: CpMsgInitNoSig() failed: %s (code %u)",
                                     getReturnCodeDescription(result), result);
                rval = -1;
            } else {
                state = ss7INITED;
                smsc_log_debug(logger, "TCAPDsp: state INITED");
            }
        } break;

        case ss7INITED: {  //Opening of the input queue. //MsgOpen(userId)
            result = EINSS7CpMsgPortOpen(userId, TRUE);
            if (result != 0) {
                smsc_log_fatal(logger, "TCAPDsp: CpMsgPortOpen(userId = %u) failed: %s (code %u)",
                                userId, getReturnCodeDescription(result), result);
                rval = -1;
            } else {
                state = ss7OPENED;
                smsc_log_debug(logger, "TCAPDsp: state OPENED (userId = %u)", userId);
            }
        } break;

        case ss7OPENED: { //Connecting to SCCP input queue.
            result = MsgConn(userId, TCAP_ID);
            if (result != 0) {
                smsc_log_fatal(logger, "TCAPDsp: MsgConn() failed: %s (code %u)", 
                             getReturnCodeDescription(result), result);
                rval = -1;
            } else {
                state = ss7CONNECTED;
                _mutex.notify(); //awake MsgListener
                smsc_log_debug(logger, "TCAPDsp: state CONNECTED (userId = %u)", userId);
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
    smsc_log_info(logger, "TCAPDsp: disconnecting SS7 stack: %u -> %u ..", state, downTo);
    while (state > downTo) {
        switch (state) {
        case ss7CONNECTED: { //Releasing of connections to other users.
            USHORT_T result;
            unbindSSNs();
            if ((result = MsgRel(userId, TCAP_ID)) != 0)
                smsc_log_error(logger, "TCAPDsp: MsgRel(%d,%d) failed: %s (code %d)",
                               userId, TCAP_ID, getReturnCodeDescription(result),
                               result);
            smsc_log_debug(logger, "TCAPDsp: state OPENED");
            state = ss7OPENED;
        } break;

        case ss7OPENED: { //Closing of the input queue.
            USHORT_T result;
            if ((result = MsgClose(userId)) != 0)
                smsc_log_error(logger, "TCAPDsp: MsgClose(%d) failed: %s (code %d)",
                               userId, getReturnCodeDescription(result), result);
            smsc_log_debug(logger, "TCAPDsp: state INITED");
            state = ss7INITED;
        } break;

        case ss7INITED: {
            MsgExit();
            state = ss7None;
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
bool TCAPDispatcher::confirmSSN(UCHAR_T ssn, UCHAR_T bindResult)
{
    SSNSession* pSession = findSession(ssn);
    if (!pSession) {
        smsc_log_warn(logger, "TCAPDsp: BindConf for invalid/inactive session, SSN: %u", ssn);
        return false;
    }

    if ((bindResult == EINSS7_I97TCAP_BIND_OK)
        || (bindResult == EINSS7_I97TCAP_BIND_SSN_IN_USE)) {
        pSession->setState(SSNSession::ssnBound);
        return true;
    }
    smsc_log_error(logger, "TCAPDsp: SSN[%u] BindReq failed: '%s' (code 0x%X)",
                       (unsigned)ssn, getTcapBindErrorMessage(bindResult), bindResult);
    pSession->setState(SSNSession::ssnError);
    return false;
}

//Opens or reinitializes SSNSession
SSNSession* TCAPDispatcher::openSession(UCHAR_T ssn, const char* own_addr, /*UCHAR_T rmt_ssn, */
                            const char* rmt_addr, ACOID::DefinedOIDidx dialog_ac_idx,
                            USHORT_T max_id)
{
    SSNSession* pSession = findSession(ssn);
    if (!pSession) {
        smsc_log_error(logger, "TCAPDsp: invalid/inactive session, SSN: %u", ssn);
    } else
        pSession->init(own_addr, rmt_addr, dialog_ac_idx, max_id);
    return pSession;
}

SSNSession* TCAPDispatcher::findSession(UCHAR_T ssn)
{
    MutexGuard tmp(_mutex);
    SSNmap_T::const_iterator it = sessions.find(ssn);
    return (it == sessions.end()) ? NULL : (*it).second;
}

} // namespace inap
} // namespace inmgr
} // namespace smsc

