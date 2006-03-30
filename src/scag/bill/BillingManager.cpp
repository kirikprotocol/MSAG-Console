#include "BillingManager.h"
#include <scag/util/singleton/Singleton.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/threads/Thread.hpp>
#include <core/buffers/IntHash.hpp>

#include "scag/exc/SCAGExceptions.h"
#include "BillingManagerWrapper.h"

namespace scag { namespace bill {

using namespace smsc::core::threads;
using namespace scag::util::singleton;
using namespace scag::exceptions;

using smsc::logger::Logger;



class BillingManagerImpl : public BillingManager, public Thread, public BillingManagerWrapper
{
    struct BillTransaction
    {
        CTransportId transportId;
        int EventMonitorIndex;
        TransactionStatus status;

        BillTransaction() : status(TRANSACTION_NOT_STARTED), EventMonitorIndex(-1) { }
    };

    struct EventMonitorEntity
    {
        EventMonitor eventMonitor;
        bool inUse;

        EventMonitorEntity() : inUse(false) {}
    };

    IntHash <BillTransaction> BillTransactionHash;

    Event connectEvent;
    Mutex stopLock;
    Mutex inUseLock;
    Event exitEvent;

    bool m_bStarted;

    EventMonitorEntity * EventMonitorArray;

    int m_MaxEventMonitors;
    int m_lastBillId; 
    int m_TimeOut;

    void Stop();
    bool isStarted();

	InfrastructureImpl infrastruct;

public:
    void init(BillingManagerConfig& cfg);

    virtual int Execute();
    virtual void Start();

    virtual int ChargeBill(CTransportId& transportId);
    virtual TransactionStatus CheckBill(int billId, EventMonitor * eventMonitor);
    virtual TransactionStatus GetStatus(int billId);

    virtual void commit(int billId);
    virtual void rollback(int billId);

    virtual void onChargeSmsResult(ChargeSmsResult* result);

    virtual Infrastructure& getInfrastructure() { return infrastruct; };

    BillingManagerImpl() : 
        m_bStarted(false), 
        EventMonitorArray(0), 
        m_lastBillId(0) {}

    ~BillingManagerImpl()
    {
        Stop();
        if (EventMonitorArray) delete EventMonitorArray;
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

    try
    {
        smsc_log_info(logger,"BillingManager connecting to host '%s', port %d", cfg.BillingHost.c_str(),cfg.BillingPort);
        initConnection(cfg.BillingHost.c_str(),cfg.BillingPort);
    }
    catch (SCAGException& e)
    {
        smsc_log_error(logger,"BillingManager error: %s", e.what());
        throw;
    }

    EventMonitorArray = new EventMonitorEntity[m_MaxEventMonitors];

    smsc_log_info(logger,"BillingManager: allowed %d threads", m_MaxEventMonitors);
    infrastruct.init(cfg.so_dir);
    smsc_log_info(logger,"BillingManager inited...");
}


int BillingManagerImpl::Execute()
{
    smsc_log_info(logger,"BillingManager::start executing");

    while (isStarted())
    {
        try
        {
            receiveCommand();
            connectEvent.Wait(1000);
        } catch (SCAGException& e)
        {
            smsc_log_error(logger, "BillingManager error: %s", e.what());
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

int BillingManagerImpl::ChargeBill(CTransportId& transportId)
{
    MutexGuard guard(inUseLock);

    m_lastBillId++;

    BillTransaction billTransaction;

    billTransaction.transportId = transportId;
    billTransaction.status = TRANSACTION_WAIT_ANSWER;

    BillTransactionHash.Insert(m_lastBillId, billTransaction);

    //TODO: send charge command with "m_lastBillId" dialod identifier

    ChargeSms op;
    op.setDialogId(m_lastBillId);

    sendChargeBillCommand(op);

    return m_lastBillId;
}

TransactionStatus BillingManagerImpl::CheckBill(int billId, EventMonitor * eventMonitor)
{
    MutexGuard guard(inUseLock);

    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);
    eventMonitor = 0;

    if (!pBillTransaction) return TRANSACTION_NOT_STARTED;

    int index = -1;
    for (int i = 0; i < m_MaxEventMonitors; i++) 
        if (!EventMonitorArray[i].inUse) 
        {
            EventMonitorArray[i].inUse = true;
            index = i;
            break;
        }

    //TODO: do what we must to do without throw exception
    if (index == -1) throw SCAGException("BillingManager error - cannot find not in use EventMonitor");

    pBillTransaction->EventMonitorIndex = index;


    eventMonitor = &(EventMonitorArray[pBillTransaction->EventMonitorIndex].eventMonitor);

    return pBillTransaction->status;
}

TransactionStatus BillingManagerImpl::GetStatus(int billId)
{
    MutexGuard guard(inUseLock);

    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);

    if (!pBillTransaction) return TRANSACTION_NOT_STARTED;
    return pBillTransaction->status;
}


void BillingManagerImpl::commit(int billId)
{
    MutexGuard guard(inUseLock);

    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);

    if (!pBillTransaction) 
    {
        //TODO: do what we must to do
        return;
    }


    //TODO: send commit

    EventMonitorArray[pBillTransaction->EventMonitorIndex].inUse = false;
    BillTransactionHash.Delete(billId);
}


void BillingManagerImpl::rollback(int billId)
{
    MutexGuard guard(inUseLock);

    BillTransaction * pBillTransaction = BillTransactionHash.GetPtr(billId);

    if (!pBillTransaction) 
    {
        //TODO: do what we must to do
        return;
    }

    EventMonitorArray[pBillTransaction->EventMonitorIndex].inUse = false;
    BillTransactionHash.Delete(billId);
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

    EventMonitorArray[pBillTransaction->EventMonitorIndex].inUse = false;
    EventMonitorArray[pBillTransaction->EventMonitorIndex].eventMonitor.notify();

}

}}

