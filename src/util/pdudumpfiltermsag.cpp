#include <stdio.h>
#include "logger/Logger.h"
#include "smpp/smpp.h"
#include "smeman/smsccmd.h"
#include "util/regexp/RegExp.hpp"
#include <vector>

using namespace smsc::smpp;
using namespace smsc::smeman;
using namespace smsc::logger;
using namespace smsc::util::regexp;
using namespace std;

void putnet(vector<char>& pdu,int val)
{
  val=htonl(val);
  pdu.insert(pdu.end(),(char*)&val,((char*)&val)+4);
}

int main(int argc,char* argv[])
{
  Logger::initForTest( Logger::LEVEL_INFO );
  FILE* in;
  FILE* out;
  if(argc==1)
  {
      in = stdin;
      out = stdout;
  } else if (argc==2)
  {
      in = fopen(argv[1],"rt");
      out = stdout;
  } else if (argc==3)
  {
      in = fopen(argv[1],"rt");
      out = fopen(argv[2],"rt");
  } else
  {
    printf("usage: pdudumpfiltermsag [infile [outfile]]\n");
    return -1;
  }
  // FILE *in=fopen(argv[1],"rt");
  // FILE *out=fopen(argv[2],"wt");
  RegExp::InitLocale();
  RegExp re;
  re.Compile("/smpp\\.dmp[^:]*:[^:]+[\\d\\.]+:[^:]+: ([\\dabcdef])/",OP_PERLSTYLE|OP_OPTIMIZE);
  vector<char> buf(65535);

  for(;;)
  {
    fpos_t pos;
    fgetpos(in,&pos);
    if(fgets(&buf[0],buf.size(),in)==0)break;
    int len=strlen(&buf[0]);
    if(buf[len-1]!=0x0a && !feof(in))
    {
      buf.resize(buf.size()*2);
      fsetpos(in,&pos);
      continue;
    }
    fwrite(&buf[0],1,len,out);

    SMatch m[10];
    int n=10;
    if(re.Search(&buf[0],&buf[len],m,n))
    {
      vector<char> pdu;
      // int sz,id,st,sq;
      int start = m[1].start;
      // sscanf(&buf[m[1].start],"%d,id=%x,st=%d,sq=%d:%n",&sz,&id,&st,&sq,&start);
      // start+=m[1].start;

      // putnet(pdu,sz);
      // putnet(pdu,id);
      // putnet(pdu,st);
      // putnet(pdu,sq);

      while(start<len)
      {
        int c;
        int next;
        if(sscanf(&buf[start],"%x%n",&c,&next)==0)break;
        start+=next;
        pdu.push_back(c);
      }

      SmppStream s;
      assignStreamWith(&s,&pdu[0],pdu.size(),true);
      try{
        SmppHeader* smpppdu=fetchSmppPdu(&s);
        dump_pdu(smpppdu,out);
        disposePdu(smpppdu);
      }catch(...)
      {
        printf("ex\n");
      }
    }
  }
  if (in != stdin) {fclose(in);}
  if (out != stdout) {fclose(out);}
  return 0;
}
