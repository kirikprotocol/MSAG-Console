#include "core/buffers/PageFile.hpp"
#include "util/BufferSerialization.hpp"
#include "AbonentProfile.hpp"
#include <vector>

int main(int argc,char* argv[])
{
  using namespace smsc::core::buffers;
  using namespace smsc::emailsme;
  if(argc==1)
  {
    printf("Usage:%s [-d] {emailsmestorefile.bin}\n",argv[0]);
    printf("\t-d dump\n");
    return 0;
  }
  bool dump=strcmp(argv[1],"-d")==0;
  std::string fileName=dump?argv[2]:argv[1];
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
//      printf(".%d.%d.%s\n",p.addr.type,p.addr.plan,p.addr.value);
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
    printf("Total profiles found:%d, fixed profiles:%d\n",totalCnt,fixedCnt);
  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
  }
}
