#define NOLOGGERPLEASE 1
#include <stdio.h>
#include "profiler/profiler-types.hpp"
#include "sms/sms_util.h"
#include "core/buffers/File.hpp"
#include <map>
#include <vector>

using namespace smsc::sms;
using namespace smsc::core::buffers;
using namespace smsc::profiler;

const char sig[]="SMSCPROF";
const uint32_t ver=0x00010001;
static char profileMagic[]="SmScPrOf";

typedef std::pair<std::string,std::string> NameValue;
typedef std::vector<NameValue> ArgsVector;

struct NameToValue{
  std::string name;
  int value;
};

NameToValue datacoding[]=
{
  {"default",ProfileCharsetOptions::Default},
  {"latin1",ProfileCharsetOptions::Latin1},
  {"ucs2",ProfileCharsetOptions::Ucs2},
  {"ussdin7bit",ProfileCharsetOptions::UssdIn7Bit},
};

NameToValue report[]=
{
  {"none",ProfileReportOptions::ReportNone},
  {"final",ProfileReportOptions::ReportFinal},
  {"full",ProfileReportOptions::ReportFull},
};

NameToValue hide[]=
{
  {"enable",HideOption::hoEnabled},
  {"disable",HideOption::hoDisabled},
  {"Substitute",HideOption::hoSubstitute},
};


template <int N>
int FindValue(const std::string& name,NameToValue (&values)[N])
{
  for(int i=0;i<N;i++)
  {
    if(name==values[i].name)return values[i].value;
  }
  throw Exception("Unknown value:%s\n",name.c_str());
}

void PatchProfile(Profile& p,const ArgsVector& args)
{
  for(ArgsVector::const_iterator it=args.begin();it!=args.end();it++)
  {
    //printf("%s = %s\n",it->first.c_str(),it->second.c_str());
    if(it->first=="datacoding")
    {
      int val=FindValue(it->second,datacoding);
      if(it->second!="ussdin7bit")
      {
        p.codepage=val;
      }else
      {
        p.codepage|=val;
      }
    }else if(it->first=="report")
    {
      p.reportoptions=FindValue(it->second,report);
    }else if(it->first=="hide")
    {
      p.hide=FindValue(it->second,hide);
    }else
    {
      throw Exception("Unknown field:%s\n",it->first.c_str());
    }
  }
}

int main(int argc,char* argv[])
{
  if(argc<4)
  {
    printf("Usage: %s infile outfile field1=value1 [field2=value2 ...]\n",argv[0]);
    return 0;
  }
  ArgsVector argsVector;
  std::string n,v;
  for(int i=3;i<argc;i++)
  {
    char* p=strchr(argv[i],'=');
    if(!p)
    {
      printf("Invalid argument:%s\n",argv[i]);
    }
    n.assign(argv[i],p-argv[i]);
    v=p+1;
    for(int j=0;j<n.length();j++)n[j]=tolower(n[j]);
    for(int j=0;j<v.length();j++)v[j]=tolower(v[j]);
    argsVector.push_back(NameValue(n,v));
  }


  std::string inFileName=argv[1];
  std::string outFileName=argv[2];
  try{
    File inFile,outFile;
    inFile.ROpen(inFileName.c_str());
    outFile.RWCreate(outFileName.c_str());
    char fileSig[9]={0,};
    uint32_t fileVer;
    inFile.Read(fileSig,8);
    if(strcmp(fileSig,sig)!=0)
    {
      throw Exception("Invalid file signature:%s, expected:%s\n",fileSig,sig);
    }
    fileVer=inFile.ReadNetInt32();
    if(fileVer!=ver)
    {
      throw Exception("Invalid file version:%08x, expected:%08x\n",fileVer,ver);
    }
    outFile.Write(sig,8);
    outFile.WriteNetInt32(ver);
    File::offset_type sz=inFile.Size();
    File::offset_type pos=inFile.Pos();
    char magic[9]={0,};
    Profile p;
    Address addr;

    typedef std::map<Address,File::offset_type> ProfilesMap;
    ProfilesMap pmap;

    int cnt=0;
    int dup=0;

    while(pos<sz)
    {
      bool used=inFile.ReadByte();
      if(used)
      {
        inFile.Read(magic,8);
        if(memcmp(profileMagic,magic,8))
        {
          throw Exception("Broken profiles storage file at offset %lld",pos);
        }
        ReadAddress(inFile,addr);
        p.Read(inFile);

        PatchProfile(p,argsVector);


        ProfilesMap::iterator it=pmap.find(addr);
        if(it!=pmap.end())
        {
          outFile.Seek(it->second);
          dup++;
        }else
        {
          pmap.insert(ProfilesMap::value_type(addr,outFile.Pos()));
        }
        cnt++;
        outFile.WriteByte(1);
        outFile.Write(profileMagic,8);
        WriteAddress(outFile,addr);
        p.Write(outFile);
        outFile.SeekEnd(0);
      }else
      {
        inFile.SeekCur(8+AddressSize()+Profile::Size());
      }
      pos+=1+8+AddressSize()+Profile::Size();
      if(sz-pos<1+8+AddressSize()+Profile::Size())
      {
        break;
      }
    }
    outFile.Flush();
    printf("%d profiles processed, %d duplicates found\n",cnt,dup);
  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
  }
  return 0;
}
