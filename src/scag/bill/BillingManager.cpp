#include "BillingManager.h"
#include <scag/util/singleton/Singleton.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/threads/Thread.hpp>
#include <core/buffers/IntHash.hpp>

#include "scag/exc/SCAGExceptions.h"
#include "BillingManagerWrapper.h"
#include "scag/config/ConfigListener.h"

//#define MSAG_INMAN_BILL


namespace scag { namespace bill {

using namespace smsc::core::threads;
using namespace scag::util::singleton;
using namespace scag::exceptions;

using smsc::logger::Logger;

class BillingManagerImpl : public BillingManager, public Thread, public BillingManagerWrapper, public ConfigListener
{
    struct BillTransaction
    {
        int EventMonitorIndex;
        TransactionStatus status;
        TariffRec tariffRec;
        smsc::inman::interaction::ChargeSms ChargeOperation;
        BillTransaction() : status(TRANSACTION_NOT_STARTED), EventMonitorIndex(-1) { }
    };

    struct EventMonitorEntity
    {
        EventMonitor eventMonitor;
        bool inUse;

        EventMonitorEntity() : inUse(false) {}
        ~EventMonitorEntity()
        {
        }
    };

    std::vector<EventMonitorEntity *> EventMonitorArray;

    IntHash <BillTransaction> BillTransactionHash;

    Event connectEvent;
    Mutex stopLock;
    Mutex inUseLock;
    Event exitEvent;

    bool m_bStarted;

    int m_MaxEventMonitors;
    int m_lastBillId; 
    int m_TimeOut;

    bool m_Connected;

    void Stop();
    bool isStarted();

    InfrastructureImpl infrastruct;

public:
    void init(BillingManagerConfig& cfg);

    virtual int Execute();
    virtual void Start();

    virtual int ChargeBill(smsc::inman::interaction::ChargeSms& op, EventMonitor ** eventMonitor, TariffRec& tariffRec);
    virtual TariffRec& CheckCharge(int billId, EventMonitor ** eventMonitor);

    virtual TransactionStatus GetStatus(int billId);

    virtual void commit(int billId);
    virtual void rollback(int billId);
    //virtual void close(int billId);


    virtual void onChargeSmsResult(ChargeSmsResult* result);

    virtual Infrastructure& getInfrastructure() { return infrastruct; };
    virtual TariffRec& getTransactionData(int billId);
    virtual void sendReject(int billId);


    void configChanged();
    
    BillingManagerImpl() : 
        m_bStarted(false), 
        m_lastBillId(0), ConfigListener(BILLMAN_CFG) {}

    ~BillingManagerImpl()
    {
        Stop();
        for (int i=0; i < EventMonitorArray.size(); i++) 
            delete EventMonitorArray[i];

        EventMonitorArray.clear();

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
    smsc_log_info(logger,"BillingManager start initing...");

    if (cfg.MaxThreads < 1) throw SCAGException("BillingManager: cant start service with %d allowed threads", cfg.MaxThreads);

    m_MaxEventMonitors = cfg.MaxThreads;
    m_TimeOut = cfg.BillingTimeOut;

    InitConnection(cfg.BillingHost, cfg.BillingPort);


    #ifdef MSAG_INMAN_BILL
    smsc_log_info(logger,"BillingManager connecting to host '%s', port %d", cfg.BillingHost.c_str(),cfg.BillingPort);
    m_Connected = Reconnect();
    #else
    smsc_log_info(logger,"BillingManager skip connection (fake connection initialized)");
    m_Connected = true;
    #endif


    for (int i=0; i < EventMonitorArray.size(); i++) 
        delete EventMonitorArray[i];

    EventMonitorArray.clear();

    for (int i=0; i < m_MaxEventMonitors; i++) 
    {
        EventMonitorEntity * ent = new EventMonitorEntity();
        EventMonitorArray.push_back(ent);
    }

    smsc_log_info(logger,"BillingManager: allowed %d threads", m_MaxEventMonitors);
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
                receiveCommand();
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

int BillingManagerImpl::ChargeBill(smsc::inman::interaction::ChargeSms& op, EventMonitor ** eventMonitor, TariffRec& tariffRec)
{
/*    BillTransaction billTransaction1;
    billTransaction1.EventMonitorIndex = 0;
    //*eventMonitor = EventMonitorArray[0].eventMonitor;
    //BillTransactionHash.Insert(1, billTransaction1);

    return 1;
*/
    MutexGuard guard(inUseLock);

    int index = -1;
    for (int i = 0; i < m_MaxEventMonitors; i++) 
        if (!EventMonitorArray[i]->inUse) 
        {
            EventMonitorArray[i]->inUse = true;
            index = i;
            break;
        }

    if (index == -1) 
        throw SCAGException("BillingManager error - cannot find not in use EventMonitor");

    BillTransaction billTransaction;
    billTransaction.EventMonitorIndex = index;

    *eventMonitor = &EventMonitorArray[billTransaction.EventMonitorIndex]->eventMonitor;

    m_lastBillId++;


    billTransaction.tariffRec = tariffRec;

    #ifdef MSAG_INMAN_BILL
    billTransaction.status = TRANSACTION_WAIT_ANSWER;
    #else
    billTransaction.status = TRANSACTION_VALID;
    #endif

    billTransaction.ChargeOperation = op;

    BillTransactionHash.Insert(m_lastBillId, billTransaction);

    op.setDialogId(m_lastBillId);

    #ifdef MSAG_INMAN_BILL
    sendCommand(op);
    #endif
    return m_lastBillId;
               

}


TransactionStatus BillingManagerImpl::GetStatus(int billId)
{
    MutexGuard guard(inUseLock);

    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);

    if (!pBillTransaction) return TRANSACTION_NOT_STARTED;
    return pBillTransaction->status;
}

TariffRec& BillingManagerImpl::getTransactionData(int billId)
{
    MutexGuard guard(inUseLock);

    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);

    if (!pBillTransaction) 
        throw SCAGException("Cannot find transaction for billId=%d", billId);

    return pBillTransaction->tariffRec;
}


TariffRec& BillingManagerImpl::CheckCharge(int billId, EventMonitor ** eventMonitor)
{
    MutexGuard guard(inUseLock);

    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);

    if (!pBillTransaction) throw SCAGException("Cannot find transaction for billId=%d", billId);

    *eventMonitor = &EventMonitorArray[pBillTransaction->EventMonitorIndex]->eventMonitor;

    pBillTransaction->ChargeOperation.setDialogId(billId);

    #ifdef MSAG_INMAN_BILL
    sendCommand(pBillTransaction->ChargeOperation);
    #else
    pBillTransaction->status = TRANSACTION_VALID;
    #endif 

    return pBillTransaction->tariffRec;
   
}


void BillingManagerImpl::commit(int billId)
{
    MutexGuard guard(inUseLock);

    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);

    if (!pBillTransaction) throw SCAGException("Cannot find transaction for billId=%d", billId);


    if (pBillTransaction->status == TRANSACTION_VALID)
    {
        smsc_log_debug(logger, "Commiting billId=%d...", billId);

        DeliverySmsResult op;
 
        op.setDialogId(billId);
        op.setResultValue(0);

        #ifdef MSAG_INMAN_BILL
        sendCommand(op);
        #endif
    }

    EventMonitorArray[pBillTransaction->EventMonitorIndex]->inUse = false;
    BillTransactionHash.Delete(billId);
}


void BillingManagerImpl::rollback(int billId)
{
    MutexGuard guard(inUseLock);
      
    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);

    if (!pBillTransaction) 
        throw SCAGException("Cannot find transaction for billId=%d", billId);

    if (pBillTransaction->status == TRANSACTION_VALID)
    {
        DeliverySmsResult op;

        op.setDialogId(billId);
        op.setResultValue(2);

        #ifdef MSAG_INMAN_BILL
        sendCommand(op);
        #endif
    }

    assert ((pBillTransaction->EventMonitorIndex >=0)&&(pBillTransaction->EventMonitorIndex < m_MaxEventMonitors));

    EventMonitorArray[pBillTransaction->EventMonitorIndex]->inUse = false;
    BillTransactionHash.Delete(billId);          
    
    smsc_log_debug(logger, "Transaction rolled back (billId=%d)", billId);
    
}

void BillingManagerImpl::sendReject(int billId)
{
    MutexGuard guard(inUseLock);

    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);

    if (!pBillTransaction) throw SCAGException("Cannot find transaction for billId=%d", billId);

    #ifdef MSAG_INMAN_BILL
    if (pBillTransaction->status == TRANSACTION_VALID)
    {
        DeliverySmsResult op;

        op.setDialogId(billId);
        op.setResultValue(1);

        pBillTransaction->status = TRANSACTION_WAIT_ANSWER;
        sendCommand(op);
    }
    #endif
    
}


void BillingManagerImpl::onChargeSmsResult(ChargeSmsResult* result)
{
    if (!result) return;

    MutexGuard guard(inUseLock);

    int billId = result->getDialogId();

    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);

    if (!pBillTransaction) 
    {
        //TODO: do what we must to do
        return;
    }
    
    if( result->GetValue() == smsc::inman::interaction::CHARGING_POSSIBLE ) 
        pBillTransaction->status = TRANSACTION_VALID;
    else 
        pBillTransaction->status = TRANSACTION_INVALID;

    EventMonitorArray[pBillTransaction->EventMonitorIndex]->inUse = false;

    EventMonitorArray[pBillTransaction->EventMonitorIndex]->eventMonitor.notify();

}

}}

