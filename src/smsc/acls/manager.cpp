#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <functional>
#include <memory>

#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/FixedRecordFile.hpp"
#include "core/buffers/DiskHash.hpp"
#include "sms/sms.h"
#include "sms/sms_util.h"
#include "util/crc32.h"

#include "interfaces.h"


#define ACLMGRPREFIX "Acl:"

namespace smsc {
namespace acls {

using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::util;
using namespace smsc::core::buffers;
using smsc::core::buffers::File;
using namespace smsc::sms;

using namespace std;

typedef smsc::util::config::Manager ConfigManager;

struct AclInfoRecord:AclInfo{
  File::offset_type offset;
  AclInfoRecord():offset(0){}

  AclInfoRecord(const AclInfo& acl):AclInfo(acl),offset(0)
  {

  }

  AclInfoRecord(const AclInfoRecord& rec):AclInfo(rec),offset(rec.offset)
  {

  }

  void Read(File& f)
  {
    ident=f.ReadNetInt32();
    f.ReadFixedString<32>(name);
    f.ReadFixedString<128>(description);
    cache=MakeAclCacheType(f.ReadByte());
  }
  void Write(File& f)const
  {
    f.WriteNetInt32(ident);
    f.WriteFixedString<32>(name);
    f.WriteFixedString<128>(description);
    f.WriteByte(AclCacheTypeAsChar(cache));
  }
  static uint32_t Size()
  {
    return 4+32+128+1;
  }
};


struct AclDiskRecord{
  Address addr;
  File::offset_type offset;

  AclDiskRecord():offset(0){}
  AclDiskRecord(const Address& argAddr):addr(argAddr),offset(0){}

  void Read(File& f)
  {
    ReadAddress(f,addr);
  }
  void Write(File& f)const
  {
    WriteAddress(f,addr);
  }
  static uint32_t Size()
  {
    return AddressSize();
  }

  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t res=0;
    do{
      res=crc32(res,&addr.type,sizeof(addr.type));
      res=crc32(res,&addr.plan,sizeof(addr.plan));
      res=crc32(res,addr.value,addr.length);
    }while(attempt-->0);
    return res;
  }

  bool operator==(const AclDiskRecord& aclr)
  {
    return addr==aclr.addr;
  }
};


struct Int64Record{
  uint64_t key;

  Int64Record():key(0){}
  Int64Record(uint64_t key):key(key){}
  Int64Record(const Int64Record& src)
  {
    key=src.key;
  }
  Int64Record& operator=(const Int64Record& src)
  {
    key=src.key;
    return *this;
  }
  uint64_t Get()const{return key;}

  static uint32_t Size(){return 8;}
  void Read(File& f)
  {
    key=f.ReadNetInt64();
  }
  void Write(File& f)const
  {
    f.WriteNetInt64(key);
  }
  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t res=0;
    res=crc32(res,&key,sizeof(key));
    for(;attempt>0;attempt--)res=crc32(res,&key,sizeof(key));
    return res;
  }
  bool operator==(const Int64Record& cmp)
  {
    return key==cmp.key;
  }
};



static const char* aclsFileSig="SMSCACLS";
static const uint32_t aclsFileVer=0x00010000;

static const char* aclMembersSig="SMSCACLMEM";
static const uint32_t aclMembersVer=0x00010000;

class AclManager : public AclAbstractMgr
{
  IntHash<AclInfoRecord> acls;
  string storeDir;
  FixedRecordFile<AclInfoRecord> aclsFile;

  File idFile;
  int idSeq;
  int idLastFlush;
  Mutex idMtx;
  enum{IdSequenceExtent=1000};

  int preCreateSize;

  bool allowFileModification;

  Mutex mtx;

  string getStoreFile(AclIdent id)
  {
    char buf[32];
    sprintf(buf,"acl%09d.bin",id);
    return storeDir+buf;
  }

  string getIndexFile(AclIdent id)
  {
    char buf[32];
    sprintf(buf,"acl%09d.idx",id);
    return storeDir+buf;
  }

  AclIdent getNextId()
  {
    MutexGuard mg(idMtx);
    idSeq++;
    idFile.Seek(0);
    idFile.WriteNetInt32(idSeq);
    idFile.Flush();
    return idSeq;
  }

  void OpenOrCreateDH(DiskHash<AclDiskRecord,Int64Record>& dh,AclIdent id)
  {
    std::string fn=getIndexFile(id);
    if(File::Exists(fn.c_str()))
    {
      dh.Open(fn.c_str(),false,false);
    }else
    {
      dh.Create(fn.c_str(),preCreateSize,false);
    }
  }

public:
  AclManager():aclsFile(aclsFileSig,aclsFileVer)
  {
    allowFileModification=false;
  }

  //struct AclEditor
  virtual void    enumerate(vector<AclNamedIdent>& result);
  virtual AclInfo getInfo(AclIdent);
  virtual void    remove(AclIdent);
  virtual void    create(AclIdent,const char* aclname,const char* descr,const vector<AclPhoneNumber>& phones,AclCacheType act);
  virtual AclIdent create2(const char* aclname,const char* descr,const vector<AclPhoneNumber>& phones,AclCacheType act);
  virtual void    lookupByPrefix(AclIdent,const char* prefix,vector<AclPhoneNumber>&);
  virtual void    removePhone(AclIdent,const AclPhoneNumber&);
  virtual void    addPhone(AclIdent,const AclPhoneNumber&);
  virtual void    updateAclInfo(AclIdent,const char* aclname,const char* acldesc,AclCacheType act);

  //struct AclLookuper
  virtual bool    isGranted(AclIdent aclid,const AclPhoneNumber& phone);
  // struct AclAbstractManager

  virtual void LoadUp(const char* argAclStore,int argPreCreate);

  void enableControllerMode()
  {
    allowFileModification=true;
  }
};


void AclManager::LoadUp(const char* argAclStore,int argPreCreate)
{
  storeDir=argAclStore;
  if(storeDir.length()>0 && storeDir[storeDir.length()-1]!='/')
  {
    storeDir+='/';
  }
  aclsFile.Open((storeDir+"acls.bin").c_str());
  AclInfoRecord acl;
  while((acl.offset=aclsFile.Read(acl))!=0)
  {
    acls.Insert(acl.ident,acl);
  }
  string fn=storeDir+"aclidseq.bin";
  if(File::Exists(fn.c_str()))
  {
    idFile.RWOpen(fn.c_str());
    idSeq=idFile.ReadNetInt32();
  }else
  {
    idFile.RWCreate(fn.c_str());
    idSeq=0;
    idFile.WriteNetInt32(idSeq);
    idFile.Flush();
  }
  preCreateSize=argPreCreate;
}

void AclManager::enumerate(vector<AclNamedIdent>& result)
{
  MutexGuard mg(mtx);
  result.resize(0);
  int k;
  AclInfoRecord v;
  IntHash<AclInfoRecord>::Iterator it=acls.First();
  while(it.Next(k,v))
  {
    result.push_back(AclNamedIdent(v.ident,v.name));
  }

  /*
  static const char* sql = "SELECT ID,NAME FROM SMS_ACLINFO";

  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception(ACLMGRPREFIX"Failed to make a query to DB");
  while ( rs->fetchNext() )
    result.push_back(MakeAclNamedIdent(rs->getInt32(1),rs->getString(2)));
  */
}

AclInfo AclManager::getInfo(AclIdent aclident)
{
  MutexGuard mg(mtx);
  AclInfoRecord* aclptr=acls.GetPtr(aclident);
  if(!aclptr)throw Exception(ACLMGRPREFIX"Has no requested records");
  return *aclptr;
  /*
  static const char* sql = "SELECT ID,NAME,DESCRIPTION,CACHE_TYPE FROM SMS_ACLINFO WHERE ID=%d";
  vector<char> query(256);
  snprintf(&query[0],query.size(),sql,aclident);
  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(&query[0]));
  if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception(ACLMGRPREFIX"Failed to make a query to DB");
  if ( rs->fetchNext() ) {
    const char* cache_type = rs->getString(4);
    return MakeAclInfo(
            rs->getInt32(1),
            rs->getString(2),
            rs->getString(3),
            (cache_type?MakeAclCacheType(cache_type[0]):ACT_UNKNOWN));
  }else throw Exception(ACLMGRPREFIX"Has no requested records");
  */
}

void AclManager::remove(AclIdent aclident)
{
  MutexGuard mg(mtx);
  AclInfoRecord* aclptr=acls.GetPtr(aclident);
  if(!aclptr)throw Exception(ACLMGRPREFIX"Has no requested records");
  if(allowFileModification)
  {
    string fn=getStoreFile(aclptr->ident);
    if(File::Exists(fn.c_str()))File::Unlink(fn.c_str());
    fn=getIndexFile(aclptr->ident);
    if(File::Exists(fn.c_str()))File::Unlink(fn.c_str());
    aclsFile.Delete(aclptr->offset);
  }
  acls.Delete(aclptr->ident);
  /*
  static const char* sql = "DELETE FROM SMS_ACLINFO WHERE ID = %d";
  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  try
  {
    vector<char> query(256);
    snprintf(&query[0],query.size(),sql,aclident);
    auto_ptr<Statement> statement(connection->createStatement(&query[0]));
    if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
    statement->executeUpdate();
    connection->commit();
  }
  catch(exception& e)
  {
    connection->rollback();
    throw;
  }
  */
}

void AclManager::create(AclIdent aclident,const char* aclname,const char* acldescr,const vector<AclPhoneNumber>& phones,AclCacheType act)
{
  MutexGuard mg(mtx);

  AclInfoRecord aclr(MakeAclInfo(aclident,aclname,acldescr,act));
  if(allowFileModification)
  {
    aclr.offset=aclsFile.Append(aclr);
  }
  acls.Insert(aclident,aclr);

  if(allowFileModification)
  {
    if(!phones.empty())
    {
      DiskHash<AclDiskRecord,Int64Record> dh;
      FixedRecordFile<AclDiskRecord> frf(aclMembersSig,aclMembersVer);

      OpenOrCreateDH(dh,aclident);
      frf.Open(getStoreFile(aclident).c_str());

      for(vector<AclPhoneNumber>::const_iterator it=phones.begin();it!=phones.end();it++)
      {
        AclDiskRecord r(Address(it->c_str()));
        r.offset=frf.Append(r);
        dh.Insert(r,r.offset);
      }
    }
  }

  /*
  static const char* sql0 = "INSERT INTO SMS_ACLINFO (ID,NAME,DESCRIPTION,CACHE_TYPE) "
                            " VALUES (:1, :2, :3, :4)";
  static const char* sql1 = "INSERT INTO SMS_ACL (ID, ADDRESS) "
                            " VALUES (:1, :2)";

  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  try
  {
    {
      auto_ptr<Statement> statement(connection->createStatement(sql0));
      if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
      char cb[2] = { AclCacheTypeAsChar(act), 0 };
      statement->setInt32(1,aclident);
      statement->setString(2,aclname);
      statement->setString(3,acldescr);
      statement->setString(4,cb);
      statement->executeUpdate();
    }

    if ( !phones.empty() )
    {
      auto_ptr<Statement> statement(connection->createStatement(sql1));
      if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
      for ( vector<AclPhoneNumber>::const_iterator i = phones.begin(), iE = phones.end(); i != iE ; ++i )
      {
        statement->setInt32(1,aclident);
        statement->setString(2,AclPhoneAsString(*i).c_str());
        statement->executeUpdate();
      }
    }

    connection->commit();
  }
  catch(exception& e)
  {
    connection->rollback();
    throw;
  }
  */
}

AclIdent AclManager::create2(const char* aclname,const char* acldescr,const vector<AclPhoneNumber>& phones,AclCacheType act)
{
  AclIdent ident=getNextId();
  create(ident,aclname,acldescr,phones,act);
  return ident;
  /*
  static const char* sql0 = "SELECT SMS_ACLINFO_SEQ.NextVal FROM DUAL";

  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql0));
  if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception(ACLMGRPREFIX"Failed to make a query to DB");
  if (!rs->fetchNext())throw Exception(ACLMGRPREFIX"Failed on sequence query to DB");
  AclIdent ident = rs->getInt32(1);
  create(ident,aclname,acldescr,phones,act);
  return ident;
  */
}

void AclManager::lookupByPrefix(AclIdent aclident,const char* prefix,vector<AclPhoneNumber>& result)
{
  MutexGuard mg(mtx);
  if(!acls.Exist(aclident))throw Exception(ACLMGRPREFIX"Acl with ident %d doesn't exists",aclident);

  FixedRecordFile<AclDiskRecord> f(aclMembersSig,aclMembersVer);
  f.Open(getStoreFile(aclident).c_str());
  AclDiskRecord rec;
  char buf[32];
  size_t plen = prefix?strlen(prefix):0;
  __trace2__("lookupByPrefix(%d):%s",aclident,prefix?prefix:"");
  while(f.Read(rec))
  {
    size_t buflen=rec.addr.getText(buf,sizeof(buf));
    __trace2__("read: %s/%d",buf,buflen);
    if(buflen>=plen && (!prefix || memcmp(buf,prefix,plen)==0))
    {
      result.push_back(MakeAclPhoneNumber(buf));
    }
  }

  /*
  static const char* sql = "SELECT ADDRESS FROM SMS_ACL WHERE ID=%d";


  vector<char> query(1024);
  snprintf(&query[0],query.size(),sql,aclident);
  ConnectionGuard connection(datasource_);

  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(&query[0]));
  if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception(ACLMGRPREFIX"Failed to make a query to DB");

  unsigned plen = prefix?strlen(prefix):0;
  while( rs->fetchNext() )
  {
    string s = rs->getString(1);
    if ( s.length() >= plen && ( !prefix || memcmp(s.c_str(),prefix,plen) == 0 ) )
      result.push_back(MakeAclPhoneNumber(s));
  }
  */
}

void AclManager::removePhone(AclIdent aclident,const AclPhoneNumber& phone)
{
  MutexGuard mg(mtx);
  AclInfoRecord* recptr=acls.GetPtr(aclident);
  if(!recptr)throw Exception(ACLMGRPREFIX"Acl with id=%d doesn't exists",aclident);
  if(!allowFileModification)
  {
    return;
  }

  DiskHash<AclDiskRecord,Int64Record> dh;
  FixedRecordFile<AclDiskRecord> frf(aclMembersSig,aclMembersVer);

  OpenOrCreateDH(dh,aclident);
  frf.Open(getStoreFile(aclident).c_str());

  Int64Record off;
  if(dh.LookUp(Address(phone.c_str()),off))
  {
    dh.Delete(Address(phone.c_str()));
    frf.Delete(off.key);
  }else
  {
    throw Exception(ACLMGRPREFIX"Phone doesn't exists");
  }


  /*
  static const char* sql = "DELETE FROM SMS_ACL WHERE ID=%d AND ADDRESS='%s'";

  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  try
  {
    vector<char> query(256);
    snprintf(&query[0],query.size(),sql,aclident,AclPhoneAsString(phone).c_str());
    auto_ptr<Statement> statement(connection->createStatement(&query[0]));
    if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
    statement->executeUpdate();
    connection->commit();
  }
  catch(exception& e)
  {
    connection->rollback();
    throw;
  }
  */
}

void AclManager::addPhone(AclIdent aclident,const AclPhoneNumber& phone)
{
  MutexGuard mg(mtx);

  AclInfoRecord* recptr=acls.GetPtr(aclident);
  if(!recptr)throw Exception(ACLMGRPREFIX"Acl with id=%d doesn't exists",aclident);
  if(!allowFileModification)
  {
    return;
  }

  DiskHash<AclDiskRecord,Int64Record> dh;
  FixedRecordFile<AclDiskRecord> frf(aclMembersSig,aclMembersVer);

  Int64Record off;
  AclDiskRecord key(Address(phone.c_str()));
  OpenOrCreateDH(dh,aclident);
  if(dh.LookUp(key,off))
  {
    return;
  }
  frf.Open(getStoreFile(aclident).c_str());

  off=frf.Append(AclDiskRecord(Address(phone.c_str())));
  dh.Insert(key,off);
  /*
  const char* sql = "INSERT INTO SMS_ACL (ID, ADDRESS) VALUES (%d,'%s')";

  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  try
  {
    vector<char> query(256);
    snprintf(&query[0],query.size(),sql,aclident,AclPhoneAsString(phone).c_str());
    auto_ptr<Statement> statement(connection->createStatement(&query[0]));
    if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
    statement->executeUpdate();
    connection->commit();
  }
  catch(exception& e)
  {
    connection->rollback();
    throw;
  }
  */
}

void AclManager::updateAclInfo(AclIdent aclident,const char* aclname,const char* acldesc,AclCacheType act)
{
  MutexGuard mg(mtx);
  AclInfoRecord* recptr=acls.GetPtr(aclident);
  if(!recptr)throw Exception(ACLMGRPREFIX"Acl with id=%d doesn't exists",aclident);
  recptr->name=aclname;
  recptr->description=acldesc;
  recptr->cache=act;
  if(!allowFileModification)
  {
    return;
  }
  aclsFile.Write(recptr->offset,*recptr);
  /*
  static const char* sql = "UPDATE SMS_ACLINFO SET NAME=:1,DESCRIPTION=:2,CACHE_TYPE=:3 where id=:4";

  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  try
  {
    auto_ptr<Statement> statement(connection->createStatement(sql));
    if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
    char cb[2] = { AclCacheTypeAsChar(act), 0 };
    statement->setString(1,aclname);
    statement->setString(2,acldesc);
    statement->setString(3,cb);
    statement->setInt32(4,aclident);
    statement->executeUpdate();
    connection->commit();
  }
  catch(exception& e)
  {
    connection->rollback();
    throw;
  }
  */
}

bool AclManager::isGranted(AclIdent aclid,const AclPhoneNumber& phone)
{
  MutexGuard mg(mtx);
  if(!acls.Exist(aclid))return false;
  DiskHash<AclDiskRecord,Int64Record> dh;
  OpenOrCreateDH(dh,aclid);
  Int64Record rec;
  return dh.LookUp(Address(phone.c_str()),rec);

  /*
  static const char* sql = "SELECT ADDRESS FROM SMS_ACL WHERE ID=%d AND ADDRESS='%s'";
  vector<char> query(1024);
  snprintf(&query[0],query.size(),sql,aclid,AclPhoneAsString(phone).c_str());
  ConnectionGuard connection(datasource_);

  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(&query[0]));
  if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception(ACLMGRPREFIX"Failed to make a query to DB");

  return rs->fetchNext();
  */
}


AclAbstractMgr* AclAbstractMgr::Create()
{
  return new AclManager();
}

} // acls namespace
} // smsc namespace
