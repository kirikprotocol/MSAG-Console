#include <xercesc/dom/DOM.hpp>

#include "util/config/Config.h"
#include "smsc/config/route/RouteConfig.h"
#include "util/xml/DOMTreeReader.h"

#include "smsc/common/TimeZoneMan.hpp"

namespace smsc{
namespace common{

TimeZoneManager* TimeZoneManager::instance=0;


static int getOffsetValue(const smsc::util::config::Config& cfg,const char* param)
{
  const char* val=cfg.getString(param);
  int rv;
  if(sscanf(val,"%d,",&rv)!=1)throw smsc::util::Exception("Invalid value for timezone:%s",val);
  return rv*60;
}


static std::string Unquote(const std::string& s)
{
  std::string rv=s;
  for(int i=0;i<rv.length();i++)if(rv[i]=='^')rv[i]='.';
  return rv;
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

  smsc::config::route::RouteConfig rcfg;
  rcfg.load(cfgRouteFileName.c_str());

  sync::MutexGuard mg(mtx);
  smsc::core::buffers::DigitalTree<int> tempTree;
  defaultOffset=getOffsetValue(cfg,"default_timezone");
  std::auto_ptr<CStrSet> masks(cfg.getChildStrParamNames("masks"));
  char buf[128];
  for(CStrSet::iterator it=masks->begin();it!=masks->end();it++)
  {
    smsc_log_info(log,"Loading mask:%s",Unquote(*it).c_str());
    snprintf(buf,128,"masks.%s",it->c_str());
    tempTree.Insert(AddrToString(Unquote(*it).c_str()).c_str(),getOffsetValue(cfg,buf));
  }
  std::auto_ptr<CStrSet> subjs(cfg.getChildStrParamNames("subjects"));
  for(CStrSet::iterator it=subjs->begin();it!=subjs->end();it++)
  {
    std::string subjName=Unquote(*it);
    smsc_log_info(log,"Loading subject:%s",subjName.c_str());
    snprintf(buf,128,"subjects.%s",it->c_str());
    try{
      smsc::config::route::Subject& subj=rcfg.getSubject(subjName.c_str());
      for(smsc::config::route::MaskVector::const_iterator sit=subj.getMasks().begin();sit!=subj.getMasks().end();sit++)
      {
        tempTree.Insert(AddrToString(sit->c_str()).c_str(),getOffsetValue(cfg,buf));
      }
    }catch(smsc::core::buffers::HashInvalidKeyException& e)
    {
      //throw smsc::util::Exception("Unknown subject:%s",subjName.c_str());
      smsc_log_warn(log,"Subject not found: '%s' - skipped",subjName.c_str());
    }
  }
  offsetMap.Swap(tempTree);
}

}//common
}//smsc
