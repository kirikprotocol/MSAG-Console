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
            if (st == re::STATUS_OK)
            {
              smsc_log_info(logger, "%p: %p, request approved", this, cx);
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
            smsc_log_debug(logger, "%p: %p, call to processResponse()", this, cx);
            if (!cx->requestFailed)
            {
                if (cx->result) {
                  cx->createFakeResponse(cx->result);
                  smsc_log_warn(logger, "%p: %p, status %d, fake response created", this, cx, cx->result);
                  cx->result = 0; // Clear status for LCM, fake resp already created
                }
				smsc_log_debug(logger, "ScagTask::Execute1. f:%d pp:%d h:%d c:%d w:%d d:%p sz:%d", cx->flags, cx->parsePosition,
					cx->command->getMessageHeaders().size(), cx->command->getContentLength(),
					cx->command->content.GetPos(), cx->command->content.get(), cx->command->content.getSize());
                st = processor.processResponse(cx->getResponse());
				smsc_log_debug(logger, "ScagTask::Execute2. f:%d pp:%d h:%d c:%d w:%d d:%p sz:%d", cx->flags, cx->parsePosition,
					cx->command->getMessageHeaders().size(), cx->command->getContentLength(),
					cx->command->content.GetPos(), cx->command->content.get(), cx->command->content.getSize());
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

            cx->getResponse().serialize();
			smsc_log_debug(logger, "ScagTask::Execute3. f:%d pp:%d h:%d c:%d w:%d d:%p sz:%d", cx->flags, cx->parsePosition,
				cx->command->getMessageHeaders().size(), cx->command->getContentLength(),
				cx->command->content.GetPos(), cx->command->content.get(), cx->command->content.getSize());
            cx->action = SEND_RESPONSE;
            manager.writerProcess(cx);       
            break;      
        case PROCESS_STATUS_RESPONSE:
            {
                if (cx->requestFailed || cx->getResponse().isFake()) {
                  smsc_log_warn(logger, "%p: %p, do not call to statusResponse(). fake status_response skipped", this, cx);
                  delete cx;
                  break;
                }

                bool delivered = !cx->result;
                smsc_log_debug(logger, "%p: %p, call to statusResponse(%s)", this, cx, delivered ? "true" : "false");
                st = processor.statusResponse(cx->getResponse(), delivered);

                if(st == re::STATUS_LONG_CALL || st == re::STATUS_PROCESS_LATER)
                    break;
                delete cx;
            }
        default:
            break;
        } // switch action
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
