#include <stdio.h>
#include <exception>
#include "SmsIndex.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

using namespace smsc::store::index;
using namespace smsc::util::config;

int main(int argc,char* argv[])
{
  Logger::Init();
  try{
    SmsIndex idx(".");

    Manager::init("config.xml");
    ConfigView cv(Manager::getInstance());

    idx.Init(cv.getSubConfig("index"));

    if(argc==1)
    {
      SMS s;

      timeval st,ed;
      for(int i=0;i<10000;i++)
      {
        char buf[64];
        sprintf(buf,".0.1.%d",1+(i+1)%10);
        s.setOriginatingAddress(buf);
        sprintf(buf,".0.1.100%d",1+(i+6)%10);
        s.setDestinationAddress(buf);
        s.setDealiasedDestinationAddress(buf);
        s.lastTime=time(NULL);

        if((i%3)==0)
        {
          s.setSourceSmeId("hello2");
          s.setDestinationSmeId("qqq");
          s.setRouteId("route");
        }
        else if((i%3)==1)
        {
          s.setSourceSmeId("qqq");
          s.setDestinationSmeId("world");
          s.setRouteId("route1");
        }
        else if((i%3)==2)
        {
          s.setSourceSmeId("hello2");
          s.setDestinationSmeId("world");
          s.setRouteId("route2");
          s.setDestinationAddress(".0.1.7000");
          s.setDealiasedDestinationAddress(".0.1.7000");
        }
        idx.IndexateSms(".",100+i,100000+i,s);
      }
      idx.EndTransaction();
      for(int i=10000;i<20000;i++)
      {
        char buf[64];
        sprintf(buf,".0.1.%d",1+(i+1)%10);
        s.setOriginatingAddress(buf);
        sprintf(buf,".0.1.100%d",1+(i+6)%10);
        s.setDestinationAddress(buf);
        s.setDealiasedDestinationAddress(buf);
        s.lastTime=time(NULL);

        if((i%3)==0)
        {
          s.setSourceSmeId("hello2");
          s.setDestinationSmeId("qqq");
          s.setRouteId("route");
        }
        else if((i%3)==1)
        {
          s.setSourceSmeId("qqq");
          s.setDestinationSmeId("world");
          s.setRouteId("route1");
        }
        else if((i%3)==2)
        {
          s.setSourceSmeId("hello2");
          s.setDestinationSmeId("world");
          s.setRouteId("route2");
          s.setDestinationAddress(".0.1.7000");
          s.setDealiasedDestinationAddress(".0.1.7000");
        }
        idx.IndexateSms(".",100+i,100000+i,s);
      }
      idx.RollBack();

    }else
    {
      ParamArray p;
      Param p1;
      p1.type=Param::tSmeId;
      p1.iValue=15000;
      p.Push(p1);
      /*
      p1.type=Param::tSrcSmeId;
      p1.sValue="hello2";
      p.Push(p1);
      p1.type=Param::tDstSmeId;
      p1.sValue="qqq";
      p.Push(p1);
      p1.type=Param::tRouteId;
      p1.sValue="route";
      p.Push(p1);

      p1.type=Param::tDstAddress;
      p1.sValue=".0.1.7001";
      //p.Push(p1);
      */
      ResultArray res;
      int c=idx.QuerySms(".",p,res);
      printf("%d\n",c);

      for(int i=0;i<res.Count();i++)
      {
        printf("%lld-%u\n",res[i].offset,res[i].lastTryTime);
      }
    }
  }catch(std::exception& e)
  {
    printf("ex:%s\n",e.what());
  }

  return 0;
}
