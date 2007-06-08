#ifndef __BDB_DB_FRAMEWORK_GENERICBDB_HPP__
# define __BDB_DB_FRAMEWORK_GENERICBDB_HPP__ 1

# include <map>
# include <list>
# include <string>
# include <memory>
# include <db/bdb_framework/bdb_api_wrapper.h>
# include <db/bdb_framework/StorableDbRecord_Iface.hpp>
# include <db/bdb_framework/Statements.hpp>
# include <util/Exception.hpp>

namespace smsc {
namespace db {
namespace bdb_framework {

/*
** Concrete class represents abstaction for Berkeley DB. DB is defined by RECORD and PRIMARY_KEY 
** template's parametes. DB can be opened/closed. User can create statments for working with DB
** over call to createStatement method. Set of statements supported by concrete DB instance
** is defined by createStatement specialization in derived class.
**
** So having classes GenericBDB and SecondaryIndex we can produce new class represeted some DB with unique 
** combination of RECORD, PRIMARY and SECONDARIES keys. For example, let's assume that we needs DB for
** storing information about automobile and that information we want to get by automobile VIN (it' primary key)
** and automabile producer company (it's secondary key). So we define class AutomobileRecord which 
** contains all necessary information about automobile and information about its primary key and defines 
** secondary key:
**
** Automobile_PK, Automobile_Producer_SK. After that we defined class AutomobileDB as
** class AutomobileDB : public GenericBDB<AutomobileRecord, AutomobileRecord::PrimaryKey>,
**                      public GenericBDB<AutomobileRecord, AutomobileRecord::PrimaryKey>::SecondaryIndex<Automobile_Producer_SK> > {
**    AutomobileDB(const std::string& dbName);
**    void open();
**    void close();
**
**    using smsc::db::bdb_framework::GenericBDB<AutomobileRecord, AutomobileRecord::PrimaryKey>::createStatement;
** }
** In open and close methods we make call to corresponding methods of parents classes.
*/
template <class RECORD, class PRIMARY_KEY>
class GenericBDB {
public:
  GenericBDB(const std::string& dbName);
  virtual ~GenericBDB();

  void open();
  void close();

  template <class STATEMENT>
  std::auto_ptr<STATEMENT> createStatement() { throw smsc::util::CustomException("GenericBDB::createStatement::: generic statement isn't realized"); }

  /*
  ** Class supports list of opened statements.
  ** When DB is being closed all opened statements in list is being closed also.
  */
  void registerOpenedStatement(DBCloseEvent_Iface* openedStatement) {
    _opened_statements.push_back(openedStatement);
  }
  void unregisterOpenedStatement(DBCloseEvent_Iface* openedStatement) {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "### GenericBDB::unregisterOpenedStatement:: remove registred statement=%p", openedStatement);
    _opened_statements.remove(openedStatement);
  }

  /*
  ** Inner template class SecondaryIndex represent abstraction for Secondary Index.
  ** Secondary Index is related with main (GeneriBDB) DB so it is defined as inner class.
  ** Index can be opend and closed. For concrete specialization of SecondaryIndex class can be called
  ** createQueryStatement method which returned statement object for query records by SECONDARY_KEY.
  */
  template<class SECONDARY_KEY>
  class SecondaryIndex : public Base_SecondaryIndex {
  public:
    SecondaryIndex(GenericBDB<RECORD, PRIMARY_KEY>& generic_bdb)
      : _generic_bdb(generic_bdb), _idx_dbp(NULL), _index_opened(false), _index_closed(false) {
      // Register new instance of SecondaryIndex class in corresponde instance of GenericBDB class.
      _generic_bdb.register_bdb_idx(SECONDARY_KEY::getKeyName(), this);
    }

    virtual ~SecondaryIndex() { closeIdx(); }

    void openIdx() {
      if ( !_index_opened ) {
        DB_ENV* dbEnv = _generic_bdb.getEnvHandle();
        if ( !dbEnv )
          throw smsc::util::CustomException("SecondaryIndex::openIdx::: can't get Berkeley environment handle");

        DB* dbp = _generic_bdb.getDbHandle();
        if ( !dbp )
          throw smsc::util::CustomException("SecondaryIndex::openIdx::: can' get Berkeley db handle");

        std::string idxFileName = _generic_bdb._dbName + SECONDARY_KEY::getKeyName() + "_idx";
        if ( create_index(&_idx_dbp, dbp, dbEnv, idxFileName.c_str(), SECONDARY_KEY::keyValueExtractor) )
          throw smsc::util::CustomException("SecondaryIndex::openIdx::: call to create_index failed [%s]", get_error_message());

        _index_opened = true;
      }
    }

    void closeIdx() {
      if ( _index_opened && !_index_closed ) {
        close_db(_idx_dbp); _idx_dbp = NULL;
        _index_closed = true;
      }
    }

    std::auto_ptr<QueryByNonuniqKey<RECORD, SECONDARY_KEY> > createQueryStatement() {
      std::auto_ptr<QueryByNonuniqKey<RECORD, SECONDARY_KEY> > statement(new QueryByNonuniqKey<RECORD, SECONDARY_KEY>(*this));
      return statement;
    }

    void registerOpenedStatement(DBCloseEvent_Iface* openedStatement) {
      _generic_bdb.registerOpenedStatement(openedStatement);
    }
    void unregisterOpenedStatement(DBCloseEvent_Iface* openedStatement) {
      _generic_bdb.unregisterOpenedStatement(openedStatement);
    }

  private:
    // break class incapsulation for friend classes only
    DB* getDbHandle() { return _idx_dbp; }
    // for access to preceding method
    friend class QueryByNonuniqKey<RECORD, SECONDARY_KEY>;

    GenericBDB<RECORD, PRIMARY_KEY>& _generic_bdb;
    DB* _idx_dbp;
    bool _index_opened, _index_closed;
  };

protected:
  class Base_SecondaryIndex {
  public:
    virtual ~Base_SecondaryIndex() {}
    virtual void openIdx() = 0;
    virtual void closeIdx() = 0;
  };

  friend class SecondaryIndex<SECONDARY_KEY>;

  /*
  ** Class supports list of opened indexes. When DB is being closed
  ** all opened indexs is being closed also.
  */
  Base_SecondaryIndex* get_bdb_idx(const std::string& key_name) {
    bdb_indexes_t::iterator iter = _bdb_indexes.find(key_name);
    if ( iter != _bdb_indexes.end() )
      return iter->second;
    else
      throw smsc::util::CustomException("get_bdb_idx: unknown key '%s'", key_name.c_str());
  }

  void register_bdb_idx(std::string kName,
                        Base_SecondaryIndex* kType) {
    _bdb_indexes.insert(std::make_pair(kName, kType));
  }

  // break class incapsulation for friend classes only
  DB* getDbHandle() { return _dbp; }
  DB_ENV *getEnvHandle() { return _dbEnv; }
  // for access to above two methods
  friend class QueryByPrimaryKey<RECORD>;
  friend class Base_Join<RECORD>;
  friend class InsertStatement<RECORD>;

  const std::string _dbName;

  DB_ENV *_dbEnv;
  DB *_dbp;

  bool _hasBeenOpened, _hasBeenClosed;
  typedef std::map<std::string, Base_SecondaryIndex*> bdb_indexes_t;
  bdb_indexes_t _bdb_indexes;

  typedef std::list<DBCloseEvent_Iface*> opened_statements_t;
  opened_statements_t _opened_statements;
};

template <class RECORD, class PRIMARY_KEY>
GenericBDB<RECORD,PRIMARY_KEY>::GenericBDB(const std::string& dbName)
  : _dbName(dbName), _dbEnv(NULL), _dbp(NULL), _hasBeenOpened(false), _hasBeenClosed(false) {}

template <class RECORD, class PRIMARY_KEY>
GenericBDB<RECORD,PRIMARY_KEY>::~GenericBDB() {
  close();
}

template <class RECORD, class PRIMARY_KEY>
void
GenericBDB<RECORD,PRIMARY_KEY>::open()
{
  if ( !_hasBeenOpened ) {
    _dbEnv = db_setup();
    if ( !_dbEnv ) 
      throw smsc::util::CustomException("GenericDB::openDB::: db_setup failed [%s]", get_error_message());

    if ( open_db(&_dbp, _dbEnv, _dbName.c_str()) )
      throw smsc::util::CustomException("GenericDB::openDB::: open_db failed [%s]", get_error_message());

    for (bdb_indexes_t::iterator iter=_bdb_indexes.begin(), end_iter=_bdb_indexes.end();
         iter != end_iter; ++iter)
      iter->second->openIdx();

    _hasBeenOpened = true;
  }
}

template <class RECORD, class PRIMARY_KEY>
void
GenericBDB<RECORD,PRIMARY_KEY>::close()
{
  if ( !_hasBeenClosed && _hasBeenOpened ) {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "### GenericBDB::close:: bypass opened statements and notify it");
    // Bypass all opened statements and notify it.
    // The notify method realization for each statement must close it.
    for(opened_statements_t::iterator iter=_opened_statements.begin(), end_iter=_opened_statements.end();
        iter!=end_iter; ++iter)
      (*iter)->notify();

    // Close all opened indexes before close db.
    for (bdb_indexes_t::iterator iter=_bdb_indexes.begin(), end_iter=_bdb_indexes.end();
         iter != end_iter; ++iter)
      iter->second->closeIdx();

    close_db(_dbp); _dbp = NULL;
    close_env(_dbEnv); _dbEnv = NULL;

    _hasBeenClosed = true; _hasBeenOpened = false;
  }
}

}}}

#endif
