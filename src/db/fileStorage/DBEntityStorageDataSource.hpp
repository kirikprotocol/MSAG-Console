#ifndef __DBENTITYSTORAGE_DBENTITYSTORAGEDATASOURCE_HPP__
# define __DBENTITYSTORAGE_DBENTITYSTORAGEDATASOURCE_HPP__ 1

# include <db/DataSource.h>

# include <logger/Logger.h>

using namespace smsc::db;

extern "C" 
DataSourceFactory*  getDataSourceFactory(void);

class DBEntityStorageDataSource;

class DBEntityStorageDataSourceFactory : public DataSourceFactory
{
public:
  DBEntityStorageDataSourceFactory()
    : DataSourceFactory(), _logger(Logger::getInstance("dbStrg"))  {}
  virtual ~DBEntityStorageDataSourceFactory() {}

protected:
  virtual DataSource* createDataSource();
  friend class DBEntityStorageDataSource; // is necessary to declare createDataSource() as a friend

private:
  smsc::logger::Logger *_logger;
};

class DBEntityStorageDriver : public DBDriver
{
public:
        
  DBEntityStorageDriver(ConfigView* config)
    throw (ConfigException);
  //  virtual ~DBEntityStorageDriver();

  virtual Connection* newConnection();
private:
  smsc::logger::Logger *_logger;
};
    
class DBEntityStorageDataSource : public DataSource
{
public:

  virtual ~DBEntityStorageDataSource()
  {}

  // called for getting Connection to data source
  virtual Connection* getConnection();
  virtual void freeConnection(Connection* connection) {}
  virtual void closeConnections() {}
  virtual void closeRegisteredQueries(const char* id);
protected:
  DBEntityStorageDataSource() : DataSource(),_logger(Logger::getInstance("dbStrgDS")) {};

  virtual void init(ConfigView* config)
    throw(ConfigException)
  {
    driver = new DBEntityStorageDriver(config);
  }
private:
  smsc::logger::Logger *_logger;

  friend DataSource* DBEntityStorageDataSourceFactory::createDataSource();
};

class DBEntityStorageConnection : public Connection
{
public:
  DBEntityStorageConnection();
  virtual ~DBEntityStorageConnection();

  virtual Statement* createStatement(const char* sql)
    throw(SQLException);
  virtual Routine* createRoutine(const char* call, bool func=false) 
    throw(SQLException);

  virtual void connect() 
    throw(SQLException);
  virtual void disconnect();

  virtual void commit()
    throw(SQLException);
  virtual void rollback()
    throw(SQLException);
  virtual void abort()
    throw(SQLException);

protected:
  virtual void ping() {}

private:
  smsc::logger::Logger *_logger;

  bool traceIf(const char* what);
};

#endif
