#ifndef __smsc_statemachine_util_hpp__
#define __smsc_statemachine_util_hpp__

#include "util/int.h"

namespace smsc{

class Smsc;

namespace statemachine{

struct TaskGuard{
  Smsc *smsc;
  uint32_t dialogId;
  uint32_t uniqueId;
  bool active;
  TaskGuard()
  {
    smsc=0;
    dialogId=0;
    uniqueId=0;
    active=false;
  }
  ~TaskGuard();
};

class ExtractPartFailedException{};

}
}

#endif
