
#include "interfaces.h"

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <functional>

#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "db/DataSource.h"
#include "db/DataSourceLoader.h"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

#define ACLMGRPRIFEX "Acl:"

namespace smsc {
namespace acls {

using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::util;
using namespace smsc::db;

using namespace std;

typedef smsc::util::config::Manager ConfigManager;

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

struct AclRecord
{
  AclInfo                info;
  unsigned               usecount;
  //vector<AclPhoneNumber> granted_cache;
};

class AclManager : public AclAbstractMgr
{
  typedef map<AclInfo,AclRecord> records_t;
  records_t records_;
  DataSource dataSource_;

  Mutex aclinfo_locker_;
  //Mutex granted_locker_;

public:
  //struct AclEditor
  virtual void    enumerate(vector<AclNamedIdent>& result);
  virtual AclInfo getInfo(AclIdent);  
  virtual bool    remove(AclIdent);
  virtual void    create(AclIdent,const char* aclname,const char* descr,const vector<AclPhoneNumber>& phones,AclCacheType act);
  virtual bool    lookupByPrefix(AclIdent,const char* prefix,vector<AclPhoneNumber>&);
  virtual bool    removePhone(AclIdent,const AclPhoneNumber&);
  virtual void    addPhone(AclIdent,const AclPhoneNumber&);
  virtual void    updateAclInfo(AclIdent,const char* aclname,const char* acldesc,AclCacheType act);
  //struct AclLookuper
  virtual bool    isGranted(AclIdent aclid,const AclPhoneNumber& phone);
  // struct AclAbstractManager
  virtual void LoadUp();
};

void AclManager::enumerate(vector<AclNamedIdent>& result)
{
}

AclInfo AclManager::getInfo(AclIdent)
{
  return AclInfo();
}

void AclManager::remove(AclIdent aclident)
{
  const char* sql = "DELETE FROM SMS_ACLINFO WHERE ID = %d"
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
  return false;
}

void AclManager::create(AclIdent aclident,const char* aclname,const char* acldescr,const vector<AclPhoneNumber>& phones,AclCacheType act)
{
  const char* sql0 = "INSERT INTO SMS_ACLINFO (ID,NAME,DESCRIPTION,CACHE_TYPE) "
                     " VALUES (:1, :2, :3, :4)";
  const char* sql1 = "INSERT INTO SMS_ACL (ID, ADDRESS) "
                      " VALUES (:1, :2)";
  
  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  try 
  {
    {
      auto_ptr<Statement> statement(connection->createStatement(sql0));
      if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
      char cb[2] = { AclCahceTypeAsChar(act), 0 };
      statement->setInt32(1,aclident);
      statement->setString(2,aclname);
      statement->setString(3,acldesc);
      statement->setString(4,&cb);
      statement->executeUpdate();
    }

    if ( !phones.empty() )
    {
      auto_ptr<Statement> statement(connection->createStatement(sql1));
      if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
      for ( vector<AclPhoneNumber>::iterator i = phones.begin(); i!=phones.end(); ++i )
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
}

bool AclManager::lookupByPrefix(AclIdent,const char* prefix,vector<AclPhoneNumber>&)
{
  return false;
}

void AclManager::removePhone(AclIdent aclidnt,const AclPhoneNumber& phone)
{
  const char* sql = "DELETE FROM SMS_ACL WHERE ID=%d AND ADRESS='%s'";

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
}

void AclManager::addPhone(AclIdent aclident,const AclPhoneNumber& phone)
{
  const char* sql = "INSERT INTO SMS_ACL (ID, ADDRESS) VALUES (%d,'%s')";

  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  try 
  {
    vector<char> query(256);
    snprintf(&query[0],query.size(),sql,AclPhoneAsString(phone).c_str());
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
}

void AclManager::updateAclInfo(AclIdent aclident,const char* aclname,const char* acldesc,AclCacheType act)
{
  const char* sql = "UPDATE SMS_ACLINFO SET NAME=:1,DESCRIPTION=:2,CACHE_TYPE=:3) where ident=:4";
  
  ConnectionGuard connection(datasource_);
  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  try 
  {
    auto_ptr<Statement> statement(connection->createStatement(sql));
    if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
    char cb[2] = { AclCahceTypeAsChar(act), 0 };
    statement->setString(1,aclname);
    statement->setString(2,acldesc);
    statement->setString(3,&cb);
    statement->setInt32(4,aclident);
    statement->executeUpdate();
    connection->commit();
  }
  catch(exception& e)
  {
    connection->rollback();
    throw;
  }
}

bool AclManager::isGranted(AclIdent aclid,const AclPhoneNumber& phone)
{
  const char* sql = "SELECT ADDRESS FROM SMS_ACL WHERE ID=%d AND ADDRESS='%s'";
  vector<char> query(1024);
  snprintf(&query[0],query.size(),sql,aclid,AclPhoneAsString(phone).c_str());
  ConnectionGuard connection(datasource_);

  if(!connection.get())throw Exception(ACLMGRPREFIX"Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(&query[0]));
  if(!statement.get())throw Exception(ACLMGRPREFIX"Failed to create statement");
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception(ACLMGRPREFIX"Failed to make a query to DB");

  return rs->fetchNext();
}

void AclManager::LoadUp(DataSource* ds)
{
  datasource_ = ds;
}

AclAbstractMgr* AclAbstractMgr::Create()
{
  return new AclManager();
}

} // acls namespace
} // smsc namespace
