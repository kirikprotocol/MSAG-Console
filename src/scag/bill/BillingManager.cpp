/* "$Id$" */
#include "BillingManager.h"
#include <scag/util/singleton/Singleton.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/threads/Thread.hpp>
#include <core/buffers/IntHash.hpp>

#include "scag/exc/SCAGExceptions.h"
#include "BillingManagerWrapper.h"
#include "scag/config/ConfigListener.h"
#include "scag/stat/Statistics.h"


namespace scag { namespace bill {

using namespace smsc::core::threads;
using namespace scag::util::singleton;
using namespace scag::exceptions;
using namespace scag::stat;

using smsc::logger::Logger;

class BillingManagerImpl : public BillingManager, public Thread, public BillingManagerWrapper, public ConfigListener
{
    struct BillTransaction
    {
        TransactionStatus status;
        TariffRec tariffRec;
        #ifdef MSAG_INMAN_BILL
        SPckChargeSms ChargeOperation;
        #endif
        EventMonitor eventMonitor;
        int billId;
        SACC_BILLING_INFO_EVENT_t billEvent;
        BillTransaction() : status(TRANSACTION_NOT_STARTED) { }
    };

    IntHash <BillTransaction *> BillTransactionHash;

    Event connectEvent;
    Mutex stopLock;
    Mutex inUseLock;
    Event exitEvent;

    bool m_bStarted;

    int m_lastBillId;
    int m_TimeOut;

    bool m_Connected;

    void Stop();
    bool isStarted();

    InfrastructureImpl infrastruct;

    #ifdef MSAG_INMAN_BILL
    void fillChargeSms(smsc::inman::interaction::ChargeSms& op, BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec);
    #endif /* MSAG_INMAN_BILL */

    void ProcessSaccEvent(BillingTransactionEvent billingTransactionEvent, BillTransaction * billTransaction);

    void modifyBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, SACC_BILLING_INFO_EVENT_t& ev);
    void makeBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, TariffRec& tariffRec, BillingInfoStruct& billingInfo, SACC_BILLING_INFO_EVENT_t& ev);


    void ClearTransactions()
    {
        int key;
        BillTransaction * value;

        for (IntHash <BillTransaction *>::Iterator it = BillTransactionHash.First(); it.Next(key, value);)
        {
            delete value;
        }

        BillTransactionHash.Empty();
    }
public:
    void init(BillingManagerConfig& cfg);

    virtual int Execute();
    virtual void Start();

    #ifdef MSAG_INMAN_BILL
    virtual void onChargeSmsResult(ChargeSmsResult* result, CsBillingHdr_dlg * hdr);
    #endif

    virtual int Open(BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec);
    virtual void Commit(int billId);
    virtual void Rollback(int billId);



    virtual Infrastructure& getInfrastructure() { return infrastruct; };


    void configChanged();

    BillingManagerImpl() :
        m_bStarted(false),
        m_lastBillId(0), ConfigListener(BILLMAN_CFG) {}

    ~BillingManagerImpl()
    {
        Stop();
        ClearTransactions();
    }

};

static bool  bBillingManagerInited = false;
static Mutex initBillingManagerLock;

inline unsigned GetLongevity(BillingManager*) { return 5; }
typedef SingletonHolder<BillingManagerImpl> SingleBM;

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void BillingManager::Init(BillingManagerConfig& cfg)
{
    if (!bBillingManagerInited)
    {
        MutexGuard guard(initBillingManagerLock);
        if (!bBillingManagerInited) {
            BillingManagerImpl& bm = SingleBM::Instance();
            bm.init(cfg); bm.Start();
            bBillingManagerInited = true;
        }
    }
}

BillingManager& BillingManager::Instance()
{
    if (!bBillingManagerInited)
    {
        MutexGuard guard(initBillingManagerLock);
        if (!bBillingManagerInited)
            throw SCAGException("BillingManager not inited!");
    }
    return SingleBM::Instance();
}

void BillingManagerImpl::Stop()
{
    MutexGuard guard(stopLock);

    if (m_bStarted)
    {
        m_bStarted = false;
        connectEvent.Signal();

        exitEvent.Wait();
    }
    smsc_log_info(logger,"BillingManager::stop");
}

bool BillingManagerImpl::isStarted()
{
    return m_bStarted;
}

void BillingManagerImpl::init(BillingManagerConfig& cfg)
{
    MutexGuard guard(inUseLock);
    smsc_log_info(logger,"BillingManager start initing...");

    if (cfg.MaxThreads < 1) throw SCAGException("BillingManager: cant start service with %d allowed threads", cfg.MaxThreads);

    m_TimeOut = cfg.BillingTimeOut;

    InitConnection(cfg.BillingHost, cfg.BillingPort);


    #ifdef MSAG_INMAN_BILL
    smsc_log_info(logger,"BillingManager connecting to host '%s', port %d", cfg.BillingHost.c_str(),cfg.BillingPort);
    m_Connected = Reconnect();
    #else
    smsc_log_info(logger,"BillingManager skip connection (fake connection initialized)");
    m_Connected = true;
    #endif

    infrastruct.init(cfg.cfg_dir);
    smsc_log_info(logger,"BillingManager inited...");
}

void BillingManagerImpl::configChanged()
{
    MutexGuard guard(inUseLock);

    Stop();
    init(ConfigManager::Instance().getBillManConfig());
    Start();
}

int BillingManagerImpl::Execute()
{
    smsc_log_info(logger,"BillingManager::start executing");

    while (isStarted())
    {
        try
        {
            if (m_Connected)
            {
                #ifdef MSAG_INMAN_BILL
                try
                {
                    receiveCommand();
                } catch (SCAGException& e)
                {
                    smsc_log_error(logger, "BillingManager error: Cannot receive command. Details: %s", e.what());
                    m_Connected = false;
                }
                #endif
                connectEvent.Wait(100);
            } else
            {
                m_Connected = Reconnect();
                if (!m_Connected) connectEvent.Wait(m_TimeOut*1000);
            }

        } catch (SCAGException& e)
        {
            smsc_log_error(logger, "BillingManager error: %s", e.what());
            m_Connected = false;
        }
    }

    smsc_log_info(logger,"BillingManager::stop executing");
    exitEvent.Signal();
    return 0;
}

void BillingManagerImpl::Start()
{
    MutexGuard guard(stopLock);
    if (!m_bStarted)
    {
        m_bStarted = true;
        Thread::Start();
    }
}

void BillingManagerImpl::ProcessSaccEvent(BillingTransactionEvent billingTransactionEvent, BillTransaction * billTransaction)
{
    MutexGuard guard(inUseLock);

    Statistics& statistics = Statistics::Instance();

    switch (billTransaction->status)
    {
    case TRANSACTION_INVALID:
        modifyBillEvent(billingTransactionEvent, INVALID_TRANSACTION, billTransaction->billEvent);
        statistics.registerSaccEvent(billTransaction->billEvent);

        BillTransactionHash.Delete(billTransaction->billId);
        delete billTransaction;
        throw SCAGException("billing transaction invalid");
        break;

    case TRANSACTION_NOT_STARTED:
        modifyBillEvent(billingTransactionEvent, REJECTED_BY_SERVER, billTransaction->billEvent);
        statistics.registerSaccEvent(billTransaction->billEvent);

        BillTransactionHash.Delete(billTransaction->billId);
        delete billTransaction;
        throw SCAGException("billing transaction deny");
        break;

    case TRANSACTION_WAIT_ANSWER:
        modifyBillEvent(billingTransactionEvent, SERVER_NOT_RESPONSE, billTransaction->billEvent);
        statistics.registerSaccEvent(billTransaction->billEvent);

        BillTransactionHash.Delete(billTransaction->billId);
        delete billTransaction;
        throw SCAGException("billing transaction time out");
        break;

    default:
        modifyBillEvent(billingTransactionEvent, COMMAND_SUCCESSFULL, billTransaction->billEvent);
        statistics.registerSaccEvent(billTransaction->billEvent);
    }

}

int BillingManagerImpl::Open(BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec)
{
    int billId;
    SACC_BILLING_INFO_EVENT_t ev;


    inUseLock.Lock();
    billId = ++m_lastBillId;
    inUseLock.Unlock();

    BillTransaction * billTransaction = new BillTransaction();
    billTransaction->tariffRec = tariffRec;
    billTransaction->billId = billId;
    makeBillEvent(TRANSACTION_OPEN, COMMAND_SUCCESSFULL, tariffRec, billingInfoStruct, billTransaction->billEvent);


    #ifdef MSAG_INMAN_BILL
    //SPckChargeSms pck;
    fillChargeSms(billTransaction->ChargeOperation.Cmd(), billingInfoStruct, tariffRec);
    billTransaction->ChargeOperation.Hdr().dlgId = billId;
    //billTransaction->ChargeOperation = pck;
    billTransaction->status = TRANSACTION_WAIT_ANSWER;
    #else
    billTransaction->status = TRANSACTION_VALID;
    #endif

    inUseLock.Lock();
    BillTransactionHash.Insert(billId, billTransaction);
    inUseLock.Unlock();


    #ifdef MSAG_INMAN_BILL
    sendCommand(billTransaction->ChargeOperation);

    billTransaction->eventMonitor.wait(1000);
    #endif

    if (billTransaction->status == TRANSACTION_WAIT_ANSWER)
    {
        inUseLock.Lock();
        delete billTransaction;
        BillTransactionHash.Delete(billId);
        inUseLock.Unlock();

        throw SCAGException("Transaction billId=%d timed out", billId);
    }

    if (billTransaction->status == TRANSACTION_INVALID)
    {
        inUseLock.Lock();
        delete billTransaction;
        BillTransactionHash.Delete(billId);
        inUseLock.Unlock();

        throw SCAGException("Transaction billId=%d: charging is impossible", billId);
    }



    ProcessSaccEvent(TRANSACTION_OPEN, billTransaction);

    #ifdef MSAG_INMAN_BILL

    SPckDeliverySmsResult opRes;
    opRes.Hdr().dlgId = billId;
    opRes.Cmd().setResultValue(1);

    billTransaction->status = TRANSACTION_WAIT_ANSWER;
    sendCommand(opRes);
    #endif

    return billId;
}


void BillingManagerImpl::Commit(int billId)
{
    {
      MutexGuard mg(inUseLock);
      BillTransaction ** pBillTransactionPtrPtr = BillTransactionHash.GetPtr(billId);
      if (!pBillTransactionPtrPtr) throw SCAGException("Cannot find transaction for billId=%d", billId);
      BillTransaction * pBillTransaction=*pBillTransactionPtrPtr;
    }


    #ifdef MSAG_INMAN_BILL
    //(*pBillTransactionPtr)->ChargeOperation.Hdr().dlgId = billId;
    //SPckChargeSms pck;
    sendCommand(pBillTransaction->ChargeOperation);
    //sendCommand(pck);
    #else
    pBillTransaction->status = TRANSACTION_VALID;
    #endif

    pBillTransaction->eventMonitor.wait(1000);

    ProcessSaccEvent(TRANSACTION_COMMITED, pBillTransaction);

    smsc_log_debug(logger, "Commiting billId=%d...", billId);

    int status = pBillTransaction->status;

    inUseLock.Lock();
    delete pBillTransaction;
    BillTransactionHash.Delete(billId);
    inUseLock.Unlock();


    if (status == TRANSACTION_WAIT_ANSWER)
        throw SCAGException("Transaction billId=%d timed out.", billId);

    if (status == TRANSACTION_INVALID)
        throw SCAGException("Transaction billId=%d invalid.", billId);


    if (status == TRANSACTION_VALID)
    {
        #ifdef MSAG_INMAN_BILL
        SPckDeliverySmsResult op;
        op.Hdr().dlgId = billId;
        sendCommand(op);
        #endif
    }
}



void BillingManagerImpl::Rollback(int billId)
{

    {
      MutexGuard mg(inUseLock);
      BillTransaction ** pBillTransactionPtrPtr = BillTransactionHash.GetPtr(billId);
      if (!pBillTransactionPtrPtr) throw SCAGException("Cannot find transaction for billId=%d", billId);
      BillTransaction * pBillTransaction=*pBillTransactionPtrPtr;
    }

    if (pBillTransaction->status == TRANSACTION_VALID)
    {
        #ifdef MSAG_INMAN_BILL
        SPckDeliverySmsResult op;
        op.Hdr().dlgId = billId;
        op.Cmd().setResultValue(2);
        sendCommand(op);
        #endif
    }

    ProcessSaccEvent(TRANSACTION_CALL_ROLLBACK, pBillTransaction);

    inUseLock.Lock();
    delete pBillTransaction;
    BillTransactionHash.Delete(billId);
    inUseLock.Unlock();

    smsc_log_debug(logger, "Transaction rolled back (billId=%d)", billId);

}


void BillingManagerImpl::modifyBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, SACC_BILLING_INFO_EVENT_t& ev)
{
    ev.Header.cCommandId = billCommand;
    ev.Header.sCommandStatus = commandStatus;

    timeval tv;
    gettimeofday(&tv,0);

    ev.Header.lDateTime = (uint64_t)tv.tv_sec*1000 + (tv.tv_usec / 1000);
}


void BillingManagerImpl::makeBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, TariffRec& tariffRec, BillingInfoStruct& billingInfo, SACC_BILLING_INFO_EVENT_t& ev)
{
    ev.Header.cCommandId = billCommand;

    ev.Header.cProtocolId = billingInfo.protocol;
    ev.Header.iServiceId = billingInfo.serviceId;

    ev.Header.iServiceProviderId = billingInfo.providerId;

    timeval tv;
    gettimeofday(&tv,0);

    ev.Header.lDateTime = (uint64_t)tv.tv_sec*1000 + (tv.tv_usec / 1000);

    ev.Header.pAbonentNumber = billingInfo.AbonentNumber;
    ev.Header.sCommandStatus = commandStatus;
    ev.Header.iOperatorId = billingInfo.operatorId;
    ev.iPriceCatId = tariffRec.CategoryId;


    ev.fBillingSumm = tariffRec.Price;
    ev.iMediaResourceType = tariffRec.MediaTypeId;

    ev.pBillingCurrency = tariffRec.Currency;

    char buff[128];
    sprintf(buff,"%s/%ld%d",billingInfo.AbonentNumber.c_str(), billingInfo.SessionBornMicrotime.tv_sec, billingInfo.SessionBornMicrotime.tv_usec / 1000);
    ev.pSessionKey.append(buff);

}



#ifdef MSAG_INMAN_BILL
void BillingManagerImpl::onChargeSmsResult(ChargeSmsResult* result, CsBillingHdr_dlg * hdr)
{
    if (!result) return;

    MutexGuard guard(inUseLock);

    int billId = hdr->dlgId;

    BillTransaction ** pBillTransactionPtr = BillTransactionHash.GetPtr(billId);

    if (!pBillTransactionPtr)
    {
        //TODO: do what we must to do
        return;
    }

    if( result->GetValue() == ChargeSmsResult::CHARGING_POSSIBLE )
        (*pBillTransactionPtr)->status = TRANSACTION_VALID;
    else
        (*pBillTransactionPtr)->status = TRANSACTION_INVALID;

    (*pBillTransactionPtr)->eventMonitor.notify();

}

void BillingManagerImpl::fillChargeSms(smsc::inman::interaction::ChargeSms& op, BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec)
{
    char buff[128];
    sprintf(buff,"%d", tariffRec.ServiceNumber);
    std::string str(buff);

    op.setDestinationSubscriberNumber(str);
    op.setCallingPartyNumber(billingInfoStruct.AbonentNumber);
    op.setServiceId(billingInfoStruct.serviceId);
    op.setUserMsgRef(billingInfoStruct.msgRef);

    smsc_log_debug(logger, "***** SN=%s, CPN=%s, SID=%d", str.c_str(), billingInfoStruct.AbonentNumber.c_str(), billingInfoStruct.serviceId);

    /*
    char buff[128];
    sprintf(buff,"%d", tariffRec.ServiceNumber);
    std::string str(buff);
    //op.setDestinationSubscriberNumber(sms.getDealiasedDestinationAddress().toString());
    if (command->cmdid == DELIVERY)
    {
        op.setDestinationSubscriberNumber(str);
        op.setCallingPartyNumber(sms.getOriginatingAddress().toString());
    } else if (command->cmdid == SUBMIT)
    {
        op.setDestinationSubscriberNumber(str);
        op.setCallingPartyNumber(sms.getDestinationAddress().toString());
    }

    op.setCallingIMSI(sms.getOriginatingDescriptor().imsi);
    //op.setSMSCAddress(INManComm::scAddr.toString());
    op.setSubmitTimeTZ(sms.getSubmitTime());
    op.setTPShortMessageSpecificInfo(0x11);
    op.setTPProtocolIdentifier(sms.getIntProperty(Tag::SMPP_PROTOCOL_ID));
    op.setTPDataCodingScheme(sms.getIntProperty(Tag::SMPP_DATA_CODING));
    op.setTPValidityPeriod(sms.getValidTime()-time(NULL));
    op.setLocationInformationMSC(sms.getOriginatingDescriptor().msc);
    op.setCallingSMEid(sms.getSourceSmeId());
    op.setRouteId(sms.getRouteId());

    op.setServiceId(command.getServiceId());

    op.setUserMsgRef(sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)?sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE):-1);
    //op.setMsgId(id);
    op.setServiceOp(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)?sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP):-1);
    op.setPartsNum(sms.getIntProperty(Tag::SMSC_ORIGINALPARTSNUM));

    if(sms.hasBinProperty(Tag::SMPP_SHORT_MESSAGE))
    {
        unsigned len;
        sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
        op.setMsgLength(len);
    }else
    {
        unsigned len;
        sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
        op.setMsgLength(len);
    } */
}

#endif /* MSAG_INMAN_BILL */

}}
