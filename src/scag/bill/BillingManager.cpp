
#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>

#include <scag/util/singleton/Singleton.h>

#include "BillingManager.h"
#include "BillingMachine.h"


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

    using smsc::logger::Logger;

    using smsc::core::buffers::IntHash;
    using smsc::core::buffers::Array;


    class BillingManagerImpl : public BillingManager
    {
        IntHash<BillingMachine *> machines; // User's billing machines by their ids
        bool isValidFileName(const std::string& fname);
        void LoadBillingMachine(const char * dlpath,const std::string& cfg_dir, const ActionFactory * mainActionFactory);

        static const char* MACHINE_INIT_FUNCTION;
        static smsc::logger::Logger *logger;
        static Array<void *>        handles;
        static Mutex                loadupLock;

    public:

        BillingManagerImpl() {};
        virtual ~BillingManagerImpl();

        void init(const BillingManagerConfig& config);
        virtual void rollback(const Bill& bill);
        virtual void commit(const Bill& bill);
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

void BillingManager::Init(const BillingManagerConfig& config)
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



void BillingManagerImpl::LoadBillingMachine(const char * dlpath,const std::string& cfg_dir, const ActionFactory * mainActionFactory)
{
    if (!mainActionFactory) return;
    MutexGuard guard(loadupLock);

    smsc_log_info(logger, "Loading '%s' library", dlpath);
    void* dlhandle = dlopen(dlpath, RTLD_LAZY);
    if (dlhandle)
    {
        initBillingMachineFn fnhandle =
            (initBillingMachineFn)dlsym(dlhandle, MACHINE_INIT_FUNCTION);
        if (fnhandle)
        {
            int id = machines.Count();
            BillingMachine* billingMachine = (*fnhandle)(id,cfg_dir);
            if (billingMachine)
            {
                machines.Insert(id,billingMachine);
                mainActionFactory->registerChild(billingMachine->getActionFactory());
                //DataSourceFactory::registerFactory(dsf, identity);
            }
            else
            {
                smsc_log_error(logger, "Load of '%s' library. Call to initBillingMachine failed ! ", dlpath);
                dlclose(dlhandle);
                throw SCAGException("Cannot load billing machine '%s'",dlpath);
            }
        }
        else
        {
            smsc_log_error(logger, "Load of '%s' library. Call to dlsym() failed ! ", dlpath);
            dlclose(dlhandle);
            throw SCAGException("Cannot load billing machine '%s'",dlpath);
        }
    }
    else
    {
        char buf[256];
        smsc_log_error(logger, "Load of '%s' at '%s' library. Call to dlopen() failed:%s ! ", dlpath,getcwd(buf,sizeof(buf)),dlerror());
        throw SCAGException("Cannot load billing machine '%s'",dlpath);
    }

    (void)handles.Push(dlhandle);
    smsc_log_info(logger, "Loading '%s' library done.", dlpath);
}


void BillingManagerImpl::init(const BillingManagerConfig& config) // possible throws exceptions
{
    // TODO: loadup all so modules from so_dir, call MACHINE_INIT_FUNCTION,
    //       add to machines & register action factory in MainActionFactory in RuleEngine

    if (!logger)
      logger = Logger::getInstance("scag.bill.BillingManager");

    DIR * pDir = 0;
    dirent * pDirEnt = 0;
    int ruleId = 0;

    pDir = opendir(config.so_dir.c_str());
    if (!config.mainActionFactory) 
    {
        smsc_log_error(logger, "Fatal Error: Main Action Factory is invalid");
        return;
    }

    if (!pDir) 
    {
        smsc_log_error(logger, "Invalid directory '%s'", config.so_dir.c_str());
        return;
    }



    while (pDir)
    {
         pDirEnt = readdir(pDir);
         if (pDirEnt)
         {
             if (isValidFileName(pDirEnt->d_name))
             {
                 std::string fileName = config.so_dir;
                 fileName.append("/");
                 fileName.append(pDirEnt->d_name);
                 try
                 {
                     LoadBillingMachine(fileName.c_str(),config.cfg_dir,config.mainActionFactory);

                     
                 } 
                 catch (SCAGException& e)
                 {
                     smsc_log_error(logger,e.what());
                 }
             }
             else if ((strcmp(pDirEnt->d_name,".")!=0)&&(strcmp(pDirEnt->d_name,"..")!=0))
             {
                 smsc_log_error(logger, "Invalid file name '%s'", pDirEnt->d_name);
             }
         }
         else
         {
             closedir(pDir);
             return;
         }
    }

    closedir(pDir);

}

void BillingManagerImpl::rollback(const Bill& bill) // possible throws exceptions
{
    // TODO: dispath call to billing machine by bill.machine_id
    if (!machines.Exist(bill.machine_id)) return;
    BillingMachine * bm = machines.Get(bill.bill_id);
    bm->rollback(bill);
}

void BillingManagerImpl::commit(const Bill& bill)
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


