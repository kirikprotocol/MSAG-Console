
#include "DistrListManager.h"

namespace smsc { namespace distrlist
{
    using std::string;

    using namespace smsc::sms;
    using namespace core::buffers;

static char lstFileSig[]="SMSCDLLST";
static char prcFileSig[]="SMSCDLPRC";
static char memFileSig[]="SMSCDLMEM";
static char sbmFileSig[]="SMSCDLSBM";

static uint32_t lstFileVer=0x010000;
static uint32_t prcFileVer=0x010000;
static uint32_t memFileVer=0x010000;
static uint32_t sbmFileVer=0x010000;

DistrListManager::DistrListManager(Manager& config)
    throw(ConfigException,smsc::core::buffers::FileException)
        : DistrListAdmin(), logger(Logger::getInstance("smsc.dl")),
          lstFile(lstFileSig,lstFileVer),
          prcFile(prcFileSig,prcFileVer),
          memFile(memFileSig,memFileVer),
          sbmFile(sbmFileSig,sbmFileVer)
{
  dir=config.getString("distrList.storeDir");
  if(dir.length()>0 && dir[dir.length()-1]!='/')dir+='/';
}

void DistrListManager::init()
{
  string lstFileName=dir+"lists.bin";
  string prcFileName=dir+"principals.bin";
  string memFileName=dir+"members.bin";
  string sbmFileName=dir+"submitters.bin";

  lstFile.Open(lstFileName.c_str());
  prcFile.Open(prcFileName.c_str());
  memFile.Open(memFileName.c_str());
  sbmFile.Open(sbmFileName.c_str());

  PrincipalRecord prcRec;
  char buf[32];
  while((prcRec.offset=prcFile.Read(prcRec))!=0)
  {
    prcRec.address.toString(buf,sizeof(buf));
    principals.Insert(buf,prcRec);
  }

  DistrListRecord lstRec;
  bool createList=true;
  while((lstRec.offset=lstFile.Read(lstRec))!=0)
  {
    createList=true;
    if(!lstRec.system)
    {
      lstRec.owner.toString(buf,sizeof(buf));
      PrincipalRecord* prcRecPtr=principals.GetPtr(buf);
      if(prcRecPtr)
      {
        prcRecPtr->lstCount++;
        if(prcRecPtr->maxLst!=0 && prcRecPtr->lstCount>prcRecPtr->maxLst)
        {
          warn2(logger,"Principal %s exceeded number of lists:%d/%d",buf,
            prcRecPtr->lstCount,prcRecPtr->maxLst);
          createList=false;
        }
      }else
      {
        warn2(logger,"OWNER OF LIST %s NOT FOUND(%s)!!!",lstRec.name,buf);
        createList=false;
      }
    }
    if(createList)
    {
      lists.Insert(lstRec.name,new DistrListRecord(lstRec));
    }
  }
  MemberRecord memRec;
  while((memRec.offset=memFile.Read(memRec))!=0)
  {
    DistrListRecord** lstRecPtr=lists.GetPtr(memRec.list);
    if(lstRecPtr)
    {
      DistrListRecord& lst=**lstRecPtr;
      lst.members.insert(memRec);
      if(!lst.system)
      {
        lst.owner.toString(buf,sizeof(buf));
        PrincipalRecord* prcPtr=principals.GetPtr(buf);
        if(!prcPtr)
        {
          warn2(logger,"PRINCIPAL NOT FOUND: %s",buf);
        }else
        {
          if(prcPtr->maxEl!=0 && (int)lst.members.size()>prcPtr->maxEl)
          {
            warn2(logger,"LIST %s EXCEEDED MAXIMUM NUMBER OF ELEMETS OF PRINCIPAL %s - %d/%d",
              lst.name,buf,lst.members.size(),prcPtr->maxEl);
          }
        }
      }
    }else
    {
      memRec.addr.toString(buf,sizeof(buf));
      warn2(logger,"MEMBER %s OF UNKNOWN LIST %s FOUND!",buf,memRec.list);
    }
  }
  SubmitterRecord sbmRec;
  while((sbmRec.offset=sbmFile.Read(sbmRec))!=0)
  {
    DistrListRecord** lstRecPtr=lists.GetPtr(sbmRec.list);
    if(lstRecPtr)
    {
      DistrListRecord& lst=**lstRecPtr;
      lst.submitters.insert(sbmRec);
    }else
    {
      sbmRec.addr.toString(buf,sizeof(buf));
      warn2(logger,"SUBMITTER %s OF UNKNOWN LIST %s FOUND!",buf,sbmRec.list);
    }
  }
}

DistrListManager::~DistrListManager()
{
    // Do nothing ?
}

void DistrListManager::addDistrList(string dlName, bool system,const Address& dlOwner,int maxel,smsc::core::buffers::File::offset_type offset1,smsc::core::buffers::File::offset_type offset2)
    throw(smsc::core::buffers::FileException, ListAlreadyExistsException,
          PrincipalNotExistsException, ListCountExceededException)
{
    MutexGuard mg(mtx);
    for(size_t i=0;i<dlName.length();i++)dlName[i]=tolower(dlName[i]);
    if(lists.Exists(dlName.c_str()))throw ListAlreadyExistsException();
    PrincipalRecord* prcPtr;
    if(!system)
    {
      prcPtr=principals.GetPtr(dlOwner.toString().c_str());
      if(!prcPtr)throw PrincipalNotExistsException();
      if(prcPtr->maxLst!=0 && prcPtr->lstCount>=prcPtr->maxLst)throw ListCountExceededException();
      prcPtr->lstCount++;
    }

    auto_ptr<DistrListRecord> lstPtr(system?new DistrListRecord(dlName.c_str(),maxel):
                                            new DistrListRecord(dlOwner,dlName.c_str(),prcPtr->maxEl));

    DistrListRecord& lst=*lstPtr.get();
    lst.offset=lstFile.Append(lst);
    lists.Insert(lst.name,lstPtr.release());

    if(!system)
    {
      SubmitterRecord sbm(lst,dlOwner);
      sbm.offset=sbmFile.Append(sbm);
      lst.submitters.insert(sbm);
    }
}

void DistrListManager::deleteDistrList(string dlName)
    throw(smsc::core::buffers::FileException, ListNotExistsException)
{
    MutexGuard mg(mtx);

    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException();
    DistrListRecord& lst=**lstPtr;
    for(DistrListRecord::SubmittersContainer::iterator it=lst.submitters.begin();it!=lst.submitters.end();it++)
    {
      sbmFile.Delete(it->offset);
    }
    lst.submitters.clear();
    for(DistrListRecord::MembersContainer::iterator it=lst.members.begin();it!=lst.members.end();it++)
    {
      memFile.Delete(it->offset);
    }
    lst.members.clear();

    if(!lst.system)
    {
      PrincipalRecord* prcPtr=principals.GetPtr(lst.owner.toString().c_str());
      if(prcPtr)
      {
        prcPtr->lstCount--;
      }else
      {
        warn2(logger,"Owner of list %s not found:%s",lst.name,lst.owner.toString().c_str());
      }
    }
    lstFile.Delete(lst.offset);
    DistrListRecord* ptr=*lstPtr;
    lists.Delete(lst.name);
    delete ptr;
}


void DistrListManager::changeDistrList(const string& dlName,int maxElements)
          throw(smsc::core::buffers::FileException,ListNotExistsException)
{
  MutexGuard mg(mtx);
  DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
  if(!lstPtr)throw ListNotExistsException();
  DistrListRecord& lst=**lstPtr;
  lst.maxEl=maxElements;
  lstFile.Write(lst.offset,lst);
}


DistrList DistrListManager::getDistrList(string dlName)
    throw(smsc::core::buffers::FileException, ListNotExistsException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException();
    return **lstPtr;
}

Array<DistrList> DistrListManager::list(const Address& dlOwner)
    throw(smsc::core::buffers::FileException, PrincipalNotExistsException)
{
    MutexGuard mg(mtx);
    Array<DistrList> rv(lists.GetCount());

    char* key;
    DistrListRecord* val;
    lists.First();
    while(lists.Next(key,val))
    {
      if(dlOwner==val->owner)rv.Push(*val);
    }
    return rv;

}

Array<DistrList> DistrListManager::list()
    throw(smsc::core::buffers::FileException)
{
  MutexGuard mg(mtx);
  Array<DistrList> rv(lists.GetCount());

  char* key;DistrListRecord* val;
  lists.First();
  while(lists.Next(key,val))
  {
    rv.Push(*val);
  }
  return rv;
}

void DistrListManager::copyDistrList(const std::string& dlName,const std::string& newDlName)
{
  MutexGuard mg(mtx);
  DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
  if(!lstPtr)
  {
    throw ListNotExistsException("ListNotExistsException:%s",dlName.c_str());
  }
  if(lists.Exists(newDlName.c_str()))
  {
    throw ListAlreadyExistsException("ListAlreadyExistsException:%s",newDlName.c_str());
  }
  DistrListRecord& lst=**lstPtr;
  PrincipalRecord* prcPtr=principals.GetPtr(lst.owner.toString().c_str());
  if(prcPtr)
  {
    if(prcPtr->lstCount>=prcPtr->maxLst)
    {
      throw ListCountExceededException();
    }
    prcPtr->lstCount++;
  }else
  {
    smsc_log_warn(logger,"Principal not found for list '%s'",lst.name);
  }

  std::auto_ptr<DistrListRecord> newList(new DistrListRecord(lst.owner,newDlName.c_str(),lst.maxEl));
  DistrListRecord& newLst=*newList.get();
  newLst.offset=lstFile.Append(newLst);
  lists.Insert(newLst.name,newList.release());
  for(DistrListRecord::SubmittersContainer::iterator it=lst.submitters.begin();it!=lst.submitters.end();it++)
  {
    SubmitterRecord sbm(newLst,it->addr);
    sbm.offset=sbmFile.Append(sbm);
    newLst.submitters.insert(sbm);
  }
  for(DistrListRecord::MembersContainer::iterator it=lst.members.begin();it!=lst.members.end();it++)
  {
    MemberRecord mem(newLst,it->addr);
    mem.offset=memFile.Append(mem);
    newLst.members.insert(mem);
  }
}

void DistrListManager::renameDistrList(const std::string& dlName,const std::string& newDlName)
{
  MutexGuard mg(mtx);
  DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
  if(!lstPtr)
  {
    throw ListNotExistsException("ListNotExistsException:%s",dlName.c_str());
  }
  if(lists.Exists(newDlName.c_str()))
  {
    throw ListAlreadyExistsException("ListAlreadyExistsException:%s",newDlName.c_str());
  }
  DistrListRecord& lst=**lstPtr;
  lst.setName(newDlName.c_str());
  for(DistrListRecord::SubmittersContainer::iterator it=lst.submitters.begin();it!=lst.submitters.end();it++)
  {
    it->setList(lst.name);
    sbmFile.Write(it->offset,*it);
  }
  for(DistrListRecord::MembersContainer::iterator it=lst.members.begin();it!=lst.members.end();it++)
  {
    it->setList(lst.name);
    memFile.Write(it->offset,*it);
  }
  lstFile.Write(lst.offset,lst);
  lists.Delete(dlName.c_str());
  lists.Insert(newDlName.c_str(),&lst);
}


Array<Address> DistrListManager::members(string dlName)
            throw(smsc::core::buffers::FileException, ListNotExistsException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException();
    DistrListRecord& lst=**lstPtr;
    Array<Address> members((int)lst.members.size());
    for(DistrListRecord::MembersContainer::iterator it=lst.members.begin();it!=lst.members.end();it++)
    {
      members.Push(it->addr);
    }
    return members;
}


Array<Address> DistrListManager::members(string dlName, const Address& submitter)
    throw(smsc::core::buffers::FileException, ListNotExistsException, IllegalSubmitterException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException();
    DistrListRecord& lst=**lstPtr;

    if(submitter.length && lst.submitters.find(SubmitterRecord(lst,submitter))==lst.submitters.end())
      throw IllegalSubmitterException();

    Array<Address> members((int)lst.members.size());
    for(DistrListRecord::MembersContainer::iterator it=lst.members.begin();it!=lst.members.end();it++)
    {
      members.Push(it->addr);
    }
    return members;
}

bool DistrListManager::checkPermission(string dlName, const Address& submitter)
    throw(smsc::core::buffers::FileException, ListNotExistsException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException();
    DistrListRecord& lst=**lstPtr;
    return lst.submitters.find(SubmitterRecord(lst,submitter))!=lst.submitters.end();
}

void DistrListManager::addPrincipal(const Principal& prc,smsc::core::buffers::File::offset_type offset)
    throw(smsc::core::buffers::FileException, PrincipalAlreadyExistsException)
{
    MutexGuard mg(mtx);
    if(principals.Exists(prc.address.toString().c_str()))throw PrincipalAlreadyExistsException();
    PrincipalRecord prcRec(prc.address,prc.maxLst,prc.maxEl);
    prcRec.offset=prcFile.Append(prcRec);
    principals.Insert(prc.address.toString().c_str(),prcRec);
}

void DistrListManager::deletePrincipal(const Address& address)
    throw(smsc::core::buffers::FileException, PrincipalNotExistsException, PrincipalInUseException)
{
    std::vector<std::string> listsToKill;
    PrincipalRecord* prcPtr;
    {
      MutexGuard mg(mtx);
      prcPtr=principals.GetPtr(address.toString().c_str());
      if(!prcPtr)throw PrincipalNotExistsException();
      char* key;
      DistrListRecord* val;
      lists.First();
      while(lists.Next(key,val))
      {
        if(address==val->owner)listsToKill.push_back(val->name);
      }
    }
    for(std::vector<std::string>::iterator it=listsToKill.begin();it!=listsToKill.end();it++)
    {
      deleteDistrList(*it);
    }
    MutexGuard mg(mtx);
    prcFile.Delete(prcPtr->offset);
    principals.Delete(prcPtr->address.toString().c_str());
}

void DistrListManager::changePrincipal(const Principal& prc)
    throw(smsc::core::buffers::FileException, PrincipalNotExistsException, IllegalPrincipalException)
{
    MutexGuard mg(mtx);
    PrincipalRecord* prcPtr=principals.GetPtr(prc.address.toString().c_str());
    if(!prcPtr)throw PrincipalNotExistsException();
    if(prcPtr->lstCount>0)throw IllegalPrincipalException("Principal for address '%s' is in use %ld times as DL owner. "
                                                "Can't change maximum lists count to %ld or maximum elements to %ld",
                                                 prc.address.toString().c_str(), prcPtr->lstCount, prc.maxLst,prc.maxEl);
    prcPtr->maxLst=prc.maxLst;
    prcPtr->maxEl=prc.maxEl;
    prcFile.Write(prcPtr->offset,*prcPtr);
}

Principal DistrListManager::getPrincipal(const Address& address)
    throw(smsc::core::buffers::FileException, PrincipalNotExistsException)
{
    MutexGuard mg(mtx);
    PrincipalRecord* prcPtr=principals.GetPtr(address.toString().c_str());
    if(!prcPtr)throw PrincipalNotExistsException();
    return *prcPtr;
}

Array<Principal> DistrListManager::getPrincipals()
            throw(smsc::core::buffers::FileException)
{
  MutexGuard mg(mtx);
  char* key;
  PrincipalRecord rec;
  principals.First();
  Array<Principal> rv;
  while(principals.Next(key,rec))
  {
    rv.Push(rec);
  }
  return rv;
}


void DistrListManager::addMember(string dlName, const Address& member,smsc::core::buffers::File::offset_type offset)
    throw(smsc::core::buffers::FileException, ListNotExistsException,
          MemberAlreadyExistsException, MemberCountExceededException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException("%s",dlName.c_str());
    DistrListRecord& lst=**lstPtr;
    if(!lst.system)
    {
      PrincipalRecord* prcPtr=principals.GetPtr(lst.owner.toString().c_str());
      if(!prcPtr)throw PrincipalNotExistsException("%s",lst.owner.toString().c_str());
      if(prcPtr->maxEl!=0 && (int)lst.members.size()>=prcPtr->maxEl)throw MemberCountExceededException();
    }

    MemberRecord memRec(lst,member);
    if(lst.members.find(memRec)!=lst.members.end())throw MemberAlreadyExistsException();

    memRec.offset=memFile.Append(memRec);
    lst.members.insert(memRec);
}

void DistrListManager::deleteMember(string dlName, const Address& member)
    throw(smsc::core::buffers::FileException, ListNotExistsException, MemberNotExistsException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException("%s",dlName.c_str());
    DistrListRecord& lst=**lstPtr;
    MemberRecord memRec(lst,member);
    DistrListRecord::MembersContainer::iterator it=lst.members.find(memRec);
    if(it==lst.members.end())throw MemberNotExistsException();
    memFile.Delete(it->offset);
    lst.members.erase(it);
}

void DistrListManager::deleteMembers(string dlName)
    throw(smsc::core::buffers::FileException, ListNotExistsException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException("%s",dlName.c_str());
    DistrListRecord& lst=**lstPtr;
    for(DistrListRecord::MembersContainer::iterator it=lst.members.begin();it!=lst.members.end();it++)
    {
      memFile.Delete(it->offset);
    }
    lst.members.clear();
}

void DistrListManager::grantPosting(string dlName, const Address& submitter,smsc::core::buffers::File::offset_type offset)
    throw(smsc::core::buffers::FileException, ListNotExistsException,
          PrincipalNotExistsException, SubmitterAlreadyExistsException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException("%s",dlName.c_str());
    DistrListRecord& lst=**lstPtr;
    SubmitterRecord sbmRec(lst,submitter);
    if(lst.submitters.find(sbmRec)!=lst.submitters.end())throw SubmitterAlreadyExistsException();
    sbmRec.offset=sbmFile.Append(sbmRec);
    lst.submitters.insert(sbmRec);
}

void DistrListManager::grantPosting(const string& dlName, const Address& owner,const Address& submitter,smsc::core::buffers::File::offset_type offset)
    throw(smsc::core::buffers::FileException, ListNotExistsException,
          PrincipalNotExistsException, SubmitterAlreadyExistsException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException("%s",dlName.c_str());
    DistrListRecord& lst=**lstPtr;
    if(strcmp(lst.owner.value,owner.value))throw PrincipalNotExistsException();

    SubmitterRecord sbmRec(lst,submitter);
    if(lst.submitters.find(sbmRec)!=lst.submitters.end())throw SubmitterAlreadyExistsException();
    sbmRec.offset=sbmFile.Append(sbmRec);
    lst.submitters.insert(sbmRec);
}

void DistrListManager::revokePosting(string dlName, const Address& owner,const Address& submitter)
    throw(smsc::core::buffers::FileException, ListNotExistsException,
          SubmitterNotExistsException, IllegalSubmitterException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException("%s",dlName.c_str());
    DistrListRecord& lst=**lstPtr;
    if(strcmp(lst.owner.value,owner.value))throw PrincipalNotExistsException();

    SubmitterRecord sbmRec(lst,submitter);
    DistrListRecord::SubmittersContainer::iterator it=lst.submitters.find(sbmRec);
    if(it==lst.submitters.end())throw SubmitterNotExistsException();
    sbmFile.Delete(it->offset);
    lst.submitters.erase(it);
}


void DistrListManager::revokePosting(string dlName, const Address& submitter)
    throw(smsc::core::buffers::FileException, ListNotExistsException,
          SubmitterNotExistsException, IllegalSubmitterException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException("%s",dlName.c_str());
    DistrListRecord& lst=**lstPtr;

    SubmitterRecord sbmRec(lst,submitter);
    DistrListRecord::SubmittersContainer::iterator it=lst.submitters.find(sbmRec);
    if(it==lst.submitters.end())throw SubmitterNotExistsException();
    sbmFile.Delete(it->offset);
    lst.submitters.erase(it);
}


void DistrListManager::getSubmitters(const string& dlName,Array<Address>& sbm)
          throw(smsc::core::buffers::FileException,ListNotExistsException)
{
    MutexGuard mg(mtx);
    DistrListRecord** lstPtr=lists.GetPtr(dlName.c_str());
    if(!lstPtr)throw ListNotExistsException("%s",dlName.c_str());
    DistrListRecord& lst=**lstPtr;
    for(DistrListRecord::SubmittersContainer::iterator it=lst.submitters.begin();it!=lst.submitters.end();it++)
    {
      sbm.Push(it->addr);
    }
}

}}
