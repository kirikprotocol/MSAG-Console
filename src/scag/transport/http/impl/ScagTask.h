#ifndef SCAG_TRANSPORT_HTTP_IMPL_SCAG_TASK
#define SCAG_TRANSPORT_HTTP_IMPL_SCAG_TASK

#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"
#include "scag/transport/http/base/HttpCommand2.h"
#include "scag/transport/http/base/HttpContext.h"
#include "scag/transport/http/base/HttpProcessor.h"
#include "TaskList.h"

namespace scag2 {
namespace transport {
namespace http {

using smsc::core::synchronization::EventMonitor;
using smsc::logger::Logger;

class HttpManagerImpl;

class ScagTask : public ThreadedTask 
{
public:
    ScagTask(HttpManagerImpl& m, HttpProcessor& p);

    virtual int Execute();
    virtual const char* taskName();
    virtual void stop();
    
protected:
    HttpManagerImpl &manager;
    HttpProcessor &processor;
    Logger *logger;
	char bufName[30];
};

}}}

#endif // SCAG_TRANSPORT_SCAG_TASK
