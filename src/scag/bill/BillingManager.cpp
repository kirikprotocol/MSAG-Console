
#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>

#include <scag/util/singleton/Singleton.h>

#include "BillingManager.h"
#include "scag/re/RuleEngine.h"

#include <sys/types.h>
#include <dirent.h>

#include <dlfcn.h>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <logger/Logger.h>
#include <scag/exc/SCAGExceptions.h>
#include <core/buffers/Array.hpp>

#include "scag/re/actions/ActionFactory.h"

namespace scag { namespace bill
{
    using namespace scag::exceptions;
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;
    using namespace scag::re::actions;
    using namespace scag::re;

    using smsc::logger::Logger;

    using smsc::core::buffers::IntHash;
    using smsc::core::buffers::Array;


    class BillingManagerImpl : public BillingManager
    {
        bool isValidFileName(const std::string& fname);

        static const char* MACHINE_INIT_FUNCTION;
        static smsc::logger::Logger *logger;
        static Array<void *>        handles;
        static Mutex                loadupLock;

    public:

        BillingManagerImpl() {};
        virtual ~BillingManagerImpl();

        void init(BillingManagerConfig& config);
        virtual void rollback(int BillId);
        virtual void commit(int BillId);
     };

const char* BillingManagerImpl::MACHINE_INIT_FUNCTION = "initBillingMachine";
smsc::logger::Logger * BillingManagerImpl::logger = 0;
Array<void *>         BillingManagerImpl::handles;
Mutex                 BillingManagerImpl::loadupLock;


// ################## Singleton related issues ##################

static bool  bBillingManagerInited = false;
static Mutex initBillingManagerLock;

inline unsigned GetLongevity(BillingManager*) { return 7; } 
typedef SingletonHolder<BillingManagerImpl> SingleBM;

void BillingManager::Init(BillingManagerConfig& config)
{
    if (!bBillingManagerInited)
    {
        MutexGuard guard(initBillingManagerLock);

        if (!bBillingManagerInited) {
            BillingManagerImpl& bm = SingleBM::Instance();
            bm.init(config);
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

bool BillingManagerImpl::isValidFileName(const std::string& fname)
{
    if (fname.size() < 4) return false;
    return (fname.substr(fname.size() - 3,3) == ".so");
}

void BillingManagerImpl::init(BillingManagerConfig& config) // possible throws exceptions
{
    // TODO: loadup all so modules from so_dir, call MACHINE_INIT_FUNCTION,
    //       add to machines & register action factory in MainActionFactory in RuleEngine


    if (!logger)
      logger = Logger::getInstance("scag.bill.BillingManager");

    smsc_log_info(logger,"Billing manager inited");
}

void BillingManagerImpl::rollback(int BillId) // possible throws exceptions
{
    // TODO: dispath call to billing machine by bill.machine_id
   /* if (!machines.Exist(bill.machine_id)) return;
    BillingMachine * bm = machines.Get(bill.machine_id);
    bm->rollback(bill);                            */
}

void BillingManagerImpl::commit(int BillId)
{

}


BillingManagerImpl::~BillingManagerImpl()
{
   MutexGuard guard(loadupLock);

   //TODO : chick if we must free billingmachines




   if (!handles.Count()) return;

   smsc_log_info(logger, "Unloading BillingMachines ...");
   while (handles.Count())
   {
       void* handle=0L;
       (void) handles.Pop(handle);
       if (handle) dlclose(handle);
   }
   smsc_log_info(logger, "BillingMachines unloaded");

}


}}


