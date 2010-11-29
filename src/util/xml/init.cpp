#include "init.h"
#include <memory>
#include <vector>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/TransENameMap.hpp>

#include <core/synchronization/Mutex.hpp>
#include <util/xml/SmscTranscoder.h>
#include <util/xml/utilFunctions.h>
#include <util/xml/IconvRegistry.h>

#include <stdio.h>

namespace smsc {
namespace util {
namespace xml {

using namespace xercesc;
using namespace smsc::core::synchronization;

Mutex mutex;
bool isXercesInitialized = false;
namespace {
std::vector<const XMLCh*> encnames;
}

void addEncoding(const char * const encodingName)
{
  std::auto_ptr<const XMLCh> xenc(XMLString::transcode(encodingName));
  XMLTransService::addEncoding(xenc.get(), new ENameMapFor<SmscTranscoder>(xenc.get()));
  encnames.push_back(xenc.release());
}

void initXerces()
{
  MutexGuard guard(mutex);
  if (!isXercesInitialized)
  {
    XMLPlatformUtils::Initialize();
    addEncoding("CP1251");
    addEncoding("CP-1251");
    addEncoding("ANSI-1251");
    addEncoding("ANSI1251");
    addEncoding("ISO8859-5");
    addEncoding("ISO-8859-5");
    addEncoding("WINDOWS-1251");
    addEncoding("KOI8-R");
    addEncoding("UTF8");
    addEncoding("UTF-8");
    addEncoding(getLocalEncoding());
    isXercesInitialized = true;
  }
}

void TerminateXerces()
{
  MutexGuard guard(mutex);
  if (isXercesInitialized)
  {
    XMLPlatformUtils::Terminate();
    clearIconvs();
    for ( std::vector< const XMLCh* >::iterator i = encnames.begin();
          i != encnames.end(); ++i ) {
        delete *i;
    }
    encnames.clear();
    isXercesInitialized = false;
  }
}

}
}
}
