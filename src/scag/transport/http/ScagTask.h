#ifndef SCAG_TRANSPORT_SCAG_TASK
#define SCAG_TRANSPORT_SCAG_TASK

#include "core/synchronization/EventMonitor.hpp"
#include "HttpCommand.h"
#include "HttpContext.h"
#include "HttpProcessor.h"
#include "TaskList.h"

namespace scag { namespace transport { namespace http
{
using smsc::core::synchronization::EventMonitor;

class HttpManager;

class ScagTask : public ThreadedTask {
public:
    ScagTask(HttpManager& m, HttpProcessor& p);

    virtual int Execute();
    virtual const char* taskName();
    
protected:
    HttpManager &manager;
    HttpProcessor &processor;
};

}}}

#endif // SCAG_TRANSPORT_SCAG_TASK
