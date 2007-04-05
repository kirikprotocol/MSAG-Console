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
        unsigned int billId;
        SaccBillingInfoEvent billEvent;
        BillingInfoStruct billingInfoStruct;
        BillTransaction() : status(TRANSACTION_NOT_STARTED) {}
    };
    #ifdef MSAG_INMAN_BILL
    struct SendTransaction
    {
        TransactionStatus status;
        EventMonitor eventMonitor;
        uint32_t billId;
        SendTransaction() : status(TRANSACTION_WAIT_ANSWER) {}
    };
    #endif

    Logger *logger;

    IntHash <BillTransaction *> BillTransactionHash;
    IntHash <SendTransaction *> SendTransactionHash;

    Event connectEvent;
    Mutex stopLock;
    Mutex inUseLock;
    Event exitEvent;

    bool m_bStarted;

    unsigned int m_lastBillId;
    int m_ReconnectTimeout, m_Timeout;

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
    TransactionStatus sendCommandAndWaitAnswer(SPckChargeSms& op);

    void fillChargeSms(smsc::inman::interaction::ChargeSms& op, BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec);
    virtual void onChargeSmsResult(ChargeSmsResult* result, CsBillingHdr_dlg * hdr);
    bool Reconnect();
    void InitConnection(std::string& host, int port)
    {
        m_Host = host;
        m_Port = port;
    }
    #endif /* MSAG_INMAN_BILL */

    void ProcessResult(const char* eventName, BillingTransactionEvent billingTransactionEvent, BillTransaction * billTransaction);

    void modifyBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, SaccBillingInfoEvent& ev);
    void makeBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, TariffRec& tariffRec, BillingInfoStruct& billingInfo, SaccBillingInfoEvent& ev);

    uint32_t genBillId();
    BillTransaction* getBillTransaction(uint32_t billId);
    void putBillTransaction(uint32_t billId, BillTransaction* p);
    void ClearTransactions()
    {
        int key;
        BillTransaction *value;
        SendTransaction *st;

        MutexGuard mg(inUseLock);
        for(IntHash <SendTransaction *>::Iterator it = SendTransactionHash.First(); it.Next(key, st);)
            st->eventMonitor.notify();

        for (IntHash <BillTransaction *>::Iterator it = BillTransactionHash.First(); it.Next(key, value);)
            delete value;

        BillTransactionHash.Empty();
    }
    void logEvent(const char *type, bool success, BillingInfoStruct& b, int billID);

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
        Stop();
        ClearTransactions();
        #ifdef MSAG_INMAN_BILL
        if(pipe) delete pipe;
        #endif
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

    m_ReconnectTimeout = cfg.BillingReconnectTimeOut;
    m_Timeout = cfg.BillingTimeOut;

    #ifdef MSAG_INMAN_BILL
    InitConnection(cfg.BillingHost, cfg.BillingPort);
    smsc_log_info(logger,"BillingManager connecting to host '%s', port %d", cfg.BillingHost.c_str(),cfg.BillingPort);
    if(socket->Init(m_Host.c_str(), m_Port, 1000))
        throw SCAGException("BillingManager: cant init socket host=%s, port=%d", m_Host.c_str(), m_Port);
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

void BillingManagerImpl::ProcessResult(const char *eventName, BillingTransactionEvent event, BillTransaction *b)
{
    BillingCommandStatus i;
    const char *p;
    switch (b->status)
    {
        case TRANSACTION_INVALID:
            i = INVALID_TRANSACTION;
            p = "invalid(charging is impossible?)";
            break;
        case TRANSACTION_NOT_STARTED:
            i = REJECTED_BY_SERVER;
            p = "denied";
            break;
        case TRANSACTION_WAIT_ANSWER:
            i = SERVER_NOT_RESPONSE;
            p = "time out";
            break;
        default:
            i = COMMAND_SUCCESSFULL;
    }
    modifyBillEvent(event, i, b->billEvent);
    Statistics::Instance().registerSaccEvent(b->billEvent);
    logEvent(eventName, i == COMMAND_SUCCESSFULL, b->billingInfoStruct, b->billId);
    if(i != COMMAND_SUCCESSFULL)
        throw SCAGException("Transaction billId=%d %s", b->billId, p);
}

void BillingManagerImpl::logEvent(const char *tp, bool success, BillingInfoStruct& b, int billID)
{
    smsc_log_info(logger, "bill %s: %s billId=%d, abonent=%s, opId=%d, sId=%d, providerId=%d",
        tp, success ? "success" : "failed", billID, b.AbonentNumber.c_str(), b.operatorId, b.serviceId, b.providerId);
}

BillingManagerImpl::BillTransaction* BillingManagerImpl::getBillTransaction(uint32_t billId)
{
    MutexGuard mg(inUseLock);
    BillTransaction** pp = BillTransactionHash.GetPtr(billId);
    if (!pp || !*pp) throw SCAGException("Cannot find transaction for billId=%d", billId);
    BillTransactionHash.Delete(billId);
    return *pp;
}

void BillingManagerImpl::putBillTransaction(uint32_t billId, BillTransaction* p)
{
    MutexGuard mg(inUseLock);
    BillTransactionHash.Insert(billId, p);
}

uint32_t BillingManagerImpl::genBillId()
{
    MutexGuard mg(inUseLock);
    return ++m_lastBillId;
}

unsigned int BillingManagerImpl::Open(BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec)
{
    uint32_t billId = genBillId();

    smsc_log_debug(logger, "Opening billId=%d...", billId);

    auto_ptr<BillTransaction> p(new BillTransaction());

    p->tariffRec = tariffRec;
    p->billingInfoStruct = billingInfoStruct;
    p->billId = billId;
    makeBillEvent(TRANSACTION_OPEN, COMMAND_SUCCESSFULL, tariffRec, billingInfoStruct, p->billEvent);

    #ifdef MSAG_INMAN_BILL
    if(tariffRec.billType == scag::bill::infrastruct::INMAN)
    {
        fillChargeSms(p->ChargeOperation.Cmd(), billingInfoStruct, tariffRec);
        p->ChargeOperation.Hdr().dlgId = billId;
        p->status = sendCommandAndWaitAnswer(p->ChargeOperation);
        if(p->status == TRANSACTION_VALID)
        {
            SPckDeliverySmsResult opRes;
            opRes.Hdr().dlgId = billId;
            opRes.Cmd().setResultValue(1);
            opRes.Cmd().setFinal(false); // To skip CDR creation
            sendCommand(opRes);
        }
    }
    else
    #endif
        p->status = TRANSACTION_VALID;

    ProcessResult("open", TRANSACTION_OPEN, p.get());

    putBillTransaction(billId, p.get());
    p.release();

    return billId;
}

void BillingManagerImpl::Commit(int billId)
{
    smsc_log_debug(logger, "Commiting billId=%d...", billId);

    auto_ptr<BillTransaction> p(getBillTransaction(billId));

    #ifdef MSAG_INMAN_BILL
    if(p->tariffRec.billType == scag::bill::infrastruct::INMAN)
    {
        p->status = sendCommandAndWaitAnswer(p->ChargeOperation);
        if(p->status == TRANSACTION_VALID)
        {
            SPckDeliverySmsResult op;
            op.Hdr().dlgId = billId;
            sendCommand(op);
        }
    }
    else
    #endif
        p->status = TRANSACTION_VALID;

    ProcessResult("commit", TRANSACTION_COMMITED, p.get());
}

void BillingManagerImpl::Rollback(int billId, bool timeout)
{
    smsc_log_debug(logger, "Rolling back billId=%d...", billId);

    auto_ptr<BillTransaction> p(getBillTransaction(billId));

    #ifdef MSAG_INMAN_BILL
    if (p->status == TRANSACTION_VALID && p->tariffRec.billType == scag::bill::infrastruct::INMAN)
    {
        SPckDeliverySmsResult op;
        op.Hdr().dlgId = billId;
        op.Cmd().setResultValue(2);
        sendCommand(op);
    }
    #endif

    ProcessResult(timeout ? "rollback(timeout)" : "rollback", 
            timeout ? TRANSACTION_TIME_OUT : TRANSACTION_CALL_ROLLBACK, p.get());
}

void BillingManagerImpl::Info(int billId, BillingInfoStruct& bis, TariffRec& tariffRec)
{
    MutexGuard mg(inUseLock);
    BillTransaction** p = BillTransactionHash.GetPtr(billId);
    if (!p || !*p) throw SCAGException("Cannot find transaction for billId=%d", billId);
    bis = (*p)->billingInfoStruct;
    tariffRec = (*p)->tariffRec;
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

    SendTransaction **p = SendTransactionHash.GetPtr(hdr->dlgId);
    if(!p)
    {
        smsc_log_error(logger, "Cannot find transaction for billId=%d", hdr->dlgId);
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


bool BillingManagerImpl::Reconnect()
{
    if(!socket->Connect()) 
    {
        pipe->Reset();
        smsc_log_warn(logger, "Connected socket to BillingServer on host '%s', port '%d'", m_Host.c_str(), m_Port);
        return true;
    }
    smsc_log_warn(logger, "Can't connect socket to BillingServer on host '%s', port '%d': error %s (%d)", m_Host.c_str(), m_Port, strerror(errno), errno);
    return false;
}

TransactionStatus BillingManagerImpl::sendCommandAndWaitAnswer(SPckChargeSms& op)
{
    SendTransaction st;
    if(!m_Connected) return TRANSACTION_WAIT_ANSWER;
    {
        MutexGuard mg(inUseLock);
        SendTransactionHash.Insert(op.Hdr().dlgId, &st);
    }
    pipe->sendPck(&op);
    st.eventMonitor.wait(m_Timeout);
    {
        MutexGuard mg(inUseLock);
        SendTransactionHash.Delete(op.Hdr().dlgId);
    }
    return st.status;
}

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
    socket->Close();
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
        socket->Close();
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
                int n = select(socket->getSocket() + 1, &read, 0, 0, &tv);
                smsc_log_info(logger,"select result =%d", n);
                if(n < 0 || (n > 0 && pipe->onReadEvent()))
                    m_Connected = false;
            }
            else
            {
                m_Connected = Reconnect();
                if(!m_Connected) connectEvent.Wait(m_ReconnectTimeout * 1000);
            }
        }catch (SCAGException& e)
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
