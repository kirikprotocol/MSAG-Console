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
  static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("map.lim");
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
    
    try{
      sync::MutexGuard mg(mtx);
      noSriUssd.clear();
      std::string noSriUssdStr=config.getString("ussd.no_sri_codes");
      smsc_log_debug(log,"ussd.no_sri_codes=%s",noSriUssdStr.c_str());
      if(noSriUssdStr.length())
      {
        std::string::size_type lastPos=0,nextPos;
        do{
          nextPos=noSriUssdStr.find(',',lastPos);
          std::string code=noSriUssdStr.substr(lastPos,nextPos==std::string::npos?nextPos:nextPos-lastPos);
          smsc_log_debug(log,"add no sri ussd code:%s",code.c_str());
          noSriUssd.insert(code);
          lastPos=nextPos+1;
        }while(nextPos!=std::string::npos);
      }
    }catch(...)
    {
      smsc_log_info(log,"ussd.no_sri_codes not found and disabled");
    }
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

