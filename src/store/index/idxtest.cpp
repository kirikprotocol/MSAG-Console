#include <stdio.h>
#include <exception>
#include "SmsIndex.hpp"

using namespace smsc::store::index;

int main(int argc,char* argv[])
{
  try{
    SmsIndex idx(".");
    if(argc==1)
    {
      SMS s;
      s.setSourceSmeId("hello");
      s.setDestinationSmeId("world");

      for(int i=0;i<30000;i++)
      {
        if((i%3)==0)
          s.setRouteId("route");
        else if((i%3)==1)
          s.setRouteId("route1");
        else if((i%3)==2)
          s.setRouteId("route2");
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
      p1.type=Param::tRouteId;
      p1.sValue="route";
      p.Push(p1);
      p1.type=Param::tSrcSmeId;
      p1.sValue="hello";
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
