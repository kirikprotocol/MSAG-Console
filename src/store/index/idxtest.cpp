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
      for(int i=0;i<100;i++)
      {
        if((i%3)==0)
        {
          s.setSourceSmeId("hello");
          s.setDestinationSmeId("qqq");
          s.setRouteId("route");
        }
        else if((i%3)==1)
        {
          s.setSourceSmeId("qqq");
          s.setDestinationSmeId("hello");
          s.setRouteId("route1");
        }
        else if((i%3)==2)
        {
          s.setSourceSmeId("hello2");
          s.setDestinationSmeId("world");
          s.setRouteId("route2");
        }
        char buf[64];
        sprintf(buf,".0.1.%d",i+1);
        s.setOriginatingAddress(buf);
        sprintf(buf,".0.1.%d",i+100001);
        s.setDestinationAddress(buf);
        s.lastTime=time(NULL);
        idx.IndexateSms(".",100+i,100000+i,s);
      }
    }else
    {
      ParamArray p;
      Param p1;
      //p1.type=Param::tDstAddress;
      //p1.sValue=".1.1.79029025800";
      p.Push(p1);
      p1.type=Param::tSmeId;
      p1.sValue="qqq";
      p.Push(p1);
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
