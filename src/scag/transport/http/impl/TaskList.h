#ifndef SCAG_TRANSPORT_HTTP_IMPL_TASK_LIST
#define SCAG_TRANSPORT_HTTP_IMPL_TASK_LIST

#include "core/threads/ThreadedTask.hpp"

namespace scag2 {
namespace transport {
namespace http {

using smsc::core::threads::ThreadedTask;

class IOTaskManager;

class IOTaskParent : public ThreadedTask {    
    friend class IOTaskManager;

public:
    IOTaskParent(unsigned int c = 0) : socketCount(c), taskIndex(0) {}
    virtual int Execute();
    virtual const char* taskName();

protected:
    unsigned int socketCount;
    
private:    
    unsigned int taskIndex;
};

}}}

#endif // SCAG_TRANSPORT_TASK_LIST
