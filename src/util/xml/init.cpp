#include "init.h"
#include <memory>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/TransENameMap.hpp>

#include <core/synchronization/Mutex.hpp>
#include <util/xml/SmscTranscoder.h>
#include <util/xml/utilFunctions.h>

#include <stdio.h>

namespace smsc {
namespace util {
namespace xml {

using namespace xercesc;
using namespace smsc::core::synchronization;

Mutex mutex;
bool isXercesInitialized = false;

void addEncoding(const char * const encodingName)
{
  std::auto_ptr<const XMLCh> xenc(XMLString::transcode(encodingName));
  XMLTransService::addEncoding(xenc.get(), new ENameMapFor<SmscTranscoder>(xenc.get()));
  xenc.release();
}

void initXerces()
{
  MutexGuard guard(mutex);
	if (!isXercesInitialized)
	{
		XMLPlatformUtils::Initialize();
    //addEncoding("CP1251");  //TODO implement support of multiple encodings
    //addEncoding("ANSI-1251");
    //addEncoding("ISO8859-5");
    //addEncoding("WINDOWS-1251");
    addEncoding(getLocalEncoding());
	}
}

void TerminateXerces()
{
  MutexGuard guard(mutex);
	if (isXercesInitialized)
	{
		XMLPlatformUtils::Terminate();
	}
}

}
}
}
