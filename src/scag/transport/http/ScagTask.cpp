#include "HttpLogger.h"
#include "ScagTask.h"
#include "Managers.h"
#include "HttpContext.h"

namespace scag { namespace transport { namespace http
{

ScagTask::ScagTask(HttpManager& m, HttpProcessor& p) : manager(m), processor(p)
{
}

int ScagTask::Execute()
{
    HttpContext *cx;

    http_log_debug( "Scag %p started", this );

    while (!isStopping) {
        while (!((cx = manager.scags.getFirst()) || isStopping)) {
            http_log_debug("Scag %p idle", this);
            manager.scags.taskMon.Lock();
            manager.scags.taskMon.wait();
            manager.scags.taskMon.Unlock();
            http_log_debug("Scag %p notified", this);        
        }
        if (isStopping)
	    break;

        http_log_debug("Scag %p choosen for context %p", this, cx);

        switch (cx->action) {
        case PROCESS_REQUEST:
            http_log_debug("Scag %p: %p, call to processRequest()", this, cx);
            if (processor.processRequest(cx->getRequest())) {
                http_log_info("Scag %p: %p, request approved", this, cx);
                cx->getRequest().serialize();
                cx->action = SEND_REQUEST;
                manager.writers.process(cx);                               
                break;
            }
            else {
                http_log_info("Scag %p: %p, request denied", this, cx);
                cx->result = 503;          
                // no break, go to the case PROCESS_RESPONSE
            }
        case PROCESS_RESPONSE:
            {
                int status = cx->result;

                if (status == 0) {
                    http_log_debug("Scag %p: %p, call to processResponse()", this, cx);
                    if (processor.processResponse(cx->getResponse())) {
                        http_log_info("Scag %p: %p, response approved", this, cx);
                    }
                    else {
                        http_log_info("Scag %p: %p, response denied", this, cx);
                        status = 503;
                    }
                }

                if (status) {
                    cx->createFakeResponse(status);
                    http_log_warn("Scag %p: %p, status %d, fake response created",
                        this, cx, status);
                }
            }

            cx->getResponse().serialize();
            cx->action = SEND_RESPONSE;
            manager.writers.process(cx);       
            break;      
        case PROCESS_STATUS_RESPONSE:
            {
                bool delivered;

                if (cx->result) {
                    cx->createFakeResponse(cx->result);
                    http_log_warn("Scag %p: %p, status %d, fake response created",
                        this, cx, cx->result);
                    delivered = false;
                }
                else {
                    delivered = true;
                }
                http_log_debug("Scag %p: %p, call to statusResponse(%s)",
                        this, cx, delivered ? "true" : "false");
                processor.statusResponse(cx->getResponse(), delivered);
                delete cx;
            }
        }
    }

    http_log_debug( "Scag %p quit", this );

    return 0;
}

const char* ScagTask::taskName() {
    return "ScagTask";
}

void ScagTask::stop() {
    isStopping = true;
    manager.scags.taskMon.Lock();
    manager.scags.taskMon.notify();    
    manager.scags.taskMon.Unlock();
}

}}}
