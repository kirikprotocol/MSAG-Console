#include "profiler/profiler.hpp"
#include <vector>
#include "core/buffers/Array.hpp"

namespace smsc{
namespace profiler{

using namespace smsc::core::buffers;


class ProfilesTable{
  struct PackedAddress{
    uint16_t planAndType;
    uint8_t addr[10];
    uint8_t mask[10];
    uint8_t addrLen;
    void set(const Address& address)
    {
      planAndType=address.type|(address.plan<<8);
      memset(addr,0,sizeof(addr));
      memset(mask,0,sizeof(mask));
      for(int i=0;i<address.lenght;i++)
      {
        addr[i>>1]|=(address.value[i]-'0')<<((i&1)?0:4);
        mask[i>>1]|=(address.value[i]=='?'?0x0:0xf)<<((i&1)?0:4);
      }
      addrLen=address.lenght;
    }
    void assign(const PackedAddress& src)
    {
      planAndType=src.planAndType;
      memcpy(addr,src.addr,sizeof(addr));
      memcpy(mask,src.mask,sizeof(mask));
      addrLen=src.addrLen;
    }
  };
  struct ProfileRecord{
    PackedAddress address;
    Profile profile;
  };
  typedef Array<ProfileRecord*> RecordsVector;

  RecordsVector table;

  static int Compare(const PackedAddress& patt,const PackedAddress& addr)
  {
    int res=patt.planAndType-addr.planAndType;
    if(res!=0)return res;
    for(int i=0;i<patt.addrLen && i<addr.addrLen;i++)
    {
      res=(patt.addr[i]&patt.mask[i]) - (addr.addr[i]&patt.mask[i]);
      if(res!=0)
      {
        return res;
      }
    }
    if(patt.addrLen==addr.addrLen)return 0;
    if(patt.addrLen<addr.addrLen)return -1;
    return 1;
  }

  static bool ExactCompare(const PackedAddress& addr1,const PackedAddress& addr2)
  {
    if(addr1.planAndType!=addr2.planAndType)return false;
    if(addr1.addrLen!=addr2.addrLen)return false;
    for(int i=0;i<addr1.addrLen;i++)
    {
      if(addr1.addr[i]!=addr2.addr[i])return false;
    }
    return true;
  }

  int findPos(const PackedAddress& pad)
  {
    if(table.Count()==0)return 0;
    int idx=table.Count()>>1;
    int step=idx>>1;
    int cmp=1;
    while(step)
    {
      cmp=Compare(table[idx]->address,pad);
      if(!cmp)
      {
        while(idx>0 && !Compare(table[idx-1]->address,pad))
        {
          idx--;
        }
        return idx;
      }
      idx+=cmp>0?-step:step;
      step>>=1;
    }
    return idx;
  }

public:
  Profile& find(const Address& addr,bool exact)
  {
    PackedAddress pad;
    pad.set(addr);
    int pos=findPos(pad);
    exact=ExactCompare(table[pos]->address,pad);
    return table[pos]->profile;
  }
  Profile& insert(const Address& addr,const Profile& profile)
  {
    PackedAddress pad;
    pad.set(addr);
    int pos=findPos(pad);

    table.Insert(pos,new ProfileRecord());
    table[pos]->profile.assign(profile);
    table[pos]->address.assign(pad);
    return table[pos]->profile;
  }
  bool remove(const Address& addr)
  {
    bool exact;
    PackedAddress pad;
    pad.set(addr);
    int pos=findPos(pad);
    if(!ExactCompare(table[pos]->address,pad))return false;
    delete table[pos];
    table.Delete(pos);
    return true;
  }
};

Profiler::Profiler()
{
  state=VALID;
  managerMonitor=NULL;
  profiles=new ProfilesTable;
}

Profiler::~Profiler()
{
  delete profiles;
}

Profile& Profiler::lookup(const Address& address)
{
  bool exact;
  return profiles->find(address,exact);
}

void Profiler::update(const Address& address,const Profile& profile)
{
  bool exact;
  Profile &prof=profiles->find(address,exact);
  if(prof==profile)return;
  if(exact)
  {
    prof.assign(profile);
    dbUpdate(prof);
  }else
  {
    dbInsert(profiles->insert(address,profile));
  }
}

void Profiler::add(const Address& address,const Profile& profile)
{
  profiles->insert(address,profile);
}

void Profiler::dbUpdate(const Profile& profile)
{
}

void Profiler::dbInsert(const Profile& profile)
{
}

static const int _update_report=1;
static const int _update_charset=2;

void Profiler::internal_update(int flag,const Address& addr,int value)
{
  Profile profile;
  profile.assign(lookup(addr));

  if(flag==_update_report)
  {
    profile.reportoptions=value;
  }
  if(flag==_update_charset)
  {
    profile.codepage=value;
  }
  update(addr,profile);
}


int Profiler::Execute()
{
  SmscCommand cmd;
  SMS *sms;
  int len;
  char body[MAX_SHORT_MESSAGE_LENGTH];
  while(!isStopping)
  {
    waitFor();
    if(!hasOutput())continue;
    cmd=getOutgoingCommand();
    if(cmd->cmdid!=smsc::smeman::SUBMIT)
    {
      __trace2__("Profiler: incorrect command submitted");
      continue;
    }
    sms = cmd->get_sms();
    Address& addr=sms->getOriginatingAddress();
    len = sms->getMessageBody().getData( (uint8_t*)body );
    if(!strncmp(body,"REPORT",6))
    {
      int i=7;
      while(i<len && !isalpha(body[i]))i++;
      if(i<len)
      {
        if(!strncmp(body+i,"NONE",4))
        {
          internal_update(_update_report,addr,ProfileReportOptions::ReportNone);
        }
        else
        if(!strncmp(body+i,"FULL",4))
        {
          internal_update(_update_report,addr,ProfileReportOptions::ReportFull);
        }
      }
    }else
    if(!strncmp(body,"UCS2",4))
    {
      internal_update(_update_charset,addr,ProfileCharsetOptions::Ucs2);

    }else
    if(!strncmp(body,"DEFAULT",4))
    {
      internal_update(_update_charset,addr,ProfileCharsetOptions::Default);
    }
  }
}


};//profiler
};//smsc
