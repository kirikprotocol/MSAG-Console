#include "core/buffers/PageFile.hpp"
#include "util/BufferSerialization.hpp"
#include "AbonentProfile.hpp"
#include <vector>

using namespace smsc::util;

int main(int argc,char* argv[])
{
  using namespace smsc::core::buffers;
  using namespace smsc::emailsme;
  if(argc==1)
  {
    printf("Usage:%s [-d|-ad] {emailsmestorefile.bin}\n",argv[0]);
    printf("\t-d full dump\n");
    printf("\t-ad address dump\n");
    return 0;
  }
  bool dump=strcmp(argv[1],"-d")==0;
  bool addrDump=strcmp(argv[1],"-ad")==0;
  std::string fileName=dump||addrDump?argv[2]:argv[1];
  try{
    PageFile pf;
    pf.Open(fileName.c_str());
    File::offset_type roff=0,off;
    std::vector<unsigned char> data;
    smsc::util::SerializationBuffer buf;
    int totalCnt=0,fixedCnt=0;
    while(roff=pf.Read(roff,data,&off))
    {
      buf.setExternalBuffer(&data.front(),(unsigned)data.size());
      AbonentProfile p;
      p.Read(buf);
      totalCnt++;
      if(dump)
      {
        printf("addr=.%d.%d.%s;user=%s;limit=%d%s;nm=%s;cntEml2Gsm=%d;cntGsm2Eml=%d\n",p.addr.type,p.addr.plan,p.addr.value,p.user.c_str(),p.limitValue,
               p.ltype==ltDay?"D":p.ltype==ltWeek?"W":"M",p.numberMap?"Y":"N",
               p.limitCountEml2Gsm,p.limitCountGsm2Eml);
      }
      if(addrDump)
      {
        printf("%s\n",p.addr.toString().c_str());
      }
      if(!dump && !addrDump)
      {
        if(p.addr.value[0]=='7' && (p.addr.type!=1 || p.addr.plan!=1))
        {
          fixedCnt++;
          p.addr.type=1;
          p.addr.plan=1;
          SerializationBuffer buf2;
          p.Write(buf2);
          pf.Update(off,buf2.getBuffer(),buf.getPos());
        }
      }
    }
    if(!dump && !addrDump)
    {
      printf("Total profiles found:%d, fixed profiles:%d\n",totalCnt,fixedCnt);
    }
  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
  }
}
