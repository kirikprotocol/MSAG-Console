#include "logger/Appender.h"

#include "util/cstrings.h"

namespace smsc {
namespace logger {

using namespace smsc::util;

Appender::Appender(const char * const name)
	:name(cStringCopy(name))
{
}

}
}
