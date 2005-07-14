#include "interfaces.h"
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <functional>
#include <memory>
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "core/buffers/FixedRecordFile.hpp"
#include "core/buffers/DiskHash.hpp"
#include "core/buffers/IntHash.hpp"
#include "sms/sms.h"
#include "sms/sms_util.h"
#include "util/crc32.h"
#include "db/DataSource.h"
#include "db/DataSourceLoader.h"
#include "logger/Logger.h"
#include "util/findConfigFile.h"

#define ACLMGRPREFIX "ACLMGR"

typedef smsc::util::config::Manager ConfigManager;
using namespace std;
using namespace smsc::acls;
using namespace smsc::sms;
using namespace smsc::logger;
using namespace smsc::util;
using namespace smsc::core::buffers;

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
    f.ReadFixedString<128>(desctiption);
    cache=MakeAclCacheType(f.ReadByte());
  }
  void Write(File& f)const
  {
    f.WriteNetInt32(ident);
    f.WriteFixedString<32>(name);
    f.WriteFixedString<128>(desctiption);
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


class ConnectionGuard{
  smsc::db::DataSource *ds;
  smsc::db::Connection *conn;
public:
  ConnectionGuard(smsc::db::DataSource *_ds):ds(_ds)
  {
    conn=ds->getConnection();
  }
  ~ConnectionGuard()
  {
    ds->freeConnection(conn);
  }
  smsc::db::Connection* operator->()
  {
    return conn;
  }
  smsc::db::Connection* get()
  {
    return conn;
  }
};


static const char* aclsFileSig="SMSCACLS";
static const uint32_t aclsFileVer=0x00010000;

static const char* aclMembersSig="SMSCACLMEM";
static const uint32_t aclMembersVer=0x00010000;

string storeDir;
int preCreateSize;

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

int main()
{
  try{
    Logger::Init();
    Logger* log=Logger::getInstance("ora2file");
    smsc::util::config::Manager::init(findConfigFile("config.xml"));
    using namespace smsc::db;
    using smsc::util::config::ConfigView;
    const char* OCI_DS_FACTORY_IDENTITY = "OCI";
    config::Manager& cfgman=config::Manager::getInstance();

    std::auto_ptr<ConfigView> dsConfig(new config::ConfigView(config::Manager::getInstance(), "StartupLoader"));
    DataSourceLoader::loadup(dsConfig.get());

    DataSource* dataSource = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
    std::auto_ptr<ConfigView> config(new ConfigView(cfgman,"DataSource"));

    dataSource->init(config.get());


    File idFile;
    int idSeq;
    enum{IdSequenceExtent=1000};
    IntHash<AclInfoRecord> acls;
    FixedRecordFile<AclInfoRecord> aclsFile(aclsFileSig,aclsFileVer);

    storeDir=cfgman.getString("acl.storeDir");
    if(storeDir.length()>0 && storeDir[storeDir.length()-1]!='/')
    {
      storeDir+='/';
    }
    aclsFile.Open((storeDir+"acls.bin").c_str());
    string fn=storeDir+"aclidseq.bin";
    idFile.RWCreate(fn.c_str());
    idSeq=0;
    idFile.WriteNetInt32(idSeq);
    idFile.Flush();
    preCreateSize=cfgman.getInt("acl.preCreateSize");

    static const char* sql = "SELECT ID,NAME,DESCRIPTION,CACHE_TYPE FROM SMS_ACLINFO";

    ConnectionGuard connection(dataSource);
    if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
    auto_ptr<Statement> statement(connection->createStatement(sql));
    if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
    auto_ptr<ResultSet> rs(statement->executeQuery());
    if(!rs.get())throw Exception(ACLMGRPREFIX"Failed to make a query to DB");
    while ( rs->fetchNext() )
    {
      int id=rs->getInt32(1);
      printf("acl:%d\n",id);
      AclInfoRecord rec=MakeAclInfo(id,rs->getString(2),rs->getString(3),MakeAclCacheType(*rs->getString(4)));
      if(id>idSeq)idSeq=id;
      rec.offset=aclsFile.Append(rec);
      acls.Insert(rec.ident,rec);

      static const char* sql2="select address from sms_acl where id=:id";
      auto_ptr<Statement> statement2(connection->createStatement(sql2));
      if(!statement2.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
      statement2->setInt32(1,id);

      auto_ptr<ResultSet> rs2(statement2->executeQuery());
      if(!rs2.get())throw Exception(ACLMGRPREFIX"Failed to make a query to DB");


      DiskHash<AclDiskRecord,Int64Record> dh;
      FixedRecordFile<AclDiskRecord> frf(aclMembersSig,aclMembersVer);


      OpenOrCreateDH(dh,id);
      frf.Open(getStoreFile(id).c_str());

      string phone;
      int recnum=0;
      while(rs2->fetchNext())
      {
        const char* addr=rs2->getString(1);
        //phone=addr;
        Int64Record off(frf.Append(AclDiskRecord(Address(addr))));
        dh.Insert(AclDiskRecord(Address(addr)),off);
        printf("rec:%d\r",recnum++);fflush(stdout);
      }
      printf("\n");
    }
    idSeq+=IdSequenceExtent-(idSeq%IdSequenceExtent);
    idFile.Seek(0);
    idFile.WriteNetInt32(idSeq);
 }catch(std::exception& e)
 {
   printf("Exception during import:%s\n",e.what());
 }
}