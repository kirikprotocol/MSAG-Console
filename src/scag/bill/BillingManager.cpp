
#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>

#include <scag/util/singleton/Singleton.h>

#include "BillingManager.h"
#include "BillingMachine.h"

namespace scag { namespace bill
{
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;

    using smsc::core::buffers::IntHash;

    class BillingManagerImpl : public BillingManager
    {
    private:

        static const char* MACHINE_INIT_FUNCTION;
        IntHash<BillingMachine *> machines; // User's billing machines by their ids

    public:

        BillingManagerImpl() {};
        virtual ~BillingManagerImpl() {};

        void init(const std::string& cfg_dir, const std::string& so_dir);
        virtual void rollback(const Bill& bill);
    };

const char* BillingManagerImpl::MACHINE_INIT_FUNCTION = "initBillingMachine";

// ################## Singleton related issues ##################

static bool  bBillingManagerInited = false;
static Mutex initBillingManagerLock;

inline unsigned GetLongevity(BillingManager*) { return 7; } 
typedef SingletonHolder<BillingManagerImpl> SingleBM;

void BillingManager::Init(const std::string& cfg_dir, const std::string& so_dir)
{
    if (!bBillingManagerInited)
    {
        MutexGuard guard(initBillingManagerLock);
        if (!bBillingManagerInited) {
            BillingManagerImpl& bm = SingleBM::Instance();
            bm.init(cfg_dir,so_dir);
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

// ################ TODO: Actual BillingManager Implementation follows ################ 

void BillingManagerImpl::init(const std::string& cfg_dir, const std::string& so_dir) // possible throws exceptions
{
    // TODO: loadup all so modules from so_dir, call MACHINE_INIT_FUNCTION,
    //       add to machines & register action factory in MainActionFactory in RuleEngine
}

void BillingManagerImpl::rollback(const Bill& bill) // possible throws exceptions
{
    // TODO: dispath call to billing machine by bill.machine_id
}

}}


