#include <ctype.h>
#include <locale.h>
#include <map>
#include <algorithm>

#include "core/buffers/Hash.hpp"
#include "DistrListManager.h"

template <int N>
void strtolower(char (&str)[N])
{
  for(int i=0;i<N;i++)
  {
    str[i]=tolower((unsigned char)str[i]);
  }
}

static char lstFileSig[]="SMSCDLLST";
static char prcFileSig[]="SMSCDLPRC";
static char memFileSig[]="SMSCDLMEM";
static char sbmFileSig[]="SMSCDLSBM";

static uint32_t lstFileVer=0x010000;
static uint32_t prcFileVer=0x010000;
static uint32_t memFileVer=0x010000;
static uint32_t sbmFileVer=0x010000;


int main(int argc,char* argv[])
{
  if(argc==1)
  {
    printf("Usage: %s distr_list_store_dir\n",argv[0]);
    return 1;
  }
  if(setlocale(LC_CTYPE,"")==0)
  {
    printf("setlocale failed\n");
    return 1;
  }
  using namespace smsc::core::buffers;
  using namespace smsc::distrlist;
  FixedRecordFile<DistrListRecord> lstFileOld(lstFileSig,lstFileVer);
  FixedRecordFile<DistrListRecord> lstFileNew(lstFileSig,lstFileVer);
  std::string path=argv[1];
  if(path.length()>0 && path[path.length()-1]!='/')
  {
    path+='/';
  }
  std::string file=path+"lists.bin";
  std::string fileBak=file+".bak";

  int lstCnt=0;
  int lstMrg=0;
  try{
    File::Rename(file.c_str(),fileBak.c_str());
    lstFileOld.Open(fileBak.c_str());
    lstFileNew.Open(file.c_str());
    DistrListRecord rec;
    Hash<int> dls;
    typedef std::map<std::string,std::string> StringMap;
    StringMap lstRemap;
    while (lstFileOld.Read(rec))
    {
      std::string oldLst=rec.name;
      strtolower(rec.name);
      if(dls.Exists(rec.name))
      {
        std::string newLst=rec.name;
        int cnt=++dls[rec.name];
        char buf[32];
        sprintf(buf,"_%d",cnt);
        newLst+=buf;
        if(newLst.length()>sizeof(rec.name)-1)
        {
          printf("Warning: distrlist name '%s' too long\n",newLst.c_str());
          newLst.erase(newLst.length()-4,2);
        }
        rec.setName(newLst.c_str());
        lstRemap.insert(StringMap::value_type(oldLst,newLst));
        dls.Insert(newLst.c_str(),0);
        lstMrg++;
      }else
      {
        dls.Insert(oldLst.c_str(),0);
        lstRemap.insert(StringMap::value_type(oldLst,rec.name));
      }
      lstFileNew.Append(rec);
      lstCnt++;
    }

    MemberRecord mem;
    file=path+"members.bin";
    fileBak=file+".bak";
    FixedRecordFile<MemberRecord> memFileOld(memFileSig,memFileVer);
    FixedRecordFile<MemberRecord> memFileNew(memFileSig,memFileVer);

    File::Rename(file.c_str(),fileBak.c_str());
    memFileOld.Open(fileBak.c_str());
    memFileNew.Open(file.c_str());
    int memCnt=0;
    while(memFileOld.Read(mem))
    {
      StringMap::iterator it=lstRemap.find(mem.list);
      if(it==lstRemap.end())
      {
        printf("Error: members of unknown list:'%s'\n",mem.list);
        continue;
      }
      memcpy(mem.list,it->second.c_str(),std::min(sizeof(mem.list),it->second.length()+1));
      memFileNew.Append(mem);
      memCnt++;
    }

    SubmitterRecord sbm;
    file=path+"submitters.bin";
    fileBak=file+".bak";
    FixedRecordFile<SubmitterRecord> sbmFileOld(sbmFileSig,sbmFileVer);
    FixedRecordFile<SubmitterRecord> sbmFileNew(sbmFileSig,sbmFileVer);

    int sbmCnt=0;
    File::Rename(file.c_str(),fileBak.c_str());
    sbmFileOld.Open(fileBak.c_str());
    sbmFileNew.Open(file.c_str());
    while(sbmFileOld.Read(sbm))
    {
      StringMap::iterator it=lstRemap.find(sbm.list);
      if(it==lstRemap.end())
      {
        printf("Error: submitter of unknown list:'%s'\n",sbm.list);
        continue;
      }
      memcpy(sbm.list,it->second.c_str(),std::min(sizeof(sbm.list),it->second.length()+1));
      sbmFileNew.Append(sbm);
      sbmCnt++;
    }
    printf("Lists processed:%d\n",lstCnt);
    printf("case conflicts resolved:%d\n",lstMrg);
    printf("Members processed:%d\n",memCnt);
    printf("Submitters processed:%d\n",sbmCnt);
  }catch(std::exception& e)
  {
    printf("Exception:%s\n",e.what());
    return 1;
  }
  return 0;
}
