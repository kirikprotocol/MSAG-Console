#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Array.hpp>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/recoder/recode_dll.h>
#include <util/smstext.h>

#include <pthread.h>
#include <signal.h>

#include <db/DataSourceLoader.h>
#include <core/synchronization/EventMonitor.hpp>

#include <system/smscsignalhandlers.h>
#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <util/xml/init.h>

#include <admin/service/Component.h>
#include <admin/service/ComponentManager.h>
#include <admin/service/ServiceSocketListener.h>
#include <system/smscsignalhandlers.h>
#include <system/status.h>

#include <util/timeslotcounter.hpp>

#include "TaskProcessor.h"
#include "MCISmeComponent.h"

#include "AbntAddr.hpp"
#include "ProfilesStorage.hpp"
#include "Profiler.h"
#include "ProfilesStorageServer.hpp"

#include "version.inc"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;
using namespace smsc::system;

using namespace smsc::admin;
using namespace smsc::admin::service;

using namespace smsc::mcisme;
using smsc::util::TimeSlotCounter;
using smsc::core::synchronization::EventMonitor;

const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

static smsc::logger::Logger *logger = 0;

static TaskProcessor*         taskProcessor = 0;
static ServiceSocketListener* adminListener = 0; 
static bool bAdminListenerInited = false;

static Event mciSmeWaitEvent;
static Event mciSmeReady;
static int   mciSmeReadyTimeout = 60000;

static Mutex needStopLock;
static Mutex needReconnectLock;

static bool  bMCISmeIsStopped    = false;
static bool  bMCISmeIsConnected  = false;
static bool  bMCISmeIsConnecting = false;

static void setNeedStop(bool stop=true) {
    MutexGuard gauard(needStopLock);
    bMCISmeIsStopped = stop;
    if (stop && taskProcessor) taskProcessor->Stop();
}
static bool isNeedStop() {
    MutexGuard gauard(needStopLock);
    return bMCISmeIsStopped;
}

static void setNeedReconnect(bool reconnect=true) {
    MutexGuard gauard(needReconnectLock);
    bMCISmeIsConnected = !reconnect;
    if (reconnect && taskProcessor) taskProcessor->Stop();
}
static bool isNeedReconnect() {
    MutexGuard gauard(needReconnectLock);
    return !bMCISmeIsConnected;
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

static int   unrespondedMessagesSleep = 10;
static int   unrespondedMessagesMax   = 3;
static int   outgoingSpeedMax = 50;

//template <class T=int,T init_v=0,T inc_v=1>
//class TimeSlotCounterEx: public TimeSlotCounter<T, init_v, inc_v>
//{
//public:
//	TimeSlotCounterEx(unsigned int nsec,unsigned int msecres=100): TimeSlotCounter(nsec, msecres){}
//	~TimeSlotCounterEx(){}
//
//  int getUsedSlots(void)
//  {
//    Inc(0);
//    return (last-first+slotsCount)%slotsCount;
//  }
//};

class TrafficControl
{
    static EventMonitor   trafficMonitor; 
    static TimeSlotCounter<int> incoming;
    static TimeSlotCounter<int> outgoing;
	static TimeSlotCounter<int> limitSpeed;
    static bool                 stopped; 

public:

    static void incOutgoing()
    {
		MutexGuard guard(trafficMonitor);
        outgoing.Inc(); 
        if (TrafficControl::stopped) return;

        int out = outgoing.Get();
        int inc = incoming.Get();
        int difference = out-inc;
        
        if (difference >= unrespondedMessagesMax) {
            smsc_log_debug(logger, "wait %d/%d (o=%d, i=%d)", 
                           difference, difference*unrespondedMessagesSleep, out, inc);
            trafficMonitor.wait(difference*unrespondedMessagesSleep);
        } else {
            smsc_log_debug(logger, "nowait");
        }

		uint32_t pause;
		while(limitSpeed.Get() >= outgoingSpeedMax)
		{
			trafficMonitor.wait(10);
			pause+=10;
		}
		limitSpeed.Inc();
		smsc_log_debug(logger, "Speed limiter stoped sending by %d milliseconds", pause);
    };
    static bool getTraffic(int& in, int& out)
    {
        MutexGuard guard(trafficMonitor);
        in = incoming.Get()/10; out = outgoing.Get()/10; 
        return (!TrafficControl::stopped);
    };
    static void incIncoming()
    {
		MutexGuard guard(trafficMonitor);
        incoming.Inc();
        if (TrafficControl::stopped) return;
        if ((outgoing.Get()-incoming.Get()) < unrespondedMessagesMax) {
            trafficMonitor.notifyAll();
        }
    };
    static void stopControl()
    {
        MutexGuard guard(trafficMonitor);
        TrafficControl::stopped = true;
        trafficMonitor.notifyAll();
    };
    static void startControl()
    {
        MutexGuard guard(trafficMonitor);
        TrafficControl::stopped = false;
    };
};

EventMonitor         TrafficControl::trafficMonitor;
TimeSlotCounter<int> TrafficControl::incoming(10, 10);
TimeSlotCounter<int> TrafficControl::outgoing(10, 10);
TimeSlotCounter<int> TrafficControl::limitSpeed(1,10);
bool                 TrafficControl::stopped = false;

class MCISmeConfig : public SmeConfig
{
private:
    
    char *strHost, *strSid, *strPassword, *strSysType, *strOrigAddr;

public:
    
    MCISmeConfig(ConfigView* config)
        throw(ConfigException)
            : SmeConfig(), strHost(0), strSid(0), strPassword(0), 
                strSysType(0), strOrigAddr(0)
    {
        // Mandatory fields
		strHost = config->getString("host", "SMSC host wasn't defined !");
        host = strHost;
        strSid = config->getString("sid", "MCISme id wasn't defined !");
        sid = strSid;
        
        port = config->getInt("port", "SMSC port wasn't defined !");
        timeOut = config->getInt("timeout", "Connect timeout wasn't defined !");
        
        // Optional fields
        try {
            strPassword = config->getString("password", "MCISme password wasn't defined !");
            password = strPassword;
        } catch (ConfigException& exc) { password = ""; strPassword = 0; }
        try {
            strSysType = config->getString("systemType", "MCISme system type wasn't defined !");
            systemType = strSysType;
        } catch (ConfigException& exc) { systemType = ""; strSysType = 0; }
        try {
            strOrigAddr = config->getString("origAddress", "MCISme originating address wasn't defined !");
            origAddr = strOrigAddr;
        } catch (ConfigException& exc) { origAddr = ""; strOrigAddr = 0; }
    };

    virtual ~MCISmeConfig()
    {
        if (strHost) delete strHost;
        if (strSid) delete strSid;
        if (strPassword) delete strPassword;
        if (strSysType) delete strSysType;
        if (strOrigAddr) delete strOrigAddr;
    };
};

class MCISmeMessageSender: public MessageSender
{
private:
    
    TaskProcessor&  processor;
    SmppSession*    session;
    Mutex           sendLock;

public:
    
    MCISmeMessageSender(TaskProcessor& processor, SmppSession* session) 
        : MessageSender(), processor(processor), session(session) {};
    virtual ~MCISmeMessageSender() {
        MutexGuard guard(sendLock);
        session = 0;
    };

    virtual int getSequenceNumber()
    {
        MutexGuard guard(sendLock);
        return (session) ? session->getNextSeq():0;
    }
    virtual bool send(int seqNumber, const Message& message)
    {
        MutexGuard guard(sendLock);
        
        if (!session) {
            smsc_log_error(logger, "Smpp session is undefined for MessageSender");
            return false;
        }
        SmppTransmitter* asyncTransmitter = session->getAsyncTransmitter();
        if (!asyncTransmitter) {
            smsc_log_error(logger, "Smpp transmitter is undefined for MessageSender");
            return false;
        }
        
        if (message.notification) smsc_log_debug(logger, "Sender: sending notification message seqNum=%d for %s",
                                                 seqNumber, message.abonent.c_str());
        else smsc_log_debug(logger, "Sender: %s%s message #%lld seqNum=%d for %s", 
                            (message.cancel) ? "canceling ":"sending", (message.cancel) ? message.smsc_id.c_str():"",
                            message.id, seqNumber, message.abonent.c_str());


        Address oa, da;
        const char* oaStr = processor.getAddress(); // TODO: caller address for notifications
        if (!oaStr || !convertMSISDNStringToAddress(oaStr, oa)) {
            smsc_log_error(logger, "Invalid originating address '%s'", oaStr ? oaStr:"-");
            return false;
        }
        const char* daStr = message.abonent.c_str();
        if (!daStr || !convertMSISDNStringToAddress(daStr, da)) {
            smsc_log_error(logger, "Invalid destination address '%s'", daStr ? daStr:"-");
            return false;
        }
        
        if (message.cancel)
        {
            //smsc_log_debug(logger, "CANCEL SMSC_ID=%s da=%s", message.smsc_id.c_str(), daStr);
            PduCancelSm sm;

            sm.set_messageId(message.smsc_id.c_str());
            sm.get_source().set_typeOfNumber(oa.type);
            sm.get_source().set_numberingPlan(oa.plan);
            sm.get_source().set_value(oa.value);
            sm.get_dest()  .set_typeOfNumber(da.type);
            sm.get_dest()  .set_numberingPlan(da.plan);
            sm.get_dest()  .set_value(da.value);
            
            sm.get_header().set_commandLength(sm.size());
            sm.get_header().set_commandId(SmppCommandSet::CANCEL_SM);
            sm.get_header().set_commandStatus(0);
            sm.get_header().set_sequenceNumber(seqNumber);
            
            asyncTransmitter->sendPdu(&(sm.get_header()));
        }
        else if(message.data_sm)
		{

			const char* out  = message.message.c_str();//"Hello World!!";
            int outLen = message.message.length();//13;
            
            char* msgBuf = 0;
            int msgDataCoding = DataCoding::LATIN1;
            if(hasHighBit(out,outLen))
            {
                int msgLen = outLen*2;
                msgBuf = new char[msgLen];
                ConvertMultibyteToUCS2(out, outLen, (short *)msgBuf, msgLen, CONV_ENCODING_CP1251);
                short* msgBufConv = (short *)msgBuf;
                for (int p=0; p<outLen; p++) msgBufConv[p] = htons(msgBufConv[p]);
                msgDataCoding = DataCoding::UCS2;
                out = msgBuf; outLen = msgLen;
            }

            time_t smsValidityDate = time(NULL) + processor.getDaysValid()*3600*24;
            
            EService svcType;
            strncpy(svcType, processor.getSvcType(), MAX_ESERVICE_TYPE_LENGTH);
            svcType[MAX_ESERVICE_TYPE_LENGTH]='\0';

            PduDataSm	sm;

			sm.get_data().set_serviceType(svcType);
			sm.get_data().get_source().set_typeOfNumber(oa.type);
            sm.get_data().get_source().set_numberingPlan(oa.plan);
            sm.get_data().get_source().set_value(oa.value);
            sm.get_data().get_dest()  .set_typeOfNumber(da.type);
            sm.get_data().get_dest()  .set_numberingPlan(da.plan);
            sm.get_data().get_dest()  .set_value(da.value);
            sm.get_data().set_esmClass(SMSC_TRANSACTION_MSG_MODE); // forward (i.e. transactional)
			sm.get_data().set_dataCoding(msgDataCoding);

            sm.get_optional().set_payloadType(0);
            sm.get_optional().set_messagePayload(out, outLen);
			sm.get_optional().set_setDpf(1);

            sm.get_header().set_commandLength(sm.size());
			sm.get_header().set_commandId(SmppCommandSet::DATA_SM);
            sm.get_header().set_commandStatus(0);
            sm.get_header().set_sequenceNumber(seqNumber);

            if (msgBuf) delete msgBuf;
			//sm.dump(stdout, 1);
			
			smsc_log_debug(logger, "Sending DATA_SM to %s seq_num = %d", daStr, sm.get_header().get_sequenceNumber());
            asyncTransmitter->sendPdu(&(sm.get_header()));
            TrafficControl::incOutgoing();

		}
		else
        {
            const char* out  = message.message.c_str();
            int outLen = message.message.length();
            
            char* msgBuf = 0;
			int msgDataCoding = DataCoding::LATIN1;
            if(hasHighBit(out,outLen))
            {
                int msgLen = outLen*2;
                msgBuf = new char[msgLen];
                ConvertMultibyteToUCS2(out, outLen, (short *)msgBuf, msgLen, CONV_ENCODING_CP1251);
                short* msgBufConv = (short *)msgBuf;
                for (int p=0; p<outLen; p++) msgBufConv[p] = htons(msgBufConv[p]);
                msgDataCoding = DataCoding::UCS2;
                out = msgBuf; outLen = msgLen;
            }

            time_t smsValidityDate = time(NULL) + processor.getDaysValid()*3600*24;
            
            EService svcType;
            strncpy(svcType, processor.getSvcType(), MAX_ESERVICE_TYPE_LENGTH);
            svcType[MAX_ESERVICE_TYPE_LENGTH]='\0';

            PduSubmitSm  sm;
            
            sm.get_message().set_serviceType(svcType);
            sm.get_message().get_source().set_typeOfNumber(oa.type);
            sm.get_message().get_source().set_numberingPlan(oa.plan);
            sm.get_message().get_source().set_value(oa.value);
            sm.get_message().get_dest()  .set_typeOfNumber(da.type);
            sm.get_message().get_dest()  .set_numberingPlan(da.plan);
            sm.get_message().get_dest()  .set_value(da.value);
            sm.get_message().set_esmClass(0); // default mode (not transactional)
            sm.get_message().set_protocolId(processor.getProtocolId());
            sm.get_message().set_priorityFlag(0);
            char timeBuffer[64];
            cTime2SmppTime(smsValidityDate, timeBuffer);
            sm.get_message().set_validityPeriod(timeBuffer);
            cTime2SmppTime(time(NULL)+60*message.attempts, timeBuffer);
            sm.get_message().set_scheduleDeliveryTime(timeBuffer);
            sm.get_message().set_registredDelivery((message.notification) ? 0:1);
            sm.get_message().set_replaceIfPresentFlag(0);

            sm.get_message().set_smDefaultMsgId(0);
            sm.get_message().set_dataCoding(msgDataCoding);
            
            if (outLen > MAX_ALLOWED_MESSAGE_LENGTH)
            {
                if (outLen > MAX_ALLOWED_PAYLOAD_LENGTH) {
                    smsc_log_error(logger, "Message #%lld is too large to send (%d bytes)", 
                                   message.id, outLen);
                    return false;
                }
                sm.get_optional().set_payloadType(0);
                sm.get_optional().set_messagePayload(out, outLen);
            } 
            else sm.get_message().set_shortMessage(out, outLen);
            
            sm.get_header().set_commandLength(sm.size(false));
            sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
            sm.get_header().set_commandStatus(0);
            sm.get_header().set_sequenceNumber(seqNumber);

            if (msgBuf) delete msgBuf;

			smsc_log_debug(logger, "Sending SUBMIT_SM to %s seq_num = %d", daStr, sm.get_header().get_sequenceNumber());
            asyncTransmitter->sendPdu(&(sm.get_header()));
            TrafficControl::incOutgoing();
        }
        return true;
    }
};

class MCISmePduListener: public SmppPduEventListener
{
protected:
    
    TaskProcessor&      processor;
    
    SmppTransmitter*    syncTransmitter;
    SmppTransmitter*    asyncTransmitter;

public:
    
    MCISmePduListener(TaskProcessor& proc) : SmppPduEventListener(),
        processor(proc), syncTransmitter(0), asyncTransmitter(0) {};
    virtual ~MCISmePduListener() {};

    void setSyncTransmitter(SmppTransmitter *transmitter) {
        syncTransmitter = transmitter;
    }
    void setAsyncTransmitter(SmppTransmitter *transmitter) {
        asyncTransmitter = transmitter;
    }
    
    void processDataSmResp(SmppHeader *pdu)
    {
        if (!pdu) return;
		int cmdId	= pdu->get_commandId();
        int seqNum	= pdu->get_sequenceNumber();
        int status	= pdu->get_commandStatus();
        
		TrafficControl::incIncoming();
		processor.invokeProcessDataSmResp(cmdId, status, seqNum);

	}
	void processAlertNotification(SmppHeader *pdu)
	{
        if (!pdu) return;
        PduAlertNotification* alert = (PduAlertNotification*)pdu;
		int cmdId	= pdu->get_commandId();
        int status	= pdu->get_commandStatus();
		int ton = alert->get_esme().get_typeOfNumber();
		int npi = alert->get_esme().get_numberingPlan();
		int ms_availability_status = 0xFF;
		
		if(alert->get_optional().has_msAvailableStatus())
			ms_availability_status = alert->get_optional().get_msAvailableStatus();

		const char* source_addr = alert->get_esme().get_value();
		int source_addr_size = strlen(source_addr);
		
		smsc_log_debug(logger, "MCISme: Recieved Alert Notification: ton=%d, npi=%d, source_addr=%s, addr_size=%d avalible = %X", 
						ton, npi, source_addr, source_addr_size, ms_availability_status);
//		if(0 != ms_availability_status)	return;
        AbntAddr	abnt(source_addr_size, ton, npi, source_addr);
		processor.invokeProcessAlertNotification(cmdId, status, abnt);
	}
    void processDeliverySm(SmppHeader *pdu)
    {
        if (!pdu || !asyncTransmitter) return;
        //bool bNeedResponce = true;

        //SMS sms;
        //fetchSmsFromSmppPdu((PduXSm*)pdu, &sms);
        //bool isReceipt = (sms.hasIntProperty(Tag::SMPP_ESM_CLASS)) ? 
        //    ((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3C) == 0x4) : false;
        //
        //if (isReceipt && ((PduXSm*)pdu)->get_optional().has_receiptedMessageId())
        //{
        //    const char* msgid = ((PduXSm*)pdu)->get_optional().get_receiptedMessageId();
        //    if (msgid && msgid[0] != '\0')
        //    {
        //        bool delivered = false;
        //        bool expired   = false;
        //        bool deleted   = false;
        //        
        //        if (sms.hasIntProperty(Tag::SMPP_MSG_STATE))
        //        {
        //            int msgState = sms.getIntProperty(Tag::SMPP_MSG_STATE);
        //            switch (msgState)
        //            {
        //            case SmppMessageState::DELIVERED:
        //                delivered = true;
        //                break;
        //            case SmppMessageState::EXPIRED:
        //                expired = true;
        //                break;
        //            case SmppMessageState::DELETED:
        //                deleted = true;
        //                break;
        //            case SmppMessageState::ENROUTE:
        //            case SmppMessageState::UNKNOWN:
        //            case SmppMessageState::ACCEPTED:
        //            case SmppMessageState::REJECTED:
        //            case SmppMessageState::UNDELIVERABLE:
        //                break;
        //            default:
        //                smsc_log_warn(logger, "Invalid state=%d received in reciept !", msgState);
        //                break;
        //            }
        //        }
        //    }
        //}
		PduDeliverySmResp smResp;
        smResp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
        smResp.set_messageId("");
        smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        asyncTransmitter->sendDeliverySmResp(smResp);
    }

    void handleEvent(SmppHeader *pdu)
    {

        if (bMCISmeIsConnecting) {
            mciSmeReady.Wait(mciSmeReadyTimeout);
            if (bMCISmeIsConnecting) {
                disposePdu(pdu);
                return;
            }
        }

        switch (pdu->get_commandId())
        {
			case SmppCommandSet::DATA_SM_RESP:
				processDataSmResp(pdu);
				break;
			case SmppCommandSet::ALERT_NOTIFICATION:
				processAlertNotification(pdu);
				break;

				//if(resp->get_optional().has_deliveryFailureReason())
				//	printf("faied = %X\n", resp->get_optional().deliveryFailureReason);
				//else
				//	printf("OK\n");
				//if(resp->get_optional().has_networkErrorCode())
				//	printf("networkErrorCode = %X\n", resp->get_optional().networkErrorCode);
				//else
				//	printf("No networkErrorCode \n");
				//if(resp->get_optional().has_additionalStatusInfoText())
				//	printf("additionalStatusInfoText = %s\n", resp->get_optional().additionalStatusInfoText);
				//else
				//	printf("No additionalStatusInfoText\n");
				//if(resp->get_optional().has_dpfResult())
				//	printf("dpfResult = %X\n", resp->get_optional().dpfResult);
				//else
				//	printf("No dpfResult\n");
				//break;
				//resp->dump(stdout, 1);

			case SmppCommandSet::DELIVERY_SM:
				smsc_log_debug(logger, "Received DELIVERY_SM seq_num = %d", pdu->get_sequenceNumber());	
				processDeliverySm(pdu);
				break;
			case SmppCommandSet::CANCEL_SM_RESP:
				smsc_log_debug(logger, "Received CANCEL_SM_RESP seq_num = %d", pdu->get_sequenceNumber());
				break;
			case SmppCommandSet::SUBMIT_SM_RESP:
				TrafficControl::incIncoming();
				smsc_log_debug(logger, "SUBMIT_SM_RESP seq_num = %d", pdu->get_sequenceNumber());
				break;
			case SmppCommandSet::ENQUIRE_LINK: case SmppCommandSet::ENQUIRE_LINK_RESP:
				break;
			default:
				smsc_log_debug(logger, "Received unsupported Pdu !");
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

extern "C" void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT)
    {
        smsc_log_info(logger, "Shutting down by signal...");
        if (bAdminListenerInited) adminListener->shutdown();
        setNeedStop(true);
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

class MCISmeAdministrator : public MCISmeAdmin
{
private:
    
    AdminInterface& admin;

public:
    
    MCISmeAdministrator(AdminInterface& _admin) : MCISmeAdmin(), admin(_admin) {};
    virtual ~MCISmeAdministrator() {};

    virtual void flushStatistics() {
        admin.flushStatistics();
    };
    virtual EventsStat  getStatistics() {
        return admin.getStatistics();
    };
    virtual RuntimeStat getRuntimeStatistics()
    {
        int inSpeed = 0; int outSpeed = 0;
        int activeTasks  = admin.getActiveTasksCount();
        int inQueueSize  = admin.getInQueueSize();
        int outQueueSize = admin.getOutQueueSize();
        TrafficControl::getTraffic(inSpeed, outSpeed);
        return RuntimeStat(activeTasks, inQueueSize, outQueueSize, inSpeed, outSpeed);
    };
};

int main(void)
{
    Logger::Init();
    logger = Logger::getInstance("smsc.mcisme.MCISme");
    
    atexit(atExitHandler);
 //   clearSignalMask();

	//shutdownThread.Start();


//	ProfilesStorage::Open();

//	ProfilesStorage* ps = ProfilesStorage::GetInstance();

	//AbntAddr abnt("+1234567890");
	//AbonentProfile prof;
	//
	//prof.eventMask = 17;
	//prof.inform = true;
	//prof.notify = false;
	//prof.informTemplateId = 18;
	//prof.notifyTemplateId = -1;

	//printf("prof.eventMask = %d\n", prof.eventMask);
	//printf("prof.inform = %d\n", prof.inform);
	//printf("prof.notify = %d\n", prof.notify);
	//printf("prof.informTemplateId = %d\n", prof.informTemplateId);
	//printf("prof.notifyTemplateId = %d\n", prof.notifyTemplateId);

	//ps->Set(abnt, prof);

	//prof.eventMask = 0;
	//prof.inform = 0;
	//prof.notify = 0;
	//prof.informTemplateId = 0;
	//prof.notifyTemplateId = 0;

	//printf("prof.eventMask = %d\n", prof.eventMask);
	//printf("prof.inform = %d\n", prof.inform);
	//printf("prof.notify = %d\n", prof.notify);
	//printf("prof.informTemplateId = %d\n", prof.informTemplateId);
	//printf("prof.notifyTemplateId = %d\n", prof.notifyTemplateId);

	//ps->Get(abnt, prof);

	//printf("prof.eventMask = %d\n", prof.eventMask);
	//printf("prof.inform = %d\n", prof.inform);
	//printf("prof.notify = %d\n", prof.notify);
	//printf("prof.informTemplateId = %d\n", prof.informTemplateId);
	//printf("prof.notifyTemplateId = %d\n", prof.notifyTemplateId);


	//uint32_t val;
	//const char* key1 = "1234";
	//ps->Set(key1, 10);
	//ps->Get(key1, val);
	//printf("val=%d\n", val);
	//ps->Set(key1, 20);
	//ps->Get(key1, val);
	//printf("val=%d\n", val);

//	exit(0);
    std::auto_ptr<ServiceSocketListener> adml(new ServiceSocketListener());
    adminListener = adml.get();

    int resultCode = 0;
    try 
    {
        smsc_log_info(logger, getStrVersion());
        Manager::init("./conf/config.xml");
        Manager& manager = Manager::getInstance();

//      ConfigView dsConfig(manager, "StartupLoader");
//      DataSourceLoader::loadup(&dsConfig);
        
        ConfigView tpConfig(manager, "MCISme");
        try { unrespondedMessagesMax = tpConfig.getInt("unrespondedMessagesMax"); } catch (...) {};
        if (unrespondedMessagesMax <= 0) {
            unrespondedMessagesMax = 1;
            smsc_log_warn(logger, "Parameter 'unrespondedMessagesMax' value is invalid. Using default %d",
                          unrespondedMessagesMax);
        }
		if (unrespondedMessagesMax > 500) {
            smsc_log_warn(logger, "Parameter 'unrespondedMessagesMax' value '%d' is too big. "
                          "The preffered max value is 500", unrespondedMessagesMax);
        }
        try { unrespondedMessagesSleep = tpConfig.getInt("unrespondedMessagesSleep"); } catch (...) {};
        if (unrespondedMessagesSleep <= 0) {
            unrespondedMessagesSleep = 100;
            smsc_log_warn(logger, "'unrespondedMessagesSleep' value is invalid. Using default %dms",
                          unrespondedMessagesSleep);
        }
        if (unrespondedMessagesSleep > 500) {
            smsc_log_warn(logger, "Parameter 'unrespondedMessagesSleep' value '%d' is too big. "
                          "The preffered max value is 500ms", unrespondedMessagesSleep);
        }

        try { outgoingSpeedMax = tpConfig.getInt("outgoingSpeedMax"); } catch (...) {};
        if (outgoingSpeedMax <= 0) {
            outgoingSpeedMax = 50;
            smsc_log_warn(logger, "'outgoingSpeedMax' value is invalid. Using default %d messages per second",
                          outgoingSpeedMax);
        }
        if (outgoingSpeedMax > 400) {
            smsc_log_warn(logger, "Parameter 'unrespondedMessagesSleep' value '%d' is too big. "
                          "The preffered max value is 400 messages per second", outgoingSpeedMax);
        }

		std::auto_ptr<ConfigView> profStorCfgGuard(tpConfig.getSubConfig("ProfileStorage"));
        ConfigView* profStorCfg = profStorCfgGuard.get();
        string locationProfStor;
        try {locationProfStor = profStorCfg->getString("location"); } catch (...){locationProfStor="./";
            smsc_log_warn(logger, "Parameter <MCISme.ProfileStorage.location> missed. Default value is './'");}
		smsc_log_warn(logger, "Profile Storage location is '%s'", locationProfStor.c_str());
		ProfilesStorage::Open(locationProfStor);
		smsc_log_warn(logger, "Profile Storage is opened");

		ProfilesStorageServer	pss;
        try 
		{
			int portProfStor = profStorCfg->getInt("port");
			if(portProfStor > 0)
			{
				pss.init("0.0.0.0", portProfStor);
				pss.Start();
				smsc_log_warn(logger, "ProfilesStorageServer started on port %d.", portProfStor);
			}
			else
				smsc_log_warn(logger, "ProfilesStorageServer NOT started - illegal port %d.", portProfStor);
		} 
		catch (...)
		{
			smsc_log_warn(logger, "Parameter <MCISme.ProfileStorage.port> missed. ProfilesStorageServer NOT started");
		}
		
		ConfigView adminConfig(manager, "MCISme.Admin");
		smsc_log_debug(logger, "%s %d", adminConfig.getString("host"), adminConfig.getInt("port"));
	    adminListener->init(adminConfig.getString("host"), adminConfig.getInt("port"));               
        bAdminListenerInited = true;
        
		TaskProcessor processor(&tpConfig);

        taskProcessor = &processor;
        
        MCISmeAdministrator adminHandler(processor);
        MCISmeComponent admin(adminHandler);                   
        ComponentManager::registerComponent(&admin); 
        adminListener->Start();
       
        ConfigView smscConfig(manager, "MCISme.SMSC");
        MCISmeConfig cfg(&smscConfig);
		
		while (!isNeedStop())
        {
            MCISmePduListener       listener(processor);
            SmppSession             session(cfg, &listener);
            MCISmeMessageSender     sender(processor, &session);
            
            smsc_log_info(logger, "Connecting to SMSC ... ");
            try
            {
                listener.setSyncTransmitter(session.getSyncTransmitter());
                listener.setAsyncTransmitter(session.getAsyncTransmitter());
                
                bMCISmeIsConnecting = true;
                mciSmeReady.Wait(0);
                TrafficControl::startControl();
                setNeedReconnect(false);
                session.connect();
                processor.assignMessageSender(&sender);
                processor.Start();
                bMCISmeIsConnecting = false;
                mciSmeReady.Signal();
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what(); 
                smsc_log_error(logger, "Connect to SMSC failed. Cause: %s", (msg) ? msg:"unknown");
                bMCISmeIsConnecting = false;
                setNeedReconnect(true);
                // ??? if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw exc;
                sleep(cfg.timeOut);
                session.close();
                continue;
            }
            smsc_log_info(logger, "Connected.");
            smsc_log_info(logger, "Running messages send loop...");
            setShutdownHandler();
			processor.Run();
            clearSignalMask();
            smsc_log_info(logger, "Message send loop exited");
            
            smsc_log_info(logger, "Disconnecting from SMSC ...");
            TrafficControl::stopControl();
            processor.assignMessageSender(0);
            session.close();
        }
        taskProcessor = 0;
    }
    catch (SmppConnectException& exc) {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            smsc_log_error(logger, "Failed to bind MCISme. Exiting");
        resultCode = -1;
    }
    catch (ConfigException& exc) {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "Top level Exception: %s Exiting", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) {
        smsc_log_error(logger, "Top level exception: %s Exiting", exc.what());
        resultCode = -4;
    }
    catch (...) {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting");
        resultCode = -5;
    }
    
    if (bAdminListenerInited)
    {
        adminListener->shutdown();
        adminListener->WaitFor();
    }

//    DataSourceLoader::unload();
    //ProfilesStorage::Close();
    smsc_log_debug(logger, "Exited !!!");
    shutdownThread.Stop();
    return resultCode;
}
