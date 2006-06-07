#include "system/common/TimeZoneMan.hpp"
#include "util/config/Config.h"
#include "util/config/route/RouteConfig.h"
#include <xercesc/dom/DOM.hpp>
#include "util/xml/DOMTreeReader.h"

namespace smsc{
namespace system{
namespace common{

TimeZoneManager* TimeZoneManager::instance=0;


static int getOffsetValue(const smsc::util::config::Config& cfg,const char* param)
{
  const char* val=cfg.getString(param);
  int rv;
  if(sscanf(val,"%d,",&rv)!=1)throw smsc::util::Exception("Invalid value for timezone:%s",val);
  return rv*60;
}

void TimeZoneManager::Reload()
{
  using namespace xercesc;
  using namespace smsc::util::xml;
  using namespace smsc::util::config;
  Config cfg;
  DOMTreeReader reader;
  DOMDocument *document = reader.read(cfgTzFileName.c_str());
  if (document && document->getDocumentElement())
  {
    DOMElement *elem = document->getDocumentElement();
    cfg.parse(*elem);
  }
  else
  {
    throw smsc::util::Exception("Parse result is null:%s",cfgTzFileName.c_str());
  }

  route::RouteConfig rcfg;
  rcfg.load(cfgRouteFileName.c_str());

  sync::MutexGuard mg(mtx);
  offsetMap.clear();
  defaultOffset=getOffsetValue(cfg,"default_timezone");
  std::auto_ptr<CStrSet> masks(cfg.getChildStrParamNames("masks"));
  char buf[128];
  for(CStrSet::iterator it=masks->begin();it!=masks->end();it++)
  {
    snprintf(buf,128,"masks.%s",it->c_str());
    offsetMap.insert(OffsetMap::value_type(it->c_str(),getOffsetValue(cfg,buf)));
  }
  std::auto_ptr<CStrSet> subjs(cfg.getChildStrParamNames("subjects"));
  for(CStrSet::iterator it=subjs->begin();it!=subjs->end();it++)
  {
    snprintf(buf,128,"subjects.%s",it->c_str());
    try{
      route::Subject& subj=rcfg.getSubject(it->c_str());
      for(route::MaskVector::const_iterator sit=subj.getMasks().begin();sit!=subj.getMasks().end();sit++)
      {
        offsetMap.insert(OffsetMap::value_type(sit->c_str(),getOffsetValue(cfg,buf)));
      }
    }catch(smsc::core::buffers::HashInvalidKeyException& e)
    {
      throw smsc::util::Exception("Unknown subject:%s",it->c_str());
    }
  }
}

}//common
}//system
}//smsc
