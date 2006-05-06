#include "ScagTask.h"
#include "Managers.h"
#include "HttpContext.h"

namespace scag { namespace transport { namespace http
{

ScagTask::ScagTask(HttpManager& m, HttpProcessor& p) : manager(m), processor(p)
{
    logger = Logger::getInstance("scag.http.scag");
}

int ScagTask::Execute()
{
    HttpContext *cx;

    smsc_log_debug(logger, "%p started", this);

    for (;;) {
        while (!((cx = manager.scags.getFirst()) || isStopping))
            manager.scags.waitForContext();

        if (isStopping)
            break;

        smsc_log_debug(logger, "%p choosen for context %p", this, cx);

        switch (cx->action) {
        case PROCESS_REQUEST:
            smsc_log_debug(logger, "%p: %p, call to processRequest()", this, cx);
            if (processor.processRequest(cx->getRequest())) {
                smsc_log_info(logger, "%p: %p, request approved", this, cx);
                cx->getRequest().serialize();
                smsc_log_debug(logger, "request %s", cx->getRequest().headers.c_str());
                cx->action = SEND_REQUEST;
                manager.writers.process(cx);                               
                break;
            }
            else {
                smsc_log_info(logger, "%p: %p, request denied", this, cx);
                cx->result = 503;          
                // no break, go to the case PROCESS_RESPONSE
            }
        case PROCESS_RESPONSE:
            {
                int status = cx->result;

                if (status == 0) {
                    smsc_log_debug(logger, "%p: %p, call to processResponse()", this, cx);
                    if (processor.processResponse(cx->getResponse())) {
                        smsc_log_info(logger, "%p: %p, response approved", this, cx);
                    }
                    else {
                        smsc_log_info(logger, "%p: %p, response denied", this, cx);
                        status = 503;
                    }
                }

                if (status) {
                    cx->createFakeResponse(status);
                    smsc_log_warn(logger, "%p: %p, status %d, fake response created",
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
                    smsc_log_warn(logger, "%p: %p, status %d, fake response created",
                        this, cx, cx->result);
                    delivered = false;
                }
                else {
                    delivered = true;
                }
                smsc_log_debug(logger, "%p: %p, call to statusResponse(%s)",
                        this, cx, delivered ? "true" : "false");
                processor.statusResponse(cx->getResponse(), delivered);
                delete cx;
            }
        }
    }

    if (cx)
        delete cx;

    smsc_log_debug(logger, "%p quit", this);

    return 0;
}

const char* ScagTask::taskName() {
    return "ScagTask";
}

void ScagTask::stop() {
    isStopping = true;

    manager.scags.wakeTask();
}

}}}
