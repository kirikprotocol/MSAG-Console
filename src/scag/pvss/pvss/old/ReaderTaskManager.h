#ifndef _SCAG_PVSS_SERVER_READERTASKMANAGER_H_
#define _SCAG_PVSS_SERVER_READERTASKMANAGER_H_

#include "IOTaskManager.h"
#include "IOTask.h"

namespace scag2 {
namespace pvss  {

class ReaderTaskManager : public IOTaskManager 
{
public:
    ReaderTaskManager(const SyncConfig& cfg) : IOTaskManager(cfg, "readerman"), perfCounterOn_(cfg.getPerfCounterOn()) {}

    void shutdown() {
        {
            smsc::core::synchronization::MutexGuard mg(perfMutex_);
            readers_.Empty();
        }
        IOTaskManager::shutdown();
    }

    Performance getPerformance() 
    {
        smsc::core::synchronization::MutexGuard mg(perfMutex_);
        Performance perf;
        if (!perfCounterOn_) {
            return perf;
        }
        for ( int i = 0, ie = readers_.Count(); i != ie; ++i ) {
            perf.inc(readers_[i]->getPerformance());
        }
        return perf;
    }

protected:
    virtual void postRegister( ConnectionContext* cx ) 
    {
        cx->getSocket()->Write("OK", 2);
    }

    IOTask* newTask() {
        MTPersReader* reader = new MTPersReader(connectionTimeout_, ioTimeout_);
        {
            smsc::core::synchronization::MutexGuard mg(perfMutex_);
            if (perfCounterOn_) {
                readers_.Push(reader);
            }
        }
        return reader;
    }

private:
    smsc::core::synchronization::Mutex          perfMutex_;
    bool perfCounterOn_;
    smsc::core::buffers::Array< MTPersReader* > readers_;
};

}//pvss
}//scag2

#endif
  
 
