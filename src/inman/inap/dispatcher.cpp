static char const ident[] = "$Id$";

#include "inman/inap/dispatcher.hpp"
#include "inman/common/util.hpp"

using smsc::inman::common::format;

static const USHORT_T MSG_INIT_MAX_ENTRIES = 512;

namespace smsc  {
namespace inman {
namespace inap  {

//assign this before calling TCAPDispatcher::getInstance()
Logger * _EINSS7_logger_DFLT = NULL;

TCAPDispatcher::TCAPDispatcher()
    : logger(_EINSS7_logger_DFLT), state(ss7None), userId(0), running(false)
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
    if (sessions.size())
        closeAllSessions();
    disconnect();
}

bool TCAPDispatcher::reconnect(SS7State_T upTo/* = ss7CONNECTED*/)
{
    if (!connect(userId, upTo)) { //try total reconnect
        disconnect();
        return connect(userId, upTo);
    }
    return true;
}

bool TCAPDispatcher::connect(USHORT_T user_id/* = MSG_USER_ID*/, SS7State_T upTo/* = ss7CONNECTED*/)
{
    USHORT_T result;
    bool     rval = true;

    smsc_log_debug(logger, "TCAPDsp: connecting SS7 stack up to state: %u ..", upTo);
    userId = user_id; //assign here, to make possible future reconnect attempts
    _mutex.Lock();
    while (rval && (state < upTo)) {
        switch (state) {
        case ss7None: { //reading cp.cnf, connecting to CP manager
            result = EINSS7CpMsgInitNoSig(MSG_INIT_MAX_ENTRIES);
            if (result != 0) {
                smsc_log_fatal(logger, "TCAPDsp: CpMsgInitNoSig() failed: %s (code %u)",
                                     getReturnCodeDescription(result), result);
                rval = false;
            } else {
                state = ss7INITED;
                smsc_log_debug(logger, "TCAPDsp: state INITED");
            }
        } break;

        case ss7INITED: {  //Opening of the input queue. //MsgOpen(userId)
            result = EINSS7CpMsgPortOpen(userId, TRUE);
            if (result != 0) {
                smsc_log_fatal(logger, "TCAPDsp: CpMsgPortOpen(user = %u) failed: %s (code %u)",
                                user_id, getReturnCodeDescription(result), result);
                rval = false;
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
                rval = false;
            } else {
                state = ss7CONNECTED;
                smsc_log_debug(logger, "TCAPDsp: state CONNECTED (userId = %u)", userId);
            }
        } break;

        case ss7CONNECTED: {
            _mutex.Unlock();
            Start();
            if (!thread) {
                smsc_log_error(logger, "TCAPDsp: unable to start Listener thread");
                rval = false;
            }
        } break;

        default:;
        } /* eosw */
    }
    _mutex.Unlock();
    return rval;
}

void TCAPDispatcher::disconnect(void)
{
    if (state == ss7None)
        return;

    smsc_log_debug(logger, "TCAPDsp: disconnecting SS7 stack ..");
    //stop message listener first
    if (running)
        Stop();
    WaitFor();

    USHORT_T result;
    _mutex.Lock();
    while (state > ss7None) {
        switch (state) {
        case ss7CONNECTED: { //Releasing of connections to other users.
            if ((result = MsgRel(userId, TCAP_ID)) != 0)
                smsc_log_error(logger, "TCAPDsp: MsgRel(%d,%d) failed: %s (code %d)",
                               userId, TCAP_ID, getReturnCodeDescription(result),
                               result);
            smsc_log_debug(logger, "TCAPDsp: state OPENED");
            state = ss7OPENED;
        } break;

        case ss7OPENED: { //Closing of the input queue.
            if ((result = MsgClose(userId)) != 0)
                smsc_log_error(logger, "TCAPDsp: MsgClose(%d) failed: %s (code %d)",
                               userId, getReturnCodeDescription(result), result);
            smsc_log_debug(logger, "TCAPDsp: state INITED");
            state = ss7INITED;
        } break;

        case ss7INITED: {
            MsgExit();
            state = ss7None;
            smsc_log_info(logger, "TCAPDsp: SS7 stack disconnection complete");
        } break;

        default:;
        } /* eosw */
    }
    _mutex.Unlock();
}

/* ************************************************************************* *
 * TCAP/SCCP message listener methods
 * ************************************************************************* */
void TCAPDispatcher::Stop(void)
{
    _mutex.Lock();
    running = false;
    _mutex.Unlock();
}

int TCAPDispatcher::Listen(void)
{
    USHORT_T result = 0;

    _mutex.Lock();
    running = true;
    while (running) {
        _mutex.Unlock();
        MSG_T msg;
        memset(&msg, 0, sizeof( MSG_T ));
        msg.receiver = MSG_USER_ID;
        
        result = MsgRecvEvent(&msg, NULL, NULL, /*MSG_INFTIM*/ 100 ); //ms
        _mutex.Lock();
        if (!running)
            break;
        if (MSG_TIMEOUT == result)
            continue;
        if (result != 0)
            smsc_log_error(logger, "TCAPDsp: MsgRecv() failed with code %d (%s)",
                           result, getReturnCodeDescription(result));
        else
            EINSS7_I97THandleInd(&msg);
        EINSS7CpReleaseMsgBuffer(&msg);
    }
    _mutex.Unlock();
    return (int)result;
}


// Listener thread entry point
int TCAPDispatcher::Execute(void)
{
    int result = 0;
    if (state < ss7CONNECTED) {
        smsc_log_error(logger, "TCAPDsp: SS7 stack is not connected!");
        return -1;
    }
    try {
        state = ss7LISTEN;
        smsc_log_debug(logger, "TCAPDsp: Listener thread started");
        result = Listen();
    } catch (const std::exception& exc) {
        smsc_log_fatal(logger, "TCAPDsp: Listener thread got an exception: %s", exc.what() );
        result = -2;
    }
    _mutex.Lock();
    if (state == ss7LISTEN)
        state = ss7CONNECTED;
    _mutex.Unlock();
    smsc_log_debug(logger, "TCAPDsp: Listener thread finished, reason %d", result);
    return result;
}

/* ************************************************************************* *
 * TCAP sessions factory methods
 * ************************************************************************* */
Session* TCAPDispatcher::openSession(UCHAR_T ownssn, const char* ownaddr,
                              UCHAR_T remotessn, const char* remoteaddr)
{
    if (state < ss7CONNECTED) {
        smsc_log_error(logger, "TCAPDsp: can't open session (oSSN=%u), factory state: %d",
                        ownssn, state);
        return NULL;
    }
    smsc_log_debug(logger, "TCAPDsp: Opening session (oSSN=%u, oGT=%s, rSSN=%d, rGT=%s)",
                   ownssn, ownaddr, remotessn, remoteaddr);

    Session* pSession = NULL;
    if (sessions.find(ownssn) != sessions.end()) {
        smsc_log_error(logger, "TCAPDsp: Session (oSSN=%d) is already opened", ownssn);
    } else {
        USHORT_T  result = EINSS7_I97TBindReq(ownssn, userId, TCAP_INSTANCE_ID, EINSS7_I97TCAP_WHITE_USER);
        if (result != 0) {
            smsc_log_error(logger, "TCAPDsp: BindReq(oSSN=%u) failed with code %d (%s)",
                            ownssn, result, getTcapReasonDescription(result));
        } else {
            pSession = new Session(ownssn, ownaddr, remotessn, remoteaddr, logger);
            sessions.insert(SessionsMap_T::value_type(ownssn, pSession));
        }
    }
    return pSession;
}

Session* TCAPDispatcher::openSession(UCHAR_T SSN, const char* ownaddr,
                                         const char* remoteaddr)
{
    return openSession(SSN, ownaddr, SSN, remoteaddr);
}

bool TCAPDispatcher::confirmSession(UCHAR_T ssn, UCHAR_T bindResult)
{
    Session* pSession = findSession(ssn);
    if (!pSession) {
        smsc_log_warn(logger, "TCAPDsp: Invalid/inactive session, SSN: %u", ssn);
        return false;
    }

    if (bindResult != EINSS7_I97TCAP_BIND_OK) {
        smsc_log_error(logger, "TCAPDsp: SSN[%u] BindReq failed: '%s' (code 0x%X)",
                       (unsigned)ssn, getTcapBindErrorMessage(bindResult), bindResult);
        closeSession(pSession);
        return false;
    }
    pSession->setState(Session::BOUND);
    return true;
}

Session* TCAPDispatcher::findSession(UCHAR_T SSN)
{
    SessionsMap_T::const_iterator it = sessions.find(SSN);
    return (it == sessions.end()) ? NULL : (*it).second;
}

void TCAPDispatcher::closeSession(Session* pSession)
{
    if (!pSession) {
        smsc_log_warn(logger, "TCAPDsp: Attemp to close NULL session");
        return;
    }

    UCHAR_T ssn = pSession->getSSN();
    smsc_log_debug(logger, "TCAPDsp: Closing session (SSN=%u)", ssn);

    if (sessions.find(ssn) == sessions.end())
        smsc_log_error(logger, "TCAPDsp: Session (SSN=%u) doesn't exist", ssn);
    else
        sessions.erase(ssn);
    delete pSession;

    USHORT_T  result = EINSS7_I97TUnBindReq(ssn, userId, TCAP_INSTANCE_ID);
    if (result != 0)
        smsc_log_error(logger, "TCAPDsp: UnBindReq(oSSN=%u) failed with code %d (%s)",
                       ssn, result, getTcapReasonDescription(result));
}

void TCAPDispatcher::closeAllSessions()
{
    smsc_log_debug(logger,"TCAPDsp: Closing all sessions ..");

    SessionsMap_T  cpmap = sessions;
    sessions.clear();
    for (SessionsMap_T::iterator it = cpmap.begin(); it != cpmap.end(); it++) {
        Session* pSession = (*it).second;
        UCHAR_T  ssn = pSession->getSSN();
        smsc_log_debug(logger, "TCAPDsp: Closing session (SSN=%u)", ssn);
        delete pSession;

        USHORT_T  result = EINSS7_I97TUnBindReq(ssn, userId, TCAP_INSTANCE_ID);
        if (result != 0)
            smsc_log_error(logger, "TCAPDsp: UnBindReq(oSSN=%u) failed with code %d (%s)",
                           ssn, result, getTcapReasonDescription(result));
    }
}

} // namespace inap
} // namespace inmgr
} // namespace smsc

