#include "ScagTask.h"
#include "Managers.h"
#include "HttpContext.h"
#include "scag/re/RuleStatus.h"

namespace scag { namespace transport { namespace http
{

ScagTask::ScagTask(HttpManagerImpl& m, HttpProcessor& p) : manager(m), processor(p)
{
    logger = Logger::getInstance("scag.http.scag");
}

int ScagTask::Execute()
{
    int st;
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
            st = processor.processRequest(cx->getRequest());
            if (st == scag::re::STATUS_OK)
            {
                smsc_log_info(logger, "%p: %p, request approved", this, cx);
                cx->getRequest().serialize();
//                smsc_log_debug(logger, "request %s", cx->getRequest().headers.c_str());
                cx->action = SEND_REQUEST;
                manager.writerProcess(cx);                               
                break;
            }
            else if(st == scag::re::STATUS_LONG_CALL || st == scag::re::STATUS_PROCESS_LATER)
				break;
            cx->requestFailed = cx->getRequest().isFailedBeforeSessionCreate();
            smsc_log_info(logger, "%p: %p, request denied", this, cx);
            cx->result = 503;          
            // no break, go to the case PROCESS_RESPONSE
        case PROCESS_RESPONSE:
            {
                int status = cx->result;

                if (status)
                {
                    cx->createFakeResponse(status);
                    smsc_log_warn(logger, "%p: %p, status %d, fake response created", this, cx, status);
                    status = 0;
                }

                if (!cx->requestFailed)
                {
                    smsc_log_debug(logger, "%p: %p, call to processResponse()", this, cx);
                    st = processor.processResponse(cx->getResponse());
                    if (st == scag::re::STATUS_OK)
                        smsc_log_info(logger, "%p: %p, response approved", this, cx);
                    else
                    {
                        if(st == scag::re::STATUS_LONG_CALL || st == scag::re::STATUS_PROCESS_LATER)
                            break;
                        
                        smsc_log_info(logger, "%p: %p, response denied", this, cx);
                        status = 503;
                    }
                }
                if (status)
                {
                    cx->createFakeResponse(status);
                    smsc_log_warn(logger, "%p: %p, status %d, fake response created", this, cx, status);
                }
/*
                if (status == 0)
                {
                    smsc_log_debug(logger, "%p: %p, call to processResponse()", this, cx);
                    st = processor.processResponse(cx->getResponse());
                    if (st == scag::re::STATUS_OK)
                        smsc_log_info(logger, "%p: %p, response approved", this, cx);
                    else
                    {
                        if(st == scag::re::STATUS_LONG_CALL || st == scag::re::STATUS_PROCESS_LATER)
                            break;

                        smsc_log_info(logger, "%p: %p, response denied", this, cx);
                        status = 503;
                    }
                }
                if (status)
                {
                    cx->createFakeResponse(status);
                    smsc_log_warn(logger, "%p: %p, status %d, fake response created", this, cx, status);
                }
*/
            }

            cx->getResponse().serialize();
            cx->action = SEND_RESPONSE;
            manager.writerProcess(cx);       
            break;      
        case PROCESS_STATUS_RESPONSE:
            {
              if (cx->requestFailed) {
                smsc_log_warn(logger, "%p: %p, do not call to statusResponse(). request failed before session create", this, cx);
                delete cx;
                break;
              }
                if(cx->result)
                {
                    cx->createFakeResponse(cx->result);
                    smsc_log_warn(logger, "%p: %p, status %d, fake response created", this, cx, cx->result);
                }
                bool delivered = !cx->result;
                    
                smsc_log_debug(logger, "%p: %p, call to statusResponse(%s)", this, cx, delivered ? "true" : "false");
                st = processor.statusResponse(cx->getResponse(), delivered);

                if(st == scag::re::STATUS_LONG_CALL || st == scag::re::STATUS_PROCESS_LATER)
                    break;
                delete cx;
            }
        }
    }

    if(cx) delete cx;

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
