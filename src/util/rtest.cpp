#include <stdio.h>
#include "util/config/route/RouteConfig.h"
#include "util/config/smeman/SmeManConfig.h"
#include "router/route_manager.h"
#include "util/Timer.hpp"
#include "smeman/smeman.h"
#include "util/regexp/RegExp.hpp"
#include "logger/Logger.h"
#include "alias/aliasman.h"
#include "util/config/alias/aliasconf.h"
#include <string>
#include <vector>
#include "core/buffers/File.hpp"
#include "core/buffers/Hash.hpp"

using namespace std;

using namespace smsc::router;
using namespace smsc::smeman;
using namespace smsc::alias;
using namespace smsc::util::config::alias;
using namespace smsc::core::buffers;

namespace smsc{
namespace system{
extern void loadRoutes(RouteManager* rm,const smsc::util::config::route::RouteConfig& rc,bool traceit=false);
}
}


void reloadAliases(AliasManager* aliaser,const AliasConfig& cfg)
{
  {
    smsc::util::config::alias::AliasConfig::RecordIterator i =
                                cfg.getRecordIterator();
    while(i.hasRecord())
    {
      smsc::util::config::alias::AliasRecord *rec;
      i.fetchNext(rec);
      __trace2__("adding %20s %20s",rec->addrValue,rec->aliasValue);
      smsc::alias::AliasInfo ai;
      ai.addr = smsc::sms::Address(
        strlen(rec->addrValue),
        rec->addrTni,
        rec->addrNpi,
        rec->addrValue);
      ai.alias = smsc::sms::Address(
        strlen(rec->aliasValue),
        rec->aliasTni,
        rec->aliasNpi,
        rec->aliasValue);
      ai.hide = rec->hide;
      aliaser->addAlias(ai);
    }
    aliaser->commit();
  }
}

void split(const string& str,char delim,vector<string>& out)
{
  int old=-1;
  int pos=0;
  while(pos!=string::npos)
  {
    pos=str.find(delim,pos+1);
    out.push_back(str.substr(old+1,pos-old-1));
    old=pos;
  }
}


int main(int argc,char* argv[])
{
  if(argc!=2)return 0;

  smsc::logger::Logger::Init();

  RouteManager rm;
  smsc::smeman::SmeManager smeman;
  AliasManager am;

  smsc::util::config::alias::AliasConfig acfg;

  acfg.load("aliases.xml");

  reloadAliases(&am,acfg);


  smsc::util::config::smeman::SmeManConfig smemanconfig;
  smemanconfig.load("sme.xml");


  {
    smsc::util::config::smeman::SmeManConfig::RecordIterator i=smemanconfig.getRecordIterator();
    using namespace smsc::util::regexp;
    RegExp re;
    while(i.hasRecord())
    {
      smsc::util::config::smeman::SmeRecord *rec;
      i.fetchNext(rec);
      SmeInfo si;
      if(rec->rectype==smsc::util::config::smeman::SMPP_SME)
      {
        si.typeOfNumber=rec->recdata.smppSme.typeOfNumber;
        si.numberingPlan=rec->recdata.smppSme.numberingPlan;
        si.interfaceVersion=rec->recdata.smppSme.interfaceVersion;
        si.rangeOfAddress=rec->recdata.smppSme.addrRange;
        si.systemType=rec->recdata.smppSme.systemType;
        si.password=rec->recdata.smppSme.password;
        si.systemId=rec->smeUid;
        si.timeout = rec->recdata.smppSme.timeout;
        si.wantAlias = rec->recdata.smppSme.wantAlias;
        si.forceDC = rec->recdata.smppSme.forceDC;
        si.proclimit=rec->recdata.smppSme.proclimit;
        si.schedlimit=rec->recdata.smppSme.schedlimit;
        si.receiptSchemeName= rec->recdata.smppSme.receiptSchemeName;
        if(si.rangeOfAddress.length() && !re.Compile(si.rangeOfAddress.c_str(),OP_OPTIMIZE|OP_STRICT))
        {
          //smsc_log_error(log, "Failed to compile rangeOfAddress for sme %s",si.systemId.c_str());
        }
        //__trace2__("INIT: addSme %s(to=%d,wa=%s)",si.systemId.c_str(),si.timeout,si.wantAlias?"true":"false");
        //si.hostname=rec->recdata->smppSme.
        si.disabled=rec->recdata.smppSme.disabled;
        using namespace smsc::util::config::smeman;
        switch(rec->recdata.smppSme.mode)
        {
          case MODE_TX:si.bindMode=smeTX;break;
          case MODE_RX:si.bindMode=smeRX;break;
          case MODE_TRX:si.bindMode=smeTRX;break;
        };

        try{
          smeman.addSme(si);
        }catch(...)
        {
          //smsc_log_warn(log, "UNABLE TO REGISTER SME:%s",si.systemId.c_str());
        }
      }
    }
  }

  smsc::util::config::route::RouteConfig rc;

  rm.assign(&smeman);


  rc.load("routes.xml");
  smsc::system::loadRoutes(&rm,rc);

  int mapidx=smeman.lookup("MAP_PROXY");

  File f;
  f.ROpen(argv[1]);
  string l;
  Hash<string> addr2sme;

  while(f.ReadLine(l))
  {
    vector<string> out;
    split(l,'|',out);


    Address src(out[1].c_str());
    Address dst(out[2].c_str());

    Address ddst;
    if(am.AliasToAddress(dst,ddst))
    {
      dst=ddst;

    }

    smsc::smeman::SmeProxy* prx;
    int idx;
    RouteInfo ri;
    bool ok;

    if(out[1].find("ussd")==string::npos)
    {
      ok=rm.lookup(mapidx,src,dst,prx,&idx,&ri);
    }else if(addr2sme.Exists(out[1].c_str()))
    {
      int idx=smeman.lookup(addr2sme.Get(out[1].c_str()));
      ok=rm.lookup(idx,src,dst,prx,&idx,&ri);
    }else
    {
      ok=rm.lookup(src,dst,prx,&idx,&ri);
    }

    if(ok)
    {
      if(ri.billing)
      {
        printf("%s|%s|%s|%s|%s|%s|%s\n",out[0].c_str(),out[1].c_str(),out[2].c_str(),out[3].c_str(),ri.routeId.c_str(),ri.srcSmeSystemId.c_str(),ri.smeSystemId.c_str());
      }
      addr2sme.Insert(out[2].c_str(),ri.smeSystemId);
    }else
    {
      fprintf(stderr,"NOT FOUND: %s->%s\n",out[1].c_str(),out[2].c_str());
    }
  }
  //printf("finished\n");
  return 0;
}
