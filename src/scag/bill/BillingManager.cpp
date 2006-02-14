#include "BillingManager.h"
#include <scag/util/singleton/Singleton.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/threads/Thread.hpp>
#include <core/buffers/IntHash.hpp>

#include "scag/exc/SCAGExceptions.h"

namespace scag { namespace bill {

using namespace smsc::core::threads;
using namespace scag::util::singleton;
using namespace scag::exceptions;



class BillingManagerImpl : public BillingManager, public Thread
{
    struct BillTransaction
    {
        CTransportId transportId;
        EventMonitor eventMonitor;
        int status;

        BillTransaction() : status(0) { /*pthread_cond_init(&cond, NULL);*/ }
    };

    IntHash <BillTransaction> BillTransactionHash;


    Mutex stopLock;
    Event exitEvent;

    bool m_bStarted;

    void Stop();
    bool isStarted();
public:
    void init();

    virtual int Execute();
    virtual void Start();

    virtual int ChargeBill(CTransportId& transportId);
    virtual bool CheckBill(int billId, EventMonitor& eventMonitor);
    virtual void CommitBill(int billId);

    BillingManagerImpl() : m_bStarted(false) {}
};

static bool  bBillingManagerInited = false;
static Mutex initBillingManagerLock;

inline unsigned GetLongevity(BillingManager*) { return 7; }
typedef SingletonHolder<BillingManagerImpl> SingleBM;

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void BillingManager::Init()
{
    if (!bBillingManagerInited)
    {
        MutexGuard guard(initBillingManagerLock);
        if (!bBillingManagerInited) {
            BillingManagerImpl& bm = SingleBM::Instance();
            bm.init(); bm.Start();
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
        //awakeEvent.Signal();
        exitEvent.Wait(); 
    }
    //smsc_log_info(logger,"BillingManager::stop");
}

bool BillingManagerImpl::isStarted()
{
    return m_bStarted;
}

void BillingManagerImpl::init()
{
}


int BillingManagerImpl::Execute()
{
    //smsc_log_info(logger,"BillingManager::start executing");

    while (isStarted())
    {
        //int secs = processExpire();
        //smsc_log_debug(logger,"SessionManager::----------- ping %d",secs);
        //awakeEvent.Wait(secs*1000);
    }
    //smsc_log_info(logger,"BillingManager::stop executing");
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
    return 0;
}

bool BillingManagerImpl::CheckBill(int billId, EventMonitor& eventMonitor)
{
    return false;
}

void BillingManagerImpl::CommitBill(int billId)
{

}



}}

