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
      s.setDestinationSmeId("world");

      timeval st,ed;
      for(int i=0;i<30000;i++)
      {
        if((i%1000)==0)
        {
          printf("%d\n",i);
        }
        if(i==10000)gettimeofday(&st,0);
        if((i%3)==0)
        {
          s.setSourceSmeId("hello");
          s.setRouteId("route");
        }
        else if((i%3)==1)
        {
          s.setSourceSmeId("hello1");
          s.setRouteId("route1");
        }
        else if((i%3)==2)
        {
          s.setSourceSmeId("hello2");
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
      gettimeofday(&ed,0);
      int diff=(ed.tv_sec-st.tv_sec)*1000;
      diff+=(ed.tv_usec-st.tv_usec)/1000;
      printf("insertion time:%d\n",diff);
    }else
    {
      ParamArray p;
      Param p1;
      //p1.type=Param::tDstAddress;
      //p1.sValue=".1.1.79029025800";
      p.Push(p1);
      p1.type=Param::tRouteId;
      p1.sValue="InfoSme -> T2";
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
