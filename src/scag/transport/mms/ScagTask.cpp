#include "ScagTask.h"
#include "Managers.h"
#include "MmsCommand.h"
#include "MmsContext.h"

namespace scag { namespace transport { namespace mms {

ScagTask::ScagTask(MmsManagerImpl &m, MmsProcessor &p):manager(m), processor(p) {
  logger = Logger::getInstance("mms.scag");
}

const char* ScagTask::taskName() {
  return "MmsScagTask";
}

void ScagTask::stop() {
  isStopping = true;
  manager.scags.wakeTask();
}

int ScagTask::Execute() {
  smsc_log_debug(logger, "%p started", this);
  MmsContext *cx;
  for (;;) {
    while (!((cx = manager.scags.getFirst()) || isStopping)) {
      manager.scags.waitForContext();
    }
    if (isStopping) {
      break;
    }

    smsc_log_debug(logger, "%p choose context %p", this, cx);
    int st;
    switch (cx->action) {
    case PROCESS_REQUEST:
      smsc_log_debug(logger, "%p: %p, call to processRequest()", this, cx);
      cx->replaceTid();
      st = processor.processRequest(cx->getRequest());
      if (scag::re::STATUS_OK == st) {
        smsc_log_info(logger, "%p: %p, request aproved", this, cx);
        cx->action = SEND_REQUEST;
        cx->serializeRequest();
        manager.writers.process(cx);
        break;
      } else if (scag::re::STATUS_LONG_CALL == st || scag::re::STATUS_PROCESS_LATER == st) {
        break;
      }
      smsc_log_info(logger, "%p: %p, request denied", this, cx);
      cx->result = st;
      //go to PROCESS_RESPONSE

    case PROCESS_RESPONSE:
      if (cx->result == 0) {
        smsc_log_debug(logger, "%p: %p, call to processResponse()", this, cx);
        cx->replaceTid();
        st = processor.processResponse(cx->getResponse());
        if (scag::re::STATUS_OK == st) {
          smsc_log_info(logger, "%p: %p, response aproved", this, cx);
        } else {
          if (scag::re::STATUS_LONG_CALL == st || scag::re::STATUS_PROCESS_LATER == st) {
            break;
          }
          smsc_log_info(logger, "%p: %p, response denied", this, cx);
          cx->result = status::SERVICE_UNAVAILABLE;
        }
      }
      if (cx->result) {
        cx->createFakeResponse();
        smsc_log_warn(logger, "%p: %p, status %d, fake response created", this, cx, cx->result);
      }
      cx->action = SEND_RESPONSE;
      cx->serializeResponse();
      manager.writers.process(cx);
      break;

    case PROCESS_STATUS_RESPONSE:
      if(cx->result) {
          cx->createFakeResponse();
          smsc_log_warn(logger, "%p: %p, status %d, fake response created", this, cx, cx->result);
      }
      bool delivered = !cx->result;
      smsc_log_debug(logger, "%p: %p, call to statusResponse(%s)", this, cx, delivered ? "true" : "false");
      st = processor.statusResponse(cx->getResponse(), delivered);
      if(scag::re::STATUS_LONG_CALL == st || scag::re::STATUS_PROCESS_LATER == st) {
        break;
      }
      delete cx;
    }
  }

  if (cx) {
    delete cx;
  }

  smsc_log_debug(logger, "%p quit", this);

  return 0;
}

}//mms
}//transport
}//scag 
