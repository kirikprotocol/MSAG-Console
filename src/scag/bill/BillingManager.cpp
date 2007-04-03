/* "$Id$" */
#include "BillingManager.h"
#include <scag/util/singleton/Singleton.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/threads/Thread.hpp>
#include <core/buffers/IntHash.hpp>

#ifdef MSAG_INMAN_BILL
#include "inman/interaction/connect.hpp"
#include "inman/interaction/MsgBilling.hpp"
#include "inman/inman.hpp"
#endif


#include "scag/exc/SCAGExceptions.h"
#include "scag/config/ConfigListener.h"
#include "scag/stat/Statistics.h"


namespace scag { namespace bill {

#ifdef MSAG_INMAN_BILL
using namespace smsc::inman::interaction;
#endif

using namespace smsc::core::threads;
using smsc::core::network::Socket;
using namespace scag::util::singleton;
using namespace scag::exceptions;
using namespace scag::stat;

using smsc::logger::Logger;

class BillingManagerImpl : public BillingManager, public Thread, public ConfigListener
#ifdef MSAG_INMAN_BILL
    , public SMSCBillingHandlerITF, public ConnectListenerITF
#endif
{
    struct BillTransaction
    {
        TransactionStatus status;
        TariffRec tariffRec;
        #ifdef MSAG_INMAN_BILL
        SPckChargeSms ChargeOperation;
        #endif
        EventMonitor eventMonitor;
        unsigned int billId;
        SaccBillingInfoEvent billEvent;
        BillingInfoStruct billingInfoStruct;
        BillTransaction() : status(TRANSACTION_NOT_STARTED) { }
    };

    Logger * logger;

    IntHash <BillTransaction *> BillTransactionHash;

    Event connectEvent;
    Mutex stopLock;
    Mutex inUseLock;
    Event exitEvent;

    bool m_bStarted;

    unsigned int m_lastBillId;
    int m_ReconnTimeout;

    bool m_Connected;
    int m_Port;
    std::string m_Host;

    void Stop();
    bool isStarted() { return m_bStarted; };

    InfrastructureImpl infrastruct;

    #ifdef MSAG_INMAN_BILL
    Socket * socket;
    Connect * pipe;
    virtual void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd);
    virtual void onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc);
    void sendCommand(INPPacketAC& op) { pipe->sendPck(&op); }

    void fillChargeSms(smsc::inman::interaction::ChargeSms& op, BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec);
    virtual void onChargeSmsResult(ChargeSmsResult* result, CsBillingHdr_dlg * hdr);
    #endif /* MSAG_INMAN_BILL */

    void ProcessSaccEvent(BillingTransactionEvent billingTransactionEvent, BillTransaction * billTransaction);

    void modifyBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, SaccBillingInfoEvent& ev);
    void makeBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, TariffRec& tariffRec, BillingInfoStruct& billingInfo, SaccBillingInfoEvent& ev);

    BillTransaction* getBillTransaction(uint32_t billId);
    void deleteBillTransaction(BillTransaction * billTransaction);
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
    void logEvent(const char *type, bool success, BillingInfoStruct& b, int billID);

    bool Reconnect();

    void InitConnection(std::string& host, int port)
    {
        #ifdef MSAG_INMAN_BILL
        m_Host = host;
        m_Port = port;
        #endif
    }

public:
    void init(BillingManagerConfig& cfg);

    virtual int Execute();
    virtual void Start();

    virtual unsigned int Open(BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec);
    virtual void Commit(int billId);
    virtual void Rollback(int billId, bool timeout);
    virtual void Info(int billId, BillingInfoStruct& bis, TariffRec& tariffRec);

    virtual Infrastructure& getInfrastructure() { return infrastruct; };

    void configChanged();

    BillingManagerImpl() :
        m_bStarted(false),
        m_lastBillId(0), ConfigListener(BILLMAN_CFG), logger(Logger::getInstance("bill.man"))
        #ifdef MSAG_INMAN_BILL
        , socket(0), pipe(0)
        #endif
    {
        #ifdef MSAG_INMAN_BILL
        socket = new Socket();

        pipe = new Connect(socket, INPSerializer::getInstance(), logger);
        pipe->addListener(this);
        #endif
    }

    ~BillingManagerImpl()
    {
        #ifdef MSAG_INMAN_BILL
        if (pipe) delete pipe;
        #endif
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

void BillingManagerImpl::init(BillingManagerConfig& cfg)
{
    MutexGuard guard(inUseLock);
    smsc_log_info(logger,"BillingManager start initing...");

    if (cfg.MaxThreads < 1) throw SCAGException("BillingManager: cant start service with %d allowed threads", cfg.MaxThreads);

    m_ReconnTimeout = cfg.BillingTimeOut;

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

void BillingManagerImpl::logEvent(const char *tp, bool success, BillingInfoStruct& b, int billID)
{
    smsc_log_info(logger, "bill %s: %s billId=%d, abonent=%s, opId=%d, sId=%d, providerId=%d",
        tp, success ? "success" : "failed", billID, b.AbonentNumber.c_str(), b.operatorId, b.serviceId, b.providerId);
}

void BillingManagerImpl::deleteBillTransaction(BillTransaction * billTransaction)
{
    MutexGuard mg(inUseLock);
    BillTransactionHash.Delete(billTransaction->billId);
    delete billTransaction;
}

BillingManagerImpl::BillTransaction* BillingManagerImpl::getBillTransaction(uint32_t billId)
{
    MutexGuard mg(inUseLock);
    BillTransaction** pp = BillTransactionHash.GetPtr(billId);
    if (!pp || !*pp) throw SCAGException("Cannot find transaction for billId=%d", billId);
    return *pp;
}

unsigned int BillingManagerImpl::Open(BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec)
{
    unsigned int billId;
    SaccBillingInfoEvent ev;

    inUseLock.Lock();
    billId = ++m_lastBillId;
    inUseLock.Unlock();

    BillTransaction * billTransaction = new BillTransaction();
    billTransaction->tariffRec = tariffRec;
    billTransaction->billingInfoStruct = billingInfoStruct;
    billTransaction->billId = billId;
    makeBillEvent(TRANSACTION_OPEN, COMMAND_SUCCESSFULL, tariffRec, billingInfoStruct, billTransaction->billEvent);

    #ifdef MSAG_INMAN_BILL
    if(tariffRec.billType != scag::bill::infrastruct::INMAN)
        billTransaction->status = TRANSACTION_VALID;
    else
    {
        //SPckChargeSms pck;
        fillChargeSms(billTransaction->ChargeOperation.Cmd(), billingInfoStruct, tariffRec);
        billTransaction->ChargeOperation.Hdr().dlgId = billId;
        //billTransaction->ChargeOperation = pck;
        billTransaction->status = TRANSACTION_WAIT_ANSWER;
    }
    #else
        billTransaction->status = TRANSACTION_VALID;
    #endif

    inUseLock.Lock();
    BillTransactionHash.Insert(billId, billTransaction);
    inUseLock.Unlock();

    MutexGuard mg(billTransaction->eventMonitor);

    #ifdef MSAG_INMAN_BILL
    if(tariffRec.billType == scag::bill::infrastruct::INMAN)
    {
        sendCommand(billTransaction->ChargeOperation);
        billTransaction->eventMonitor.wait(1000);
    }

    if (billTransaction->status == TRANSACTION_WAIT_ANSWER)
    {
        deleteBillTransaction(billTransaction);
        logEvent("open", false, billingInfoStruct, billId);
        throw SCAGException("Transaction billId=%d timed out", billId);
    }

    if (billTransaction->status == TRANSACTION_INVALID)
    {
        deleteBillTransaction(billTransaction);
        logEvent("open", false, billingInfoStruct, billId);
        throw SCAGException("Transaction billId=%d: charging is impossible", billId);
    }
    #endif

    ProcessSaccEvent(TRANSACTION_OPEN, billTransaction);

    #ifdef MSAG_INMAN_BILL
    if(tariffRec.billType == scag::bill::infrastruct::INMAN)
    {
        SPckDeliverySmsResult opRes;
        opRes.Hdr().dlgId = billId;
        opRes.Cmd().setResultValue(1);
        opRes.Cmd().setFinal(false); // To skip CDR creation

        billTransaction->status = TRANSACTION_WAIT_ANSWER;
        sendCommand(opRes);
    }
    #endif

    logEvent("open", true, billingInfoStruct, billId);
    return billId;
}


void BillingManagerImpl::Commit(int billId)
{
    BillTransaction * pBillTransaction = getBillTransaction(billId);

    #ifdef MSAG_INMAN_BILL
    if(pBillTransaction->tariffRec.billType == scag::bill::infrastruct::INMAN)
    {
        //(*pBillTransactionPtr)->ChargeOperation.Hdr().dlgId = billId;
        //SPckChargeSms pck;
        sendCommand(pBillTransaction->ChargeOperation);
        //sendCommand(pck);
        pBillTransaction->eventMonitor.wait(1000);
    }
    else
        pBillTransaction->status = TRANSACTION_VALID;
    #else
    pBillTransaction->status = TRANSACTION_VALID;
    #endif

    ProcessSaccEvent(TRANSACTION_COMMITED, pBillTransaction);

    smsc_log_debug(logger, "Commiting billId=%d...", billId);

    int status = pBillTransaction->status, billtype = pBillTransaction->tariffRec.billType;

    logEvent("commit", !(status == TRANSACTION_WAIT_ANSWER || status == TRANSACTION_INVALID), pBillTransaction->billingInfoStruct, billId);

    deleteBillTransaction(pBillTransaction);

    #ifdef MSAG_INMAN_BILL
    if (status == TRANSACTION_VALID && billtype == scag::bill::infrastruct::INMAN)
    {
        SPckDeliverySmsResult op;
        op.Hdr().dlgId = billId;
        sendCommand(op);
    }
    #endif

    if(status == TRANSACTION_WAIT_ANSWER || status == TRANSACTION_INVALID)
        throw SCAGException("Transaction billId=%d %s.", billId, status == TRANSACTION_INVALID ? "invalid" : "timed out");
}

void BillingManagerImpl::Rollback(int billId, bool timeout)
{
    BillTransaction * pBillTransaction = getBillTransaction(billId);

    #ifdef MSAG_INMAN_BILL
    if (pBillTransaction->status == TRANSACTION_VALID && pBillTransaction->tariffRec.billType == scag::bill::infrastruct::INMAN)
    {
        SPckDeliverySmsResult op;
        op.Hdr().dlgId = billId;
        op.Cmd().setResultValue(2);
        sendCommand(op);
    }
    #endif

    ProcessSaccEvent(timeout ? TRANSACTION_TIME_OUT : TRANSACTION_CALL_ROLLBACK, pBillTransaction);

    logEvent(timeout ? "rollback(timeout)" : "rollback", true, pBillTransaction->billingInfoStruct, billId);

    deleteBillTransaction(pBillTransaction);

    smsc_log_debug(logger, "Transaction rolled back (billId=%d) %s", billId, timeout ? "by timeout" : "");
}

void BillingManagerImpl::Info(int billId, BillingInfoStruct& bis, TariffRec& tariffRec)
{
    BillTransaction *p = getBillTransaction(billId);
    bis = p->billingInfoStruct;
    tariffRec = p->tariffRec;
}

void BillingManagerImpl::modifyBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, SaccBillingInfoEvent& ev)
{
    ev.Header.cCommandId = billCommand;
    ev.Header.sCommandStatus = commandStatus;

    timeval tv;
    gettimeofday(&tv,0);

    ev.Header.lDateTime = (uint64_t)tv.tv_sec*1000 + (tv.tv_usec / 1000);
}

void BillingManagerImpl::makeBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, TariffRec& tariffRec, BillingInfoStruct& billingInfo, SaccBillingInfoEvent& ev)
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
    ev.Header.pSessionKey.append(buff);
}

#ifdef MSAG_INMAN_BILL
void BillingManagerImpl::onChargeSmsResult(ChargeSmsResult* result, CsBillingHdr_dlg * hdr)
{
    if (!result) return;

    MutexGuard guard(inUseLock);

    int billId = hdr->dlgId;

    BillTransaction **p = BillTransactionHash.GetPtr(billId);

    if(!p)
    {
        smsc_log_error(logger, "Cannot find transaction for billId=%d", billId);
        //TODO: do what we must to do
        return;
    }

    (*p)->status = result->GetValue() == ChargeSmsResult::CHARGING_POSSIBLE ? TRANSACTION_VALID : TRANSACTION_INVALID;

    (*p)->eventMonitor.notify();
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
    op.setSmsXSrvs(SMSX_INCHARGE_SRV);

    smsc_log_debug(logger, "***** SN=%s, CPN=%s, SID=%d", str.c_str(), billingInfoStruct.AbonentNumber.c_str(), billingInfoStruct.serviceId);
}

#endif /* MSAG_INMAN_BILL */

bool BillingManagerImpl::Reconnect()
{
    #ifdef MSAG_INMAN_BILL

    if (socket->Init(m_Host.c_str(), m_Port, 1000)) 
    {
        smsc_log_warn(logger, "Can't init socket to BillingServer on host '%s', port '%d': error %s (%d)\n", m_Host.c_str(), m_Port, strerror(errno), errno);
        return false;
    }

    if (socket->Connect())
    {
        smsc_log_warn(logger, "Can't connect socket to BillingServer on host '%s', port '%d' : %s (%d)\n", m_Host.c_str(), m_Port, strerror(errno), errno);
        return false;
    }        
    #endif

    return true;
}

#ifdef MSAG_INMAN_BILL
void BillingManagerImpl::onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& pck)
{
    INPPacketAC* c = static_cast<INPPacketAC *>(pck.get());
    CsBillingHdr_dlg* hdr = static_cast<CsBillingHdr_dlg*>(c->pHdr());
    ChargeSmsResult* cmd = static_cast<ChargeSmsResult*>(c->pCmd());

     if(hdr->Id() != INPCSBilling::HDR_DIALOG)
        throw SCAGException("unsupported Inman packet header: %u", hdr->Id());

     if(cmd->Id() == INPCSBilling::CHARGE_SMS_RESULT_TAG)
     {
         try { 
             cmd->loadDataBuf();
             this->onChargeSmsResult(cmd, hdr);
         } catch (SerializerException& exc)
         {
             throw SCAGException("Corrupted cmd %u (dlgId: %u): %s",
                                 cmd->Id(), hdr->dlgId, exc.what());
         }
     } else throw SCAGException("Unknown command recieved: %u", cmd->Id());
}

void BillingManagerImpl::onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc)
{
    smsc_log_error(logger, "BillingManager error: Cannot receive command. Details: %s", p_exc->what());
    m_Connected = false;
}
#endif

void BillingManagerImpl::Start()
{
    MutexGuard guard(stopLock);
    if (!m_bStarted)
    {
        m_bStarted = true;
    #ifdef MSAG_INMAN_BILL
        Thread::Start();
    #endif
    }
}

void BillingManagerImpl::Stop()
{
    MutexGuard guard(stopLock);
    if(m_bStarted)
    {
        m_bStarted = false;
        connectEvent.Signal();
        exitEvent.Wait();
    }
    smsc_log_info(logger,"BillingManager::stop");
}

int BillingManagerImpl::Execute()
{
    #ifdef MSAG_INMAN_BILL
    smsc_log_info(logger,"BillingManager::start executing");
    while(isStarted())
    {
        try
        {
            if(m_Connected)
            {
                fd_set read;
                FD_ZERO( &read );
                FD_SET( socket->getSocket(), &read );

                struct timeval tv;
                tv.tv_sec = 10; 
                tv.tv_usec = 500;

                int n = select(socket->getSocket()+1, &read, 0, 0, &tv);

                if( n > 0 ) pipe->onReadEvent();
            }
            else
            {
                m_Connected = Reconnect();
                if(!m_Connected) connectEvent.Wait(m_ReconnTimeout * 1000);
            }
        } catch (SCAGException& e)
        {
            smsc_log_error(logger, "BillingManager error: %s", e.what());
            m_Connected = false;
        }
    }

    smsc_log_info(logger,"BillingManager::stop executing");
    exitEvent.Signal();
    #endif
    return 0;
}

}}
