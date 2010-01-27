#include <cstdlib>
#include "TrapRecord.h"

namespace scag2 {
namespace snmp {

TrapRecordQueue* TrapRecordQueue::instance_ = 0;

TrapRecordQueue::TrapRecordQueue()
{
    if (instance_ != 0) abort();
    instance_ = this;
}

TrapRecordQueue::~TrapRecordQueue() {
    if (instance_ == 0) abort();
    instance_ = 0;
}

}
}
