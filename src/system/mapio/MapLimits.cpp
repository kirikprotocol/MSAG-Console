#include "MapLimits.hpp"
#include "util/config/Config.h"
#include "util/findConfigFile.h"
#include <xercesc/dom/DOM.hpp>
#include "util/xml/DOMTreeReader.h"
#include "logger/Logger.h"

namespace smsc{
namespace system{
namespace mapio{


MapLimits* MapLimits::instance=0;


void MapLimits::Init(const char* fn)
{
  instance=new MapLimits();
  instance->configFilename=smsc::util::findConfigFile(fn);
  instance->Reinit();
}

void MapLimits::Reinit()
{
  using namespace xercesc;
  smsc::util::xml::DOMTreeReader reader;
  DOMDocument *document = reader.read(configFilename.c_str());
  if (document && document->getDocumentElement())
  {
    DOMElement *elem = document->getDocumentElement();
    smsc::util::config::Config config;
    config.parse(*elem);

    limitIn=config.getInt("dlglimit.in");
    limitInSRI=config.getInt("dlglimit.insri");
    limitUSSD=config.getInt("dlglimit.ussd");
    limitOut=config.getInt("dlglimit.out");
    limitNIUSSD=config.getInt("dlglimit.niussd");
    static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("map.lim");
    smsc_log_info(log,"New limits loaded: in=%d, insri=%d, ussd=%d, out=%d, niussd=%d",
                  limitIn,limitInSRI,limitUSSD,limitOut,limitNIUSSD);
  }else
  {
    throw smsc::util::Exception("Failed to parse config:'%s'",configFilename.c_str());
  }

}

}
}
}

