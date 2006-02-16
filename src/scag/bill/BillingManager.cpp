#include "BillingManager.h"
#include <scag/util/singleton/Singleton.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/threads/Thread.hpp>
#include <core/buffers/IntHash.hpp>

#include "scag/exc/SCAGExceptions.h"

#include "logger/Logger.h"
#include "core/network/Socket.hpp"

#include "inman/interaction/messages.hpp"
#include "inman/common/console.hpp"
#include "inman/common/util.hpp"
#include "inman/interaction/connect.hpp"


namespace scag { namespace bill {

using namespace smsc::core::threads;
using namespace scag::util::singleton;
using namespace scag::exceptions;

using smsc::core::network::Socket;
using smsc::logger::Logger;
using namespace smsc::inman::interaction;

using smsc::core::threads::Thread;
using smsc::inman::common::Console;
using smsc::inman::common::format;



class BillingManagerImpl : public BillingManager, public Thread, public SmscHandler
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


    Mutex stopLock;
    Mutex inUseLock;
    Event exitEvent;

    bool m_bStarted;
    Logger * logger;

    Socket * socket;
    Connect * pipe;

    EventMonitorEntity * EventMonitorArray;

    int m_MaxEventMonitors;
    int m_lastBillId; 

    void Stop();
    bool isStarted();
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

    BillingManagerImpl() : 
        m_bStarted(false), 
        EventMonitorArray(0), 
        socket(0), 
        pipe(0), 
        logger(Logger::getInstance("scag.BM")),
        m_lastBillId(0) {}

    ~BillingManagerImpl()
    {
        if (EventMonitorArray) delete EventMonitorArray;

        if (socket) delete socket;
        if (pipe) delete pipe;
    }

};

static bool  bBillingManagerInited = false;
static Mutex initBillingManagerLock;

inline unsigned GetLongevity(BillingManager*) { return 7; }
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
            throw std::runtime_error("BillingManager not inited!");
    }
    return SingleBM::Instance();
}

void BillingManagerImpl::Stop()
{
    MutexGuard guard(stopLock);

    if (m_bStarted) 
    {
        m_bStarted = false;
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
    m_MaxEventMonitors = cfg.MaxThreads;
    EventMonitorArray = new EventMonitorEntity[m_MaxEventMonitors];
    
    std::string msg;

    if (socket->Init(cfg.BillingHost.c_str(), cfg.BillingPort, 1000)) {
        msg = format("BillingManager error - can't init socket: %s (%d)\n", strerror(errno), errno);

        smsc_log_error(logger, msg.c_str());
        throw SCAGException(msg.c_str());
    }

    if (socket->Connect()) {
        msg = format("BillingManager error - can't connect socket: %s (%d)\n", strerror(errno), errno);
        smsc_log_error(logger, msg.c_str());
        throw SCAGException(msg.c_str());
    }
    pipe = new Connect(socket, SerializerInap::getInstance(), Connect::frmLengthPrefixed, logger);

}


int BillingManagerImpl::Execute()
{
    smsc_log_info(logger,"BillingManager::start executing");

    while (isStarted())
    {
/*
            fd_set read;
            FD_ZERO( &read );
            FD_SET( socket->getSocket(), &read );

            int n = select(  socket->getSocket()+1, &read, 0, 0, 0 );

            if( n > 0 )
            {
                SmscCommand* cmd = static_cast<SmscCommand*>(pipe->receive());
                assert( cmd );
                cmd->handle( this );
            }
*/

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

