#ifndef SCAG_TRANSPORT_SCAG_TASK
#define SCAG_TRANSPORT_SCAG_TASK

#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"
#include "HttpCommand.h"
#include "HttpContext.h"
#include "HttpProcessor.h"
#include "TaskList.h"

namespace scag { namespace transport { namespace http
{
using smsc::core::synchronization::EventMonitor;
using smsc::logger::Logger;

class HttpManager;

class ScagTask : public ThreadedTask {
public:
    ScagTask(HttpManager& m, HttpProcessor& p);

    virtual int Execute();
    virtual const char* taskName();
    virtual void stop();
    
protected:
    HttpManager &manager;
    HttpProcessor &processor;
    Logger *logger;
};

}}}

#endif // SCAG_TRANSPORT_SCAG_TASK
