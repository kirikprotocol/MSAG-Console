#include <ctype.h>
#include <locale.h>
#include <map>
#include <algorithm>

#include "DistrListManager.h"
#include "core/buffers/Hash.hpp"

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
  using namespace smsc::core::buffers;
  using namespace smsc::distrlist;

  std::string path=argv[1];
  if(path.length()>0 && path[path.length()-1]!='/')
  {
    path+='/';
  }

  FixedRecordFile<DistrListRecord> lstFile(lstFileSig,lstFileVer);
  FixedRecordFile<MemberRecord>    memFile(memFileSig,memFileVer);
  FixedRecordFile<SubmitterRecord> sbmFile(sbmFileSig,sbmFileVer);
  FixedRecordFile<PrincipalRecord> prcFile(prcFileSig,prcFileVer);

  try{
    prcFile.Open((path+"principals.bin").c_str());

    Hash<PrincipalRecord> prcs;

    PrincipalRecord prc;
    while(prcFile.Read(prc))
    {
      prcs.Insert(prc.address.toString().c_str(),prc);
      printf("Prc:'%s' maxLst=%d, maxEl=%d\n",prc.address.toString().c_str(),prc.maxLst,prc.maxEl);
    }

    lstFile.Open((path+"lists.bin").c_str());
    DistrListRecord lst;
    Hash<DistrListRecord> dls;
    typedef std::map<std::string,std::string> StringMap;
    while (lstFile.Read(lst))
    {
      printf("Lst:'%s' system=%s, maxEl=%d, owner=%s\n",lst.name,lst.system?"true":"false",lst.maxEl,lst.owner.toString().c_str());
      if(!lst.system)
      {
        PrincipalRecord* prcptr=prcs.GetPtr(lst.owner.toString().c_str());
        if(!prcptr)
        {
          printf("Principal %s for list %s not found!\n",lst.owner.toString().c_str(),lst.name);
        }else
        {
          dls.Insert(lst.name,lst);
          prcptr->lstCount++;
          if(prcptr->lstCount>prcptr->maxLst)
          {
            printf("Max lists number exceeded for principal %s\n",lst.owner.toString().c_str());
          }
        }
      }else
      {
        dls.Insert(lst.name,lst);
      }
    }

    MemberRecord mem;
    memFile.Open((path+"members.bin").c_str());
    while(memFile.Read(mem))
    {
      DistrListRecord* ptr=dls.GetPtr(mem.list);
      if(!ptr)
      {
        printf("List %s for member %s not found!\n",mem.list,mem.addr.toString().c_str());
        continue;
      }
      printf("Mem:'%s' list=%s\n",mem.addr.toString().c_str(),mem.list);
    }

    SubmitterRecord sbm;
    sbmFile.Open((path+"submitters.bin").c_str());

    while(sbmFile.Read(sbm))
    {
      DistrListRecord* ptr=dls.GetPtr(sbm.list);
      if(!ptr)
      {
        printf("List %s for submitter %s not found!\n",sbm.list,sbm.addr.toString().c_str());
        continue;
      }
      printf("Sbm:'%s' list=%s\n",sbm.addr.toString().c_str(),sbm.list);
    }

  }catch(std::exception& e)
  {
    printf("Exception:%s\n",e.what());
    return 1;
  }
  return 0;
}

