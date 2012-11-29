#include "Utility.h"
#include "util/TimeSource.h"

namespace eyeline {
namespace smpp {

msectime_type currentTimeMillis()
{
    typedef smsc::util::TimeSourceSetup::AbsMSec msecSource;
    return msectime_type(msecSource::getMSec());
}

}
}
