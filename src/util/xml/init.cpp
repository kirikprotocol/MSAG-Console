#include "init.h"
#include <xercesc/util/PlatformUtils.hpp>

namespace smsc {
namespace util {
namespace xml {

bool isXercesInitialized = false;

void initXerces()
{
	if (!isXercesInitialized)
	{
		XMLPlatformUtils::Initialize();
	}
}

}
}
}
