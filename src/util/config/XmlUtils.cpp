#include "XmlUtils.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMString.hpp>

namespace smsc   {
namespace util   {
namespace config {

static bool isXercesInitialized = false;

DOMString & createDOMString(const char * const str)
{
	if (!isXercesInitialized)
	{
		XMLPlatformUtils::Initialize();
	}
	DOMString * result = new DOMString(str);
	return *result;
}

}
}
}

