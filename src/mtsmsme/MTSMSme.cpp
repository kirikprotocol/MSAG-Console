#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/recoder/recode_dll.h>
#include <util/smstext.h>

#include <system/smscsignalhandlers.h>
#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <util/xml/init.h>

#include <system/smscsignalhandlers.h>
#include <system/status.h>

#include "processor/Processor.h"
#include "version.inc"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;
using namespace smsc::mtsmsme::processor;
using namespace smsc::system::Status;
static smsc::logger::Logger *logger = 0;

static Event mtsmSmeWaitEvent, mtsmSmeReady;
static Mutex needStopLock, needReconnectLock;

static int   mtsmSmeReadyTimeout  = 60000;
static bool  bMTSMSmeIsStopped    = false;
static bool  bMTSMSmeIsConnected  = false;
static bool  bMTSMSmeIsConnecting = false;

// TODO: Implement Stop & Reconnect logic !!!

/*
static void setNeedStop(bool stop=true) {
    MutexGuard gauard(needStopLock);
    bMCISmeIsStopped = stop;
    if (stop && taskProcessor) taskProcessor->Stop();
}*/
static bool isNeedStop() {
    MutexGuard gauard(needStopLock);
    return bMTSMSmeIsStopped;
}

static void setNeedReconnect(bool reconnect=true) {
    MutexGuard gauard(needReconnectLock);
    bMTSMSmeIsConnected = !reconnect;
    //if (reconnect && taskProcessor) taskProcessor->Stop();
}
static bool isNeedReconnect() {
    MutexGuard gauard(needReconnectLock);
    return !bMTSMSmeIsConnected;
}

extern bool isMSISDNAddress(const char* string)
{
    try { Address converted(string); } catch (...) { return false; }
    return true;
}
extern bool convertMSISDNStringToAddress(const char* string, Address& address)
{
    try { Address converted(string); address = converted; } catch (...) { return false; }
    return true;
}

class AliasManager
{
private:

    Mutex           aliasLock;
    Hash<Address>   aliasByAddress;

public:

    AliasManager(ConfigView* config) throw(ConfigException)
    {
        MutexGuard guard(aliasLock); // init alias mapping

        smsc_log_info(logger, "Address to alias mapping loading ...");
        std::auto_ptr< std::set<std::string> > setGuard(config->getShortSectionNames());
        std::set<std::string>* set = setGuard.get();
        for (std::set<std::string>::iterator i=set->begin(); i!=set->end(); i++)
        {
            const char* section = (const char *)i->c_str();
            if (!section || !section[0]) continue;

            std::auto_ptr<ConfigView> addressConfigGuard(config->getSubConfig(section));
            ConfigView* addressConfig = addressConfigGuard.get();

            const char* address = addressConfig->getString("address");
            if (!address || !address[0])
                throw ConfigException("Address is empty or wasn't specified");
            const char* alias = addressConfig->getString("alias");
            if (!alias || !alias[0])
                throw ConfigException("Alias for address '%s' is empty or wasn't specified", address);

            Address addressAddress;
            if (!convertMSISDNStringToAddress(address, addressAddress)) {
                smsc_log_warn(logger, "Address '%s' has invalid format, skipped", address);
                continue;
            }
            address = addressAddress.toString().c_str();
            if (!aliasByAddress.Exists(address)) {
                Address aliasAddress;
                if (convertMSISDNStringToAddress(alias, aliasAddress)) aliasByAddress.Insert(address, aliasAddress);
                else smsc_log_warn(logger, "Alias '%s' for address '%s' has invalid format, skipped", alias, address);
            }
            else smsc_log_warn(logger, "Alias for address '%s' already defined", address);
        }
        smsc_log_info(logger, "Address to alias mapping loaded");
    }

    bool getAliasForAddress(const Address& address, Address& alias)
    {
        try
        {
            const char* addressStr = address.toString().c_str();
            if (!addressStr || !addressStr[0]) return false;

            MutexGuard guard(aliasLock);
            Address* aliasPtr = aliasByAddress.GetPtr(addressStr);
            if (!aliasPtr) return false;
            alias = *aliasPtr;
            return true;
        }
        catch (Exception& exc) {
            smsc_log_error(logger, "Alias mapping failure");
            return false;
        }
    }
};

class RequestController
{
private:

    Mutex             requestLock;
    IntHash<Request*> requestBySeqNum;

public:

    RequestController() {};
    ~RequestController()
    {
        // set error for all waiting requests
        MutexGuard guard(requestLock);
        IntHash<Request*>::Iterator it = requestBySeqNum.First();
        int seqNum; Request* request;
        while (it.Next(seqNum, request))
        {
            if (!request) {
                smsc_log_error(logger, "RequestController shutdown: request is null for seqNum=%d", seqNum);
                continue;
            }
            smsc_log_warn(logger, "RequestController shutdown: request for seqNum=%d skipped", seqNum);
            request->setSendResult(SMENOTCONNECTED); // TODO: what errorcode ???
        }

    };

    bool setRequest(int seqNum, Request* request)
    {
        MutexGuard guard(requestLock);
        if (requestBySeqNum.Exist(seqNum)) {
            smsc_log_error(logger, "RequestController: SMPP seqNum=%d already exists", seqNum);
            return false;
        }
        requestBySeqNum.Insert(seqNum, request);
        return true;
    }
    bool stateRequest(int seqNum, int status)
    {
        Request* request = 0;
        {
            MutexGuard guard(requestLock);
            Request** requestPtr = requestBySeqNum.GetPtr(seqNum);
            if (!requestPtr || !(*requestPtr)) {
                smsc_log_error(logger, "RequestController: Request is undefined for seqNum=%d", seqNum);
                return false;
            }
            request = *requestPtr;
            requestBySeqNum.Delete(seqNum);
        }
        request->setSendResult(status);
        return true;
    }
};

class MTSMSmeRequestSender: public RequestSender
{
private:

    SmppSession*        session;
    AliasManager&       aliaser;
    RequestController&  controller;
    Mutex               sendLock;

public:

    MTSMSmeRequestSender(SmppSession* _session, AliasManager& _aliaser,
                         RequestController& _controller) : RequestSender(),
        session(_session), aliaser(_aliaser), controller(_controller) {};

    virtual ~MTSMSmeRequestSender() {
        MutexGuard guard(sendLock);
        session = 0;
    };

    virtual bool send(Request* request)
    {
        MutexGuard guard(sendLock);

        if (!request) {
            smsc_log_error(logger, "Request to send is null in RequestSender");
            return false;
        }
        if (!session) {
            smsc_log_error(logger, "Smpp session is undefined in RequestSender");
            return false;
        }
        SmppTransmitter* asyncTransmitter = session->getAsyncTransmitter();
        if (!asyncTransmitter) {
            smsc_log_error(logger, "Smpp transmitter is undefined in RequestSender");
            return false;
        }

        const SMS* sms = &(request->sms);
        Address da;
        if (!aliaser.getAliasForAddress(sms->destinationAddress, da)) {
            const char* daStr = sms->destinationAddress.toString().c_str();
            smsc_log_error(logger, "Invalid destination address '%s'", daStr ? daStr:"-");
            request->setSendResult(NOROUTE);
            return false;
        }

        PduSubmitSm  sm;
        sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
        fillSmppPduFromSms(&sm, (SMS *)sms);

        int seqNum = session->getNextSeq();
        sm.get_header().set_sequenceNumber(seqNum);
        sm.get_message().get_dest()  .set_typeOfNumber(da.type);
        sm.get_message().get_dest()  .set_numberingPlan(da.plan);
        sm.get_message().get_dest()  .set_value(da.value);

        smsc_log_info(logger,
                      "Sending sms to smsc oa=%s da=%s imsi=%s",
                      sms->originatingAddress.toString().c_str(),
                      da.toString().c_str(),
                      sms->destinationAddress.toString().c_str());

        // TODO: set transactional mode in ESM_CLASS (by OR) ???
        controller.setRequest(seqNum,request);
        asyncTransmitter->sendPdu(&(sm.get_header()));
        return true;
    }
};

class MTSMSmePduListener: public SmppPduEventListener
{
protected:

    RequestController&  controller;

    SmppTransmitter*    syncTransmitter;
    SmppTransmitter*    asyncTransmitter;

public:

    MTSMSmePduListener(RequestController& _controller) : SmppPduEventListener(),
        controller(_controller), syncTransmitter(0), asyncTransmitter(0) {};
    virtual ~MTSMSmePduListener() {};

    void setSyncTransmitter(SmppTransmitter *transmitter) {
        syncTransmitter = transmitter;
    }
    void setAsyncTransmitter(SmppTransmitter *transmitter) {
        asyncTransmitter = transmitter;
    }

    void processResponce(SmppHeader *pdu)
    {
        if (!pdu) return;

        int seqNum = pdu->get_sequenceNumber();
        int status = pdu->get_commandStatus();

        // search and report status to Request via RequestController
        smsc_log_info(logger, "Got responce for seqNum=%d (status=%d)", seqNum, status);
        controller.stateRequest(seqNum, status);
    }
    void handleEvent(SmppHeader *pdu)
    {
        if (bMTSMSmeIsConnecting) {
            mtsmSmeReady.Wait(mtsmSmeReadyTimeout);
            if (bMTSMSmeIsConnecting) {
                disposePdu(pdu);
                return;
            }
        }

        switch (pdu->get_commandId())
        {
        case SmppCommandSet::SUBMIT_SM_RESP:
            processResponce(pdu);
            break;
        case SmppCommandSet::ENQUIRE_LINK: case SmppCommandSet::ENQUIRE_LINK_RESP:
            break;
        case SmppCommandSet::DELIVERY_SM:  case SmppCommandSet::CANCEL_SM_RESP:
        default:
            smsc_log_debug(logger, "Received unsupported Pdu %d !", pdu->get_commandId());
            break;
        }

        disposePdu(pdu);
    }
    void handleError(int errorCode)
    {
        smsc_log_error(logger, "Transport error handled! Code is: %d", errorCode);
        setNeedReconnect(true);
    }
};

class MTSMSmeConfig : public SmeConfig
{
private:

    char *strHost, *strSid, *strPassword, *strSysType, *strOrigAddr;

public:

    MTSMSmeConfig(ConfigView* config) throw(ConfigException)
        : SmeConfig(), strHost(0), strSid(0), strPassword(0), strSysType(0), strOrigAddr(0)
    {
        // Mandatory fields
        strHost = config->getString("host", "SMSC host wasn't defined !"); host = strHost;
        strSid = config->getString("sid", "MTSMSme id wasn't defined !");   sid = strSid;

        port = config->getInt("port", "SMSC port wasn't defined !");
        timeOut = config->getInt("timeout", "Connect timeout wasn't defined !");

        // Optional fields
        try {
            strPassword = config->getString("password", "MTSMSme password wasn't defined !");
            password = strPassword;
        } catch (ConfigException& exc) { password = ""; strPassword = 0; }
        try {
            strSysType = config->getString("systemType", "MTSMSme system type wasn't defined !");
            systemType = strSysType;
        } catch (ConfigException& exc) { systemType = ""; strSysType = 0; }
        try {
            strOrigAddr = config->getString("origAddress", "MTSMSme originating address wasn't defined !");
            origAddr = strOrigAddr;
        } catch (ConfigException& exc) { origAddr = ""; strOrigAddr = 0; }
    };

    virtual ~MTSMSmeConfig()
    {
        if (strHost) delete strHost;
        if (strSid) delete strSid;
        if (strPassword) delete strPassword;
        if (strSysType) delete strSysType;
        if (strOrigAddr) delete strOrigAddr;
    };
};

extern "C" void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT)
    {
        smsc_log_info(logger, "Stopping ...");
        // TODO: call Stop on RequestProcessor
    }
}
extern "C" void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
    smsc::logger::Logger::Shutdown();
}

extern "C" void setShutdownHandler(void)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
    if(pthread_sigmask(SIG_UNBLOCK, &set, NULL) != 0) {
        if (logger) smsc_log_error(logger, "Failed to set signal mask (shutdown handler)");
    }
    sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
}
extern "C" void clearSignalMask(void)
{
    sigset_t set;
    sigemptyset(&set);
    for(int i=1;i<=37;i++)
        if(i!=SIGQUIT && i!=SIGBUS && i!=SIGSEGV) sigaddset(&set,i);
    if(pthread_sigmask(SIG_SETMASK, &set, NULL) != 0) {
        if (logger) smsc_log_error(logger, "Failed to clear signal mask");
    }
}

struct ShutdownThread : public Thread
{
    Event shutdownEvent;

    ShutdownThread() : Thread() {};
    virtual ~ShutdownThread() {};

    virtual int Execute() {
        clearSignalMask();
        setShutdownHandler();
        shutdownEvent.Wait();
        return 0;
    };
    void Stop() {
        shutdownEvent.Signal();
    };
} shutdownThread;

int main(void)
{
    smsc::logger::Logger::Init();
    logger = smsc::logger::Logger::getInstance("mt.sme");

    atexit(atExitHandler);
    clearSignalMask();

    shutdownThread.Start();

    int resultCode = 0;
    try
    {
        smsc_log_info(logger, getStrVersion());

        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView smscConfig(manager, "MTSMSme.SMSC");
        MTSMSmeConfig cfg(&smscConfig);

        ConfigView aliasConfig(manager, "MTSMSme.Aliases");
        AliasManager aliaser(&aliasConfig);

        RequestProcessor* processor = RequestProcessor::getInstance();
        if (!processor) throw Exception("RequestProcessor is undefined");

        while (!isNeedStop())
        {
            RequestController       controller;
            MTSMSmePduListener      listener(controller);
            SmppSession             session(cfg, &listener);
            MTSMSmeRequestSender    sender(&session, aliaser, controller);

            smsc_log_info(logger, "Connecting to SMSC ... ");
            try
            {
                listener.setSyncTransmitter(session.getSyncTransmitter());
                listener.setAsyncTransmitter(session.getAsyncTransmitter());

                bMTSMSmeIsConnecting = true;
                mtsmSmeReady.Wait(0);
                setNeedReconnect(false);
                session.connect();
                processor->setRequestSender(&sender);
                bMTSMSmeIsConnecting = false;
                mtsmSmeReady.Signal();
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what();
                smsc_log_error(logger, "Connect to SMSC failed. Cause: %s", (msg) ? msg:"unknown");
                bMTSMSmeIsConnecting = false;
                setNeedReconnect(true);
                // ??? if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw exc;
                sleep(cfg.timeOut);
                session.close();
                continue;
            }
            smsc_log_info(logger, "Connected.");

            smsc_log_info(logger, "Running RequestProcessor ...");
            processor->Run();
            smsc_log_info(logger, "RequestProcessor exited");

            clearSignalMask();
            smsc_log_info(logger, "Disconnecting from SMSC ...");
            session.close();
        }

    } catch (SmppConnectException& exc) {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            smsc_log_error(logger, "Failed to bind MTSMSme. Exiting");
        resultCode = -1;
    } catch (ConfigException& exc) {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting", exc.what());
        resultCode = -2;
    } catch (Exception& exc) {
        smsc_log_error(logger, "Top level Exception: %s Exiting", exc.what());
        resultCode = -3;
    } catch (exception& exc) {
        smsc_log_error(logger, "Top level exception: %s Exiting", exc.what());
        resultCode = -4;
    } catch (...) {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting");
        resultCode = -5;
    }

    shutdownThread.Stop();
    return resultCode;
}
