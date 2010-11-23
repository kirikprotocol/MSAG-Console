/*
 * AclManager.cpp
 *
 *  Created on: Jun 25, 2010
 *      Author: skv
 */

#include "smsc/acls/AclManager.hpp"
#include "util/Exception.hpp"

namespace smsc{
namespace acl{

AclStore* AclStore::instance=0;

AclStore::AclStore():aclStoreFile("ACLSTORE",0x01),maxIdent(1)
{
  allowDiskWrites=false;

}

AclStore::~AclStore()
{
  for(AclMap::iterator it=aclMap.begin(),end=aclMap.end();it!=end;it++)
  {
    delete it->second;
  }
  aclMap.clear();
}

void    AclStore::enumerate(std::vector<AclInfo>& result)
{
  sync::MutexGuard mg(mtx);
  for(AclMap::iterator it=aclMap.begin(),end=aclMap.end();it!=end;it++)
  {
    result.push_back(it->second->info);
  }
}

AclInfo AclStore::getInfo(AclIdent ident)
{
  sync::MutexGuard mg(mtx);
  AclMap::iterator it=aclMap.find(ident);
  if(it==aclMap.end())
  {
    throw smsc::util::Exception("AclIdent=%d not found",ident);
  }
  return it->second->info;
}

void    AclStore::remove(AclIdent ident)
{
  sync::MutexGuard mg(mtx);
  AclMap::iterator it=aclMap.find(ident);
  if(it==aclMap.end())
  {
    throw smsc::util::Exception("AclIdent=%d not found",ident);
  }
  if(allowDiskWrites)
  {
    buf::File::Unlink(mkStoreFileName(ident).c_str());
    aclStoreFile.Delete(it->second->info.offset);
  }
  delete it->second;
  aclMap.erase(it);
}

void    AclStore::create(AclIdent ident,const char* aclname,const char* descr,const std::vector<std::string>& phones)
{
  sync::MutexGuard mg(mtx);
  AclMap::iterator it=aclMap.find(ident);
  if(it!=aclMap.end())
  {
    throw smsc::util::Exception("AclIdent=%d duplicate",ident);
  }
  AclData* ad=new AclData;
  ad->info.ident=ident;
  ad->info.name=aclname;
  ad->info.description=descr;
  if(allowDiskWrites)
  {
    ad->info.offset=aclStoreFile.Append(ad->info);
    ad->store.Open(mkStoreFileName(ident).c_str());
  }
  aclMap.insert(AclMap::value_type(ident,ad));
  for(std::vector<std::string>::const_iterator pit=phones.begin(),end=phones.end();pit!=end;pit++)
  {
    AclRecord ar(pit->c_str());
    if(allowDiskWrites)
    {
      ar.offset=ad->store.Append(ar);
    }
    ad->numbers.insert(ar);
  }
}

AclIdent AclStore::create2(const char* aclname,const char* descr,const std::vector<std::string>& phones)
{
  AclIdent ident;
  {
    sync::MutexGuard mg(mtx);
    ident=++maxIdent;
  }
  create(ident,aclname,descr,phones);
  return ident;
}

void    AclStore::lookupByPrefix(AclIdent ident,const char* prefix,std::vector<AclPhoneNumber>& result)
{
  sync::MutexGuard mg(mtx);
  AclMap::iterator it=aclMap.find(ident);
  if(it==aclMap.end())
  {
    throw smsc::util::Exception("AclIdent=%d not found",ident);
  }
  for(AclData::AclSet::iterator sit=it->second->numbers.begin(),end=it->second->numbers.end();sit!=end;sit++)
  {
    size_t i=0;
    while(prefix[i] && prefix[i]==sit->number.str[i])i++;
    if(!prefix[i])
    {
      result.push_back(sit->number);
    }
  }
}
void    AclStore::removePhone(AclIdent ident,const AclPhoneNumber& phone)
{
  sync::MutexGuard mg(mtx);
  AclMap::iterator it=aclMap.find(ident);
  if(it==aclMap.end())
  {
    throw smsc::util::Exception("AclIdent=%d not found",ident);
  }
  AclData::AclSet::iterator sit=it->second->numbers.find(phone);
  if(sit==it->second->numbers.end())
  {
    throw smsc::util::Exception("Number %s not found in AclIdent=%d",phone.c_str(),ident);
  }
  if(allowDiskWrites)
  {
    it->second->store.Delete(sit->offset);
  }
  it->second->numbers.erase(sit);
}

void    AclStore::addPhone(AclIdent ident,const AclPhoneNumber& phone)
{
  sync::MutexGuard mg(mtx);
  AclMap::iterator it=aclMap.find(ident);
  if(it==aclMap.end())
  {
    throw smsc::util::Exception("AclIdent=%d not found",ident);
  }
  AclData::AclSet::iterator sit=it->second->numbers.find(phone);
  if(sit!=it->second->numbers.end())
  {
    throw smsc::util::Exception("Duplicate Number %s in AclIdent=%d",phone.c_str(),ident);
  }
  AclRecord rec(phone);
  if(allowDiskWrites)
  {
    rec.offset=it->second->store.Append(rec);
  }
  it->second->numbers.insert(rec);
}

void    AclStore::updateAclInfo(AclIdent ident,const char* aclname,const char* acldesc)
{
  sync::MutexGuard mg(mtx);
  AclMap::iterator it=aclMap.find(ident);
  if(it==aclMap.end())
  {
    throw smsc::util::Exception("AclIdent=%d not found",ident);
  }
  it->second->info.name=aclname;
  it->second->info.description=acldesc;
  if(allowDiskWrites)
  {
    aclStoreFile.Write(it->second->info.offset,it->second->info);
  }
}

bool AclStore::isGranted(AclIdent ident,const AclPhoneNumber& phone)
{
  sync::MutexGuard mg(mtx);
  AclMap::iterator it=aclMap.find(ident);
  if(it==aclMap.end())
  {
    throw smsc::util::Exception("AclIdent=%d not found",ident);
  }
  return it->second->numbers.find(phone)!=it->second->numbers.end();
}

void AclStore::Load(const char* argAclStoreDir)
{
  path=argAclStoreDir;
  if(!path.empty() && *path.rbegin()!='/')
  {
    path+='/';
  }
  std::string storeFileName=path+"acl.bin";
  aclStoreFile.Open(storeFileName.c_str());
  AclInfo info;
  while((info.offset=aclStoreFile.Read(info)))
  {
    if(info.ident>maxIdent)
    {
      maxIdent=info.ident;
    }
    AclData* ad=new AclData;
    ad->info=info;
    storeFileName=mkStoreFileName(info.ident);
    ad->store.Open(storeFileName.c_str());
    AclRecord ar;
    while((ar.offset=ad->store.Read(ar)))
    {
      ad->numbers.insert(ar);
    }
    if(!allowDiskWrites)
    {
      ad->store.Close();
    }
  }
  if(!allowDiskWrites)
  {
    aclStoreFile.Close();
  }
}

void AclStore::enableControllerMode()
{
  allowDiskWrites=true;
}

void AclStore::Init()
{
  instance=new AclStore;
}

void AclStore::Shutdown()
{
  delete instance;
  instance=0;
}


}
}
