#include "ScagTask.h"
#include "Managers.h"
#include "scag/re/base/RuleStatus2.h"

namespace scag2 {
namespace transport {
namespace http {

ScagTask::ScagTask(HttpManagerImpl& m, HttpProcessor& p) : manager(m), processor(p)
{
    logger = Logger::getInstance("http.scag");
}

int ScagTask::Execute()
{
    int st;
    HttpContext *cx;

    smsc_log_debug(logger, "%p started", this);

//    for (;;) {
//	    while (!((cx = manager.scags.getFirst()) || isStopping))
    while ( !isStopping ) {
        cx = manager.scags.getFirst();
    	if ( !cx ) {
            manager.scags.waitForContext();
            continue;
    	}
    	if ( !cx )
    		smsc_log_error(logger, "%p, HttpContext %p SOMETHING WRONG!", this, cx );
/*
        { //debug
        	uint32_t req, resp, stat, lcm;
        	manager.scags.queueLen(req, resp, stat, lcm);
            smsc_log_debug(logger, "%p ScagTask::Execute (%u %u %u %u) stop:%s cx %p act:%s command %p", this,req,resp,stat,lcm,(isStopping?"y":"n"),
            		cx, (cx?HttpContext::actionName(cx->action):"Undefined"), (cx?cx->command:NULL));
        }
*/
//        if (isStopping) break;

        switch (cx->action) {
        case PROCESS_REQUEST:
            st = processor.processRequest(cx->getRequest());
            if (st == re::STATUS_OK)
            {
//              smsc_log_info(logger, "%p: %p, request approved", this, cx);
              cx->getRequest().checkConnectionFields();
              cx->getRequest().serialize();
              cx->action = SEND_REQUEST;
              manager.writerProcess(cx);                               
              break;
            }
            else if(st == re::STATUS_LONG_CALL || st == re::STATUS_PROCESS_LATER) {
              break;
            }
            else if (st == re::STATUS_FAILED) {
              cx->requestFailed = cx->getRequest().isFailedBeforeSessionCreate();
              int processResult = cx->getRequest().trc.result;
              smsc_log_info(logger, "%p: %p, request denied, status %d", this, cx, processResult);
              cx->result = processResult > 0 ? processResult : 503;          
              cx->action = PROCESS_RESPONSE;
              // no break, go to the case PROCESS_RESPONSE
            }
        case PROCESS_RESPONSE:
            if (!cx->requestFailed)
            {
                if (cx->result) {
                  cx->createFakeResponse(cx->result);
                  smsc_log_warn(logger, "%p: %p, status %d, fake response created", this, cx, cx->result);
                  cx->result = 0; // Clear status for LCM, fake resp already created
                }
                st = processor.processResponse(cx->getResponse());
                if (st == re::STATUS_OK) {
                  smsc_log_info(logger, "%p: %p, response approved", this, cx);
                }
                else if (st == re::STATUS_LONG_CALL || st == re::STATUS_PROCESS_LATER) {
                  break;
                }
                else {
                  int processResult = cx->getResponse().trc.result;
                  smsc_log_info(logger, "%p: %p, response denied, status %d", this, cx, processResult);
                  cx->result = processResult > 0 ? processResult : 503;          
                }
            }  
            if (cx->result)
            {
                cx->createFakeResponse(cx->result);
                smsc_log_warn(logger, "%p: %p, status %d, fake response created", this, cx, cx->result);
            }

            cx->getResponse().checkConnectionFields();
            cx->getResponse().serialize();
            cx->action = SEND_RESPONSE;
            manager.writerProcess(cx);       
            break;      
        case PROCESS_STATUS_RESPONSE:
        	try
            {
                if (cx->requestFailed || cx->getResponse().isFake()) {
                  smsc_log_warn(logger, "%p, case PROCESS_STATUS_RESPONSE, failed request: HttpContext %p delete", this, cx);
                  delete cx;
                  cx = NULL;
                  break;
                }

                bool delivered = !cx->result;
                st = processor.statusResponse(cx->getResponse(), delivered);

                if(st == re::STATUS_LONG_CALL || st == re::STATUS_PROCESS_LATER)
                    break;
//                smsc_log_debug(logger, "%p, case PROCESS_STATUS_RESPONSE, normal HttpContext %p delete", this, cx);
                delete cx;
                cx = NULL;
            }
        	catch(...) {
                smsc_log_error(logger, "%p, Unresolved exception when case PROCESS_STATUS_RESPONSE, delete HttpContext %p", this, cx);
        	}
        	break;
        default:
            smsc_log_error(logger, "%p, HttpContext %p action=%s (wrong value)", this, cx, (cx?HttpContext::actionName(cx->action):"Undefined"));
            break;
        } // switch action
    }

    if (cx) {
    	delete cx;
    }

    smsc_log_debug(logger, "%p quit", this);

    return 0;
}

const char* ScagTask::taskName() {
	sprintf(bufName, "ScagTask[%p]", this);
    return bufName; // "ScagTask";
}

void ScagTask::stop() {
    isStopping = true;

    manager.scags.wakeTask();
}

}}}
