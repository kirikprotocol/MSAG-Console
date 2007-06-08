#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <db.h>
#include <assert.h>
#include <vector>
#include "bdb_api_wrapper.h"

#include <logger/Logger.h>
// for debug
static void hex_dump(const char* what, uint8_t* buf, uint32_t sz)
{
  printf("%s=[", what);
  for(uint32_t i=0; i<sz; ++i)
    printf("%02X", buf[i]);
  printf("]\n");
}


static const int MAX_ERR_MSG_LEN = 256;

static pthread_key_t err_messge_key;

static bool initialize_tsd()
{
  int ret = pthread_key_create(&err_messge_key, free);
  assert(ret == 0);
  return true;
}

static bool tsdWasInitialized = initialize_tsd();

static void set_error_message(int ret_code,
                              const char* what="")
{
  char *tsdErrMsg;
  
  if ( !(tsdErrMsg = static_cast<char*>(pthread_getspecific(err_messge_key))) ) {
    tsdErrMsg = static_cast<char*>(malloc(MAX_ERR_MSG_LEN));
    if ( !tsdErrMsg ) return;
    pthread_setspecific(err_messge_key, tsdErrMsg);
  }
  snprintf(tsdErrMsg, MAX_ERR_MSG_LEN, "%s (%s)", db_strerror(ret_code), what);
}

#include <string>
#include <sstream>

struct BerkeleyDB_Config
{
  BerkeleyDB_Config() : dbSubDir(""), cacheSize(0), isNeedLog(false), isNeedTransaction(false) {}
  std::string dbSubDir;
  uint32_t cacheSize;
  bool isNeedLog, isNeedTransaction;

  std::string toString() {
    std::ostringstream obuf;
    obuf << "dbSubDir=[" << dbSubDir
         << "],cacheSize=[" << cacheSize
         << "],isNeedLog=[" << isNeedLog
         << "],isNeedTransaction=[" << isNeedTransaction
         << "]";
    return obuf.str();
  };
};

static BerkeleyDB_Config* _db_config;
static DB_ENV *_dbenv;

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

class BerkeleyDB_Config_Reader : public BerkeleyDB_Config
{
public:
  void read(smsc::util::config::Manager& manager)
  {
    if (!manager.findSection("db_parameters"))
      throw smsc::util::config::ConfigException("\'db_parameters\' section is missed");
    smsc::util::config::ConfigView dbParamCfg(manager, "db_parameters");

    dbSubDir = dbParamCfg.getString("db_subdir");

    try {
      cacheSize = dbParamCfg.getInt("cacheSize");
    } catch(smsc::util::config::ConfigException& exc) {}

    try {
      isNeedLog = dbParamCfg.getBool("isNeedLog");
    } catch(smsc::util::config::ConfigException& exc) {}

    try {
      isNeedTransaction = dbParamCfg.getBool("isNeedTransaction");
    } catch(smsc::util::config::ConfigException& exc) {}
  }
};

const char* get_error_message()
{
  char *tsdErrMsg = static_cast<char*>(pthread_getspecific(err_messge_key));
  if ( !tsdErrMsg ) return "";
  else return tsdErrMsg;
}

DB_ENV *
db_setup()
{
  int ret;

  if ( !_db_config ) {
    BerkeleyDB_Config_Reader* readableConfig = new BerkeleyDB_Config_Reader();
    readableConfig->read(smsc::util::config::Manager::getInstance());
    _db_config = readableConfig;
  } else return _dbenv;

  /*
   * Create an environment and initialize it for additional error
   * reporting.
   */
  if ((ret = db_env_create(&_dbenv, 0)) != 0) {
    set_error_message(ret, "call to db_env_create failed");
    _dbenv->close(_dbenv, 0);
    return _dbenv = NULL;
  }

  if ( _db_config->cacheSize > 0 ) { // cachesize value must be specified in MB units
    if ((ret = _dbenv->set_cachesize(_dbenv, 0, _db_config->cacheSize * 1024 * 1024, 0)) != 0) {
      set_error_message(ret, "call to set_cachesize failed");
      _dbenv->close(_dbenv, 0);
      return _dbenv = NULL;
    }
  }

  u_int32_t dbenv_flags = DB_CREATE /*| DB_RECOVER*/ | DB_THREAD | DB_INIT_LOCK | DB_INIT_MPOOL;
  if ( _db_config->isNeedLog ) {
    dbenv_flags |= DB_INIT_LOG;
  } if ( _db_config->isNeedTransaction ) {
    dbenv_flags |= DB_INIT_TXN;
  }
  if ((ret = _dbenv->open(_dbenv, _db_config->dbSubDir.c_str(), dbenv_flags, 0)) != 0) {
    char what[MAX_ERR_MSG_LEN];
    snprintf(what,MAX_ERR_MSG_LEN, "environment open failed: %s", _db_config->dbSubDir.c_str());
    set_error_message(ret, what);
    _dbenv->close(_dbenv, 0);
    return _dbenv = NULL;
  }

  return (_dbenv);
}

int
open_db(DB **dbpp, DB_ENV *dbEnv, const char *dbFileName, int is_secondary)
{
  DB *dbp;    /* For convenience */
  u_int32_t open_flags;
  int ret;
  char errMsg[MAX_ERR_MSG_LEN];

  /* Initialize the DB handle */
  ret = db_create(&dbp, dbEnv, 0);
  if (ret != 0) {
    set_error_message(ret, "call to db_create failed");
    return ret;
  }
  /* Point to the memory malloc'd by db_create() */
  *dbpp = dbp;

  /*
   * If this is a secondary database, then we want to allow
   * sorted duplicates.
   */
  if (is_secondary) {
    ret = dbp->set_flags(dbp, DB_DUPSORT);
    if (ret != 0) {
      set_error_message(ret, "Attempt to set DUPSORT flags failed");
      return ret;
    }
  }

  /* Set the open flags */
  open_flags = DB_CREATE | DB_THREAD;    /* Allow database creation and free-threaded DB handle */

  /* Now open the database */
  ret = dbp->open(dbp,        /* Pointer to the database */
                  NULL,       /* Txn pointer */
                  dbFileName,  /* File name */
                  NULL,       /* Logical db name */
                  DB_BTREE,   /* Database type (using btree) */
                  open_flags, /* Open flags */
                  0);         /* File mode. Using defaults */
  if (ret != 0) {
    char open_failed_msg[MAX_ERR_MSG_LEN];
    snprintf(open_failed_msg, sizeof(open_failed_msg), "Database '%s' open failed.", dbFileName);
    set_error_message(ret, open_failed_msg);
    return ret;
  }

  return 0;
}

/* Close the database. */
int
close_db(DB *dbp)
{
  int ret;
  /*
   * Note that closing a database automatically flushes its cached data
   * to disk, so no sync is required here.
   */
  if (dbp) {
    ret = dbp->close(dbp, 0);
    if (ret != 0) {
      set_error_message(ret, "Database close failed");
      return ret;
    }
  }
  return (0);
}

void
close_env(DB_ENV *dbEnv)
{
  if ( dbEnv )
    dbEnv->close(dbEnv, 0);
}

/*
** Return value:
** 1 - record was inserted
** 0 - value can't be inserted because of dup index value existence
** < 0 - error occured
*/
int insert_record(DB *dbp,
                  const void* key, size_t keySize,
                  const void* value, size_t valueSize)
{
  DBT db_key, db_data;

  /* Zero out the DBTs before using them. */
  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  std::vector<uint8_t> non_const_key_value(static_cast<const uint8_t*>(key), static_cast<const uint8_t*>(key) + keySize);
  std::vector<uint8_t> non_const_data_value(static_cast<const uint8_t*>(value), static_cast<const uint8_t*>(value) + valueSize);

  db_key.data = &non_const_key_value[0];
  db_key.size = non_const_key_value.size();

  db_data.data = &non_const_data_value[0];
  db_data.size = non_const_data_value.size(); 

  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");

  int ret = dbp->put(dbp, NULL, &db_key, &db_data, DB_NOOVERWRITE);
  if ( ret ) {
    if (ret == DB_KEYEXIST) {
      set_error_message(ret, "Put failed because of duplicate index value");
      return 0;
    } else {
      set_error_message(ret, "Can't insert value. call to db->put was failed");
      return -1;
    }
  }
  return 1;
}

/*
** Return value:
** 1 - record found
** 0 - record not found
** < 0 - error occured
*/
int find_at_most_one_record_by_key(DB *dbp,
                                   const void* key, size_t keySize,
                                   void* value, size_t maxValueSize, size_t* realValueSize)
{
  DBT db_key, db_data;

  /* Zero out the DBTs before using them. */
  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  std::vector<uint8_t> non_const_key_value(static_cast<const uint8_t*>(key), static_cast<const uint8_t*>(key) + keySize);

  db_key.data = &non_const_key_value[0];
  db_key.size = keySize;

  db_data.data = value;
  db_data.ulen = maxValueSize;
  db_data.flags = DB_DBT_USERMEM;

  int ret = dbp->get(dbp, NULL, &db_key, &db_data, 0);
  if ( ret ) {
    if ( ret == DB_NOTFOUND ) return 0;
    else {
      set_error_message(ret, "call to dp->get was failed");
      return -1;
    }
  }
  *realValueSize = db_data.size;
  return 1;
}

int fetch_first_record_by_key(DB *secondary_dbp,
                              DBC **cursorpp,
                              const void* key, size_t keySize,
                              void* value, size_t maxValueSize, size_t* realValueSize)
{
  DBT db_key, db_data;

  /* Initialize our DBTs. */
  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  /* Get a cursor to the db */
  secondary_dbp->cursor(secondary_dbp, 0,
                        cursorpp, 0);

  DBC *c_dbp = *cursorpp;

  if ( !c_dbp )
    return 0;

  /*
   * Get the search key.
   */
  std::vector<uint8_t> non_const_key_value(static_cast<const uint8_t*>(key), static_cast<const uint8_t*>(key) + keySize);
  db_key.data = &non_const_key_value[0];
  db_key.size = keySize;

  db_data.data = value;
  db_data.ulen = maxValueSize;
  db_data.flags = DB_DBT_USERMEM;
  /*
   * Position our cursor to the first record in the secondary
   * database that has the appropriate key.
   */
  int ret = c_dbp->c_get(c_dbp, &db_key, &db_data, DB_SET);
  if ( !ret ) {
    *realValueSize = db_data.size;
    return 1;
  } else {
    c_dbp->c_close(c_dbp);
    c_dbp = NULL;
    if ( ret == DB_NOTFOUND ) {
      return 0;
    } else {
      set_error_message(ret, "call to c_dbp->c_get was failed");
      return -1;
    }
  }
}

int fetch_next_record_by_key(DB *secodary_dbp,
                             DBC **cursorpp,
                             const void* key, size_t keySize,
                             void* value, size_t maxValueSize, size_t* realValueSize)
{
  DBT db_key, db_data;

  /* Initialize our DBTs. */
  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));
  /*
   * Get the search key.
   */
  std::vector<uint8_t> non_const_key_value(static_cast<const uint8_t*>(key), static_cast<const uint8_t*>(key) + keySize);
  db_key.data = &non_const_key_value[0];
  db_key.size = keySize;

  db_data.data = value;
  db_data.ulen = maxValueSize;
  db_data.flags = DB_DBT_USERMEM;

  DBC *c_dbp = *cursorpp;

  if ( !c_dbp )
    return 0;

  int ret = c_dbp->c_get(c_dbp, &db_key, &db_data,
                         DB_NEXT_DUP);
  if ( ret == 0 ) {
    *realValueSize = db_data.size;
    return 1;
  } else {
    c_dbp->c_close(c_dbp);
    *cursorpp = NULL;

    if ( ret == DB_NOTFOUND ) {
      return 0;
    } else {
      set_error_message(ret, "call to c_dbp->c_get was failed");
      return -1;
    }
  }
}
/*
** Return value:
** 1 - record was deleted
** 0 - record not found
** < 0 - error occured
*/
int delete_record_by_key(DB *dbp,
                         const void* key, size_t keySize)
{
  DBT db_key;

  /* Zero out the DBT before using them. */
  memset(&db_key, 0, sizeof(DBT));
  
  std::vector<uint8_t> non_const_key_value(static_cast<const uint8_t*>(key), static_cast<const uint8_t*>(key) + keySize);
  
  db_key.data = &non_const_key_value[0];
  db_key.size = keySize;

  int ret = dbp->del(dbp, NULL, &db_key, 0);
  if ( ret ) {
    if ( ret == DB_NOTFOUND ) return 0;
    else {
      set_error_message(ret, "call to dp->del was failed");
      return -1;
    }
  }
  return 1;
}

/*
** Update record data value by primary key
** Return value:
** 1 - record was updated
** 0 - record not found
** < 0 - error occured
*/
int update_record_by_primary_key(DB *dbp,
                                 const void* key, size_t keySize,
                                 const void* replacingValue, size_t replacingValueSize)
{
  DBC *cursorp;

  /* Get the cursor */
  int ret = dbp->cursor(dbp, NULL, &cursorp, 0);
  if (ret || !cursorp) {
    set_error_message(ret, "call to dbp->cursor was failed");
    return -1;
  }

  DBT db_key, db_data;

  /* Zero out the DBTs before using them. */
  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  std::vector<uint8_t> non_const_key_value(static_cast<const uint8_t*>(key), static_cast<const uint8_t*>(key) + keySize);

  db_key.data = &non_const_key_value[0];
  db_key.size = non_const_key_value.size();

  /* Position the cursor */
  ret = cursorp->c_get(cursorp, &db_key, &db_data, DB_SET);
  if (ret == 0) {
    std::vector<uint8_t> non_const_data_value(static_cast<const uint8_t*>(replacingValue), static_cast<const uint8_t*>(replacingValue) + replacingValueSize);
    db_data.data = &non_const_data_value[0];
    db_data.size = non_const_data_value.size();
    ret = cursorp->c_put(cursorp, &db_key, &db_data, DB_CURRENT);
  }

  cursorp->c_close(cursorp); 
  if ( ret ) {
    if ( ret == DB_NOTFOUND )
      return 0;
    else set_error_message(ret, "call to cursor->c_get was failed");
      return -1;
  } else return 1;

}

int
create_index(DB **idx_dbpp,
             DB *dbp,
             DB_ENV *dbEnv,
             const char *idxFileName,
             index_key_creator_func_t callback)
{
  if ( open_db(idx_dbpp, dbEnv, idxFileName, 1) )
    return -1;

  DB *sdbp = *idx_dbpp;

  /* Now associate the secondary to the primary */
  int ret = dbp->associate(dbp,            /* Primary database */
                           NULL,           /* TXN id */
                           sdbp,           /* Secondary database */
                           callback,       /* Callback used for key creation. */
                           0);             /* Flags */

  return 0;
}

int position_cursor_to_first_record(DB *secondary_dbp,
                                    DBC **cursorpp,
                                    const void* key, size_t keySize)
{
  DBT db_key, db_data;

  /* Initialize our DBTs. */
  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  /* Get a cursor to the db */
  secondary_dbp->cursor(secondary_dbp, 0,
                        cursorpp, 0);

  DBC *c_dbp = *cursorpp;

  if ( !c_dbp )
    return 0;

  /*
   * Get the search key.
   */
  std::vector<uint8_t> non_const_key_value(static_cast<const uint8_t*>(key), static_cast<const uint8_t*>(key) + keySize);
  db_key.data = &non_const_key_value[0];
  db_key.size = keySize;

  /*
   * Position our cursor to the first record in the secondary
   * database that has the appropriate key.
   */
  int ret = c_dbp->c_get(c_dbp, &db_key, &db_data, DB_SET);
  if ( ret == DB_NOTFOUND )
    return 0;
  else if ( ret != 0 ) {
    set_error_message(ret, "call to c_dbp->c_get was failed");
    return -1;
  }

  return 1;
}

int iterate_over_join(DBC* join_curs,
                      void* key, size_t maxKeySize, size_t* realKeySize,
                      void* value, size_t maxValueSize, size_t* realValueSize)
{
  DBT db_key, db_data;

  /* Zero out the DBTs before using them. */
  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  db_key.data = key;
  db_key.ulen = maxKeySize;
  db_key.flags = DB_DBT_USERMEM;

  db_data.data = value;
  db_data.ulen = maxValueSize;
  db_data.flags = DB_DBT_USERMEM;

  /* Iterate using the join cursor */
  int ret = join_curs->c_get(join_curs, &db_key, &db_data, 0);
  if (ret == 0) {
    *realValueSize = db_data.size; *realKeySize = db_key.size;
    return 1;
  } else {
    if ( ret == DB_NOTFOUND ) return 0;
    else {
      set_error_message(ret, "call to join_curs->c_get was failed");
      return -1;
    }
  }
}
