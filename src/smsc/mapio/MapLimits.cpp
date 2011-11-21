#include "MapLimits.hpp"
#include "util/config/Config.h"
#include "util/findConfigFile.h"
#include <xercesc/dom/DOM.hpp>
#include "util/xml/DOMTreeReader.h"
#include "logger/Logger.h"

namespace smsc{
namespace mapio{


MapLimits* MapLimits::instance=0;


void MapLimits::Init(const char* fn)
{
  instance=new MapLimits();
  instance->configFilename=smsc::util::findConfigFile(fn);
  instance->Reinit();
}

void MapLimits::parseUssdCodes(const char* name,const std::string& str,StringSet& codes)
{
  if(str.length())
  {
    std::string::size_type lastPos=0,nextPos;
    do{
      nextPos=str.find(',',lastPos);
      std::string code=str.substr(lastPos,nextPos==std::string::npos?nextPos:nextPos-lastPos);
      smsc_log_debug(log,"add '%s' ussd code:%s",name,code.c_str());
      codes.insert(code);
      lastPos=nextPos+1;
    }while(nextPos!=std::string::npos);
  }
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
    limitOutSRI=config.getInt("dlglimit.outsri");
    limitNIUSSD=config.getInt("dlglimit.niussd");

    try{
      sync::MutexGuard mg(mtx);
      noSriUssd.clear();
      std::string noSriUssdStr=config.getString("ussd.no_sri_codes");
      smsc_log_debug(log,"ussd.no_sri_codes=%s",noSriUssdStr.c_str());
      parseUssdCodes("no sri",noSriUssdStr,noSriUssd);
    }catch(...)
    {
      smsc_log_info(log,"ussd.no_sri_codes not found and disabled");
    }
    try{
      sync::MutexGuard mg(mtx);
      condSriUssd.clear();
      std::string condSriUssdStr=config.getString("ussd.cond_sri_codes");
      smsc_log_debug(log,"ussd.cond_sri_codes=%s",condSriUssdStr.c_str());
      parseUssdCodes("cond sri",condSriUssdStr,condSriUssd);
    }catch(...)
    {
      smsc_log_info(log,"ussd.cond_sri_codes not found and disabled");
    }
    try{
      sync::MutexGuard mg(mtx);
      atiUssd.clear();
      std::string atiUssdStr=config.getString("ussd.ati_codes");
      smsc_log_debug(log,"ussd.ati_codes=%s",atiUssdStr.c_str());
      parseUssdCodes("ati",atiUssdStr,atiUssd);
    }catch(...)
    {
      smsc_log_info(log,"ussd.ati_codes not found and disabled");
    }
    try{
      sync::MutexGuard mg(mtxOpenResp);
      openRespRealAddr.clear();
      std::string orUssdStr=config.getString("ussd.openRespRealAddr");
      smsc_log_debug(log,"ussd.openRespRealAddr=%s",orUssdStr.c_str());
      parseUssdCodes("open resp real addr",orUssdStr,openRespRealAddr);
    }catch(...)
    {
      smsc_log_info(log,"ussd.openRespRealAddr not found and disabled");
    }
    defaultUPM=upmOnlyStar;
    try {
      std::string mode=config.getString("ussd.defaultParsingMode");
      if(mode=="always")
      {
        defaultUPM=upmAlways;
      }else if(mode=="never")
      {
        defaultUPM=upmNever;
      }else if(mode=="onlyStar")
      {
        defaultUPM=upmOnlyStar;
      }else
      {
        smsc_log_warn(log,"parsing mode '%s' is invalid. using default: onlyStar");
      }
    } catch (std::exception& e)
    {
      smsc_log_info(log,"ussd.defaultParsingMode not found, default is onlyStar");
    }
    try{
      sync::MutexGuard mg(mtxUpm);
      ussdParseMode.clear();
      StringSet ss;
      std::string str=config.getString("ussd.parseAlways");
      smsc_log_debug(log,"ussd.parseAlways=%s",str.c_str());
      parseUssdCodes("parse always",str,ss);
      for(StringSet::iterator it=ss.begin(),end=ss.end();it!=end;++it)
      {
        ussdParseMode.insert(StrModeMap::value_type(*it,upmAlways));
      }
      try{
        ss.clear();
        str=config.getString("ussd.parseOnlyStar");
        smsc_log_debug(log,"ussd.parseOnlyStar=%s",str.c_str());
        parseUssdCodes("parse only star",str,ss);
        for(StringSet::iterator it=ss.begin(),end=ss.end();it!=end;++it)
        {
          ussdParseMode.insert(StrModeMap::value_type(*it,upmOnlyStar));
        }
      }catch(...)
      {
        smsc_log_info(log,"ussd.parseOnlyStar not found and disabled");
      }
      try{
        ss.clear();
        str=config.getString("ussd.parseNever");
        smsc_log_debug(log,"ussd.parseNever=%s",str.c_str());
        parseUssdCodes("parse never",str,ss);
        for(StringSet::iterator it=ss.begin(),end=ss.end();it!=end;++it)
        {
          ussdParseMode.insert(StrModeMap::value_type(*it,upmNever));
        }
      }catch(...)
      {
        smsc_log_info(log,"ussd.parseNever not found and disabled");
      }
    }catch(...)
    {
      smsc_log_info(log,"ussd.parseAlways not found and disabled");
    }
    try {
      smsOpenRespRealAddr=config.getBool("sms.openRespRealAddr");
    } catch (std::exception& e)
    {
      smsc_log_info(log,"sms.openRespRealAddr not found, disabled by default");
    }

    try{
      noSriSms=config.getBool("sms.noMoSri");
    }catch(std::exception& e)
    {
      smsc_log_info(log,"sms.noMoSti not found, disabled by default");
    }

    char buf[64];
    for(int i=0;i<maxCLevels;i++)
    {
      CLevelInfo li;
      li.dlgCount=0;
      sprintf(buf,"clevels.level%d.dialogsLimit",i+1);
      li.dlgLimit=config.getInt(buf);
      sprintf(buf,"clevels.level%d.failUpperLimit",i+1);
      li.failUpperLimit=config.getInt(buf);
      sprintf(buf,"clevels.level%d.failLowerLimit",i+1);
      li.failLowerLimit=config.getInt(buf);
      sprintf(buf,"clevels.level%d.okToLower",i+1);
      li.okToLower=config.getInt(buf);
      limitsOut[i]=li;
      smsc_log_info(log,"Limits for clevel=%d: dlg=%d, up=%d, low=%d, ok=%d",i+1,li.dlgLimit,li.failUpperLimit,li.failLowerLimit,li.okToLower);
    }
    smsc_log_info(log,"New limits loaded: in=%d, insri=%d, ussd=%d, out=%d, niussd=%d",
                  limitIn,limitInSRI,limitUSSD,limitOutSRI,limitNIUSSD);
  }else
  {
    throw smsc::util::Exception("Failed to parse config:'%s'",configFilename.c_str());
  }

}

}
}

