#include <sms/sms.h>
#include "store/FileStorage.h"
#include "core/buffers/File.hpp"
#include "core/buffers/Hash.hpp"
#include "logger/Logger.h"
#include "util/sleep.h"

using namespace smsc::core::buffers;
using namespace smsc::sms;
using namespace smsc::util;
using namespace smsc::store;

static std::string dir;

struct Counters{
  Counters():ok(0),pok(0),fail(0),pfail(0){}
  int ok;
  int pok;
  int fail;
  int pfail;
};

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  if(argc==1)
  {
    printf("Usage: %s path_to_arc_dir\n",argv[0]);
    return 0;
  }

  std::vector<std::string> routes;

  if(File::Exists("routes.txt"))
  {
    File f;
    f.ROpen("routes.txt");
    std::string s;
    while(f.ReadLine(s))
    {
      routes.push_back(s);
    }
  }

  /*
  const char* routes[]={
      "plmn.abk > Anybody",
      "plmn.abk > plmn.abk",
      "plmn.brn > Anybody",
      "plmn.brn > plmn.brn",
      "plmn.ga > Anybody",
      "plmn.ga > plmn.ga",
      "plmn.kem > Anybody",
      "plmn.kem > plmn.kem",
      "plmn.ksn > Anybody",
      "plmn.ksn > plmn.ksn",
      "plmn.nor > Anybody",
      "plmn.nor > plmn.nor",
      "plmn.nsk > Anybody",
      "plmn.nsk > plmn.nsk",
      "plmn.oms > Anybody",
      "plmn.oms > plmn.oms",
      "plmn.tom > Anybody",
      "plmn.tom > plmn.tom",
  };
  */

  Hash<Counters> r;
  for(size_t i=0;i<routes.size();i++)
  {
    r.Insert(routes[i].c_str(),Counters());
  }


  try{
    dir=argv[1];
    if(dir.length() && *dir.rbegin()!='/')
    {
      dir+='/';
    }
    for(int hour=0;hour<24;hour++)
    {
      char fnbuf[64];
      sprintf(fnbuf,"%02d.arc",hour);
      if(!File::Exists((dir+fnbuf).c_str()))
      {
        continue;
      }
      fprintf(stderr,"Reading file %s\n",fnbuf);
      PersistentStorage st(dir,fnbuf);
      SMS sms;
      SMSId id;

      int ok=0;
      int pok=0;
      int pfail=0;
      int fail=0;

      int cnt=0;

      File::offset_type pieceStartOffset=0;
      uint64_t readStartMs=getmillis();

      while(st.readRecord(id,sms))
      {
        cnt++;
        Counters* rCntPtr=r.GetPtr(sms.getRouteId());
        int parts=sms.getIntProperty(Tag::SMSC_ORIGINALPARTSNUM)?sms.getIntProperty(Tag::SMSC_ORIGINALPARTSNUM):1;
        if(rCntPtr)
        {
          if(sms.getLastResult()==0)
          {
            rCntPtr->ok++;
            rCntPtr->pok+=parts;
          }else
          {
            rCntPtr->fail++;
            rCntPtr->pfail+=parts;
          }
        }
        if(sms.getLastResult()==0)
        {
          ok++;
          pok+=parts;
        }else
        {
          fail++;
          pfail+=parts;
        }
        if((cnt%10000)==0)
        {
          fprintf(stderr,"Reading record %d\r",cnt);
        }
      }
      fprintf(stderr,"\nHour %d finished\n",hour);
      printf("Hour %02d (ok, ok parts, fail, fail parts)\nTotal,%d,%d,%d,%d\n",hour,ok,pok,fail,pfail);
      for(size_t i=0;i<routes.size();i++)
      {
        Counters& rCnt=r.Get(routes[i].c_str());
        printf("%s,%d,%d,%d,%d\n",routes[i].c_str(),rCnt.ok,rCnt.pok,rCnt.fail,rCnt.pfail);
        rCnt=Counters();
      }
      fflush(stdout);
    }
  }catch(std::exception& e)
  {
    printf("Exception:'%s'\n",e.what());
  }
  fprintf(stderr,"\nDone\n");
}

