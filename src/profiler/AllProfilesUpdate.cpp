#include <stdio.h>
#include "profiler/profiler-types.hpp"
#include "sms/sms_util.h"
#include "core/buffers/File.hpp"
#include <map>
#include <vector>
#include "util/config/Manager.h"
#include "util/findConfigFile.h"

using namespace smsc::sms;
using namespace smsc::core::buffers;
using namespace smsc::profiler;
using namespace smsc::util;

const char sig[]="SMSCPROF";
#ifdef SMSEXTRA
const uint32_t ver=0x00010100;
#else
const uint32_t ver=0x00010001;
#endif
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
  {"substitute",HideOption::hoSubstitute},
};


struct FieldToValues{
  template <int N>
  FieldToValues(const char* fldName,NameToValue (&valuesArray)[N])
  {
    field=fldName;
    values=valuesArray;
    valuesCount=N;
  }
  std::string field;
  NameToValue* values;
  int valuesCount;
};

FieldToValues allFields[]=
{
  FieldToValues("datacoding",datacoding),
  FieldToValues("report",report),
  FieldToValues("hide",hide)
};

template <int N>
void DumpAllFields(FieldToValues (&fields)[N])
{
  for(int i=0;i<N;i++)
  {
    printf("%s=\n",fields[i].field.c_str());
    for(int j=0;j<fields[i].valuesCount;j++)
    {
      printf("\t%s\n",fields[i].values[j].name.c_str());
    }
  }
  printf("accesssMaskIn={hex value}\n");
  printf("accesssMaskOut={hex value}\n");
}

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
    }
    else if(it->first=="accessmaskin")
    {
      sscanf(it->second.c_str(),"%x",&p.accessMaskIn);
    }
    else if(it->first=="accessmaskout")
    {
      sscanf(it->second.c_str(),"%x",&p.accessMaskOut);
    }
    else if(it->first=="divertmodifiable")
    {
      p.divertModifiable=it->second=="true" || it->second=="yes";
    }else
    {
      throw Exception("Unknown field:%s\n",it->first.c_str());
    }
  }
}

smsc::profiler::Profile defProfile;

void InitDefProfile(smsc::util::config::Manager* cfg)
{
  char *rep=cfg->getString("profiler.defaultReport");
  char *dc=cfg->getString("profiler.defaultDataCoding");


  char *str=rep;
  while((*str=toupper(*str)))str++;
  str=dc;
  while((*str=toupper(*str)))str++;
  defProfile.locale=cfg->getString("core.default_locale");

  defProfile.hide=cfg->getBool("profiler.defaultHide");
  defProfile.hideModifiable=cfg->getBool("profiler.defaultHideModifiable");
  defProfile.divertModifiable=cfg->getBool("profiler.defaultDivertModifiable");

  defProfile.udhconcat=cfg->getBool("profiler.defaultUdhConcat");

  if(!strcmp(dc,"DEFAULT"))
    defProfile.codepage=ProfileCharsetOptions::Default;
  else if(!strcmp(dc,"UCS2"))
    defProfile.codepage=ProfileCharsetOptions::Ucs2;
  else
  {
    //smsc_log_warn(log, "Profiler:Unrecognized default data coding");
  }

  if(cfg->getBool("profiler.defaultUssdIn7Bit"))
    defProfile.codepage|=ProfileCharsetOptions::UssdIn7Bit;

  if(!strcmp(rep,"NONE"))
    defProfile.reportoptions=ProfileReportOptions::ReportNone;
  else if(!strcmp(rep,"FULL"))
    defProfile.reportoptions=ProfileReportOptions::ReportFull;
  else
  {
    //smsc_log_warn(log, "Profiler:Unrecognized default report options");
  }
}

bool isDefault(const Profile& p)
{
  bool rv=p==defProfile;
  if(!rv)
  {
    //rintf("%s  !=\n%s\n\n",p.toString().c_str(),defProfile.toString().c_str());
  }
  return rv;
}



int main(int argc,char* argv[])
{
  if(argc<3)
  {
    printf("Usage: %s infile outfile [field1=value1] [field2=value2 ...]\n",argv[0]);
    printf("Available fields:\n");
    DumpAllFields(allFields);
    printf("\nRemove abonents with specified numbers:\nremoveabonents={filename}\n");
    printf("Apply changes to abonents with prefix only:\nprefix=addrprefix\n");
    printf("Apply changes or create profile for abonents in file:\nabonents={filename}\n");
    return 0;
  }
  smsc::logger::Logger::Init();
  ArgsVector argsVector;
  std::string filterFileName;
  std::string abonentsFile;
  std::string prefix;
  std::string n,v;
  try{
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
      if(n=="removeabonents")
      {
        filterFileName=v;
        continue;
      }
      if(n=="prefix")
      {
        Address a(v.c_str());
        prefix=a.toString().c_str();
        continue;
      }
      if(n=="abonents")
      {
        abonentsFile=v;
        continue;
      }
      argsVector.push_back(NameValue(n,v));
    }
  }catch(std::exception& e)
  {
    printf("Exception:%s\n",e.what());
    return 0;
  }
  std::set<Address> filter;
  std::set<Address> abonents;

  std::string inFileName=argv[1];
  std::string outFileName=argv[2];
  try{
    if(filterFileName.length()>0)
    {
      File f;
      f.ROpen(filterFileName.c_str());
      std::string addr;
      while(f.ReadLine(addr))
      {
        if(addr.length()==0)
        {
          continue;
        }
        if(addr.length()>5 && addr[0]=='7')addr='+'+addr;
        filter.insert(addr.c_str());
      }
    }

    if(abonentsFile.length())
    {
      File f;
      f.ROpen(abonentsFile.c_str());
      std::string addr;
      while(f.ReadLine(addr))
      {
        if(addr.length()==0)
        {
          continue;
        }
        if(addr.length()>5 && addr[0]=='7')addr='+'+addr;
        abonents.insert(addr.c_str());
      }
    }

    smsc::util::config::Manager::init(findConfigFile("config.xml"));
    smsc::util::config::Manager* cfg=&smsc::util::config::Manager::getInstance();

    InitDefProfile(cfg);


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
    int skip=0;
    int filtered=0;
    int patched=0;

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

        if(filter.find(addr)!=filter.end())
        {
          filtered++;
        }
        else
        {
          if(prefix.length())
          {
            if(std::string(addr.toString().c_str()).find(prefix)==0)
            {
              if(!argsVector.empty())
              {
                PatchProfile(p,argsVector);
                patched++;
              }
            }
          }else if(!abonents.empty())
          {
            std::set<Address>::iterator it=abonents.find(addr);
            if(it!=abonents.end())
            {
              if(!argsVector.empty())
              {
                PatchProfile(p,argsVector);
                abonents.erase(it);
                patched++;
              }
            }
          }else
          {
            if(!argsVector.empty())
            {
              PatchProfile(p,argsVector);
              patched++;
            }
          }

          if(isDefault(p))
          {
            skip++;
            inFile.SeekCur(8+AddressSize()+Profile::Size());
          }else
          {

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
          }
        }
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

    int created=0;
    {
      p=defProfile;
      PatchProfile(p,argsVector);
      if(!isDefault(p))
      {
        for(std::set<Address>::iterator it=abonents.begin();it!=abonents.end();it++)
        {
          outFile.WriteByte(1);
          outFile.Write(profileMagic,8);
          WriteAddress(outFile,*it);
          p.Write(outFile);
          outFile.SeekEnd(0);
          created++;
        }
      }
    }

    outFile.Flush();
    printf("%d profiles processed\n%d updated\n%d duplicates found\n%d matched default and skipped\n"
           "%d removed by request\n%d created\n",cnt,patched,dup,skip,filtered,created);
  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
  }
  return 0;
}
