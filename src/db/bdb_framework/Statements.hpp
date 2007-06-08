#ifndef __SMSC_DB_BDB_STATEMENTS_HPP__
# define __SMSC_DB_BDB_STATEMENTS_HPP__ 1

# include <db/bdb_framework/StorableDbRecord_Iface.hpp>
# include <core/synchronization/EventMonitor.hpp>
# include <core/synchronization/Mutex.hpp>
# include <util/Exception.hpp>
# include <logger/Logger.h>

namespace smsc {
namespace db {
namespace bdb_framework {

/*
** helper class for tracking shutdown db condition when execute any db statements.
*/
class DbShutdownChecker {
public:
  DbShutdownChecker() : _dbShutdownInProgress(false), _operationIsActive(false) {}

  typedef enum { OPERATION_IS_ACTIVE=1,OPERATION_IS_INACTIVE } flg_t;

  bool checkShutdowInProgressAndSetFlg(flg_t flg) {
    smsc::core::synchronization::MutexGuard lockGuard(_dbShutdownEvent);
    if (flg == OPERATION_IS_ACTIVE) _operationIsActive = true;
    else if (flg == OPERATION_IS_INACTIVE && _operationIsActive) {
      _operationIsActive = false;
      _dbShutdownEvent.notify();
    }
    if (_dbShutdownInProgress) return true;
    else return false;
  }

  void setShutdownInProgress() {
    core::synchronization::MutexGuard lockGuard(_dbShutdownEvent);
    while(_operationIsActive)
      _dbShutdownEvent.wait();
    _dbShutdownInProgress = true;
  }
private:
  smsc::core::synchronization::EventMonitor _dbShutdownEvent;
  bool _dbShutdownInProgress, _operationIsActive;
};


/*
** DB close event notifier. The method notify() is being called for all opened statement when db is being closed.
*/
class DBCloseEvent_Iface {
public:
  virtual void notify() = 0;
};

/*
** The Statement abstraction. The statement can be executed so execute() is only available method.
*/
class Statement {
public:
  Statement() {}
  virtual ~Statement() {}
  /*
  ** Returns num. of processed records or generates exception on failure
  */
  virtual int execute() = 0;
private:
  Statement(const Statement& rhs);
  Statement& operator=(const Statement& rhs);
};

template <class RECORD>
class InsertStatement : public Statement,
                        public DBCloseEvent_Iface {
public:
  InsertStatement(GenericBDB<RECORD, RECORD::PrimaryKey>& db) : _db(db), _record(NULL) {}
  ~InsertStatement() { delete _record; }

  void setValue(const RECORD& record) { _record = new RECORD(record); }
  virtual int execute() {
    std::vector<uint8_t> key_rawbuf;
    _record->getPrimaryKey().marshal(key_rawbuf);

    std::vector<uint8_t> record_rawbuf;
    _record->marshal(record_rawbuf);

    if ( _dbShutdownChecker.checkShutdowInProgressAndSetFlg(DbShutdownChecker::OPERATION_IS_ACTIVE) )
      throw smsc::util::CustomException("InsertStatement::execute::: db shutdown in progress");

    int res = insert_record(_db.getDbHandle(), &key_rawbuf[0], key_rawbuf.size(),
                            &record_rawbuf[0], record_rawbuf.size());

    if ( _dbShutdownChecker.checkShutdowInProgressAndSetFlg(DbShutdownChecker::OPERATION_IS_INACTIVE) )
      throw smsc::util::CustomException("InsertStatement::execute::: db shutdown in progress");

    if ( res < 0 )
      throw smsc::util::CustomException("InsertStatement::execute::: Error occured: %s", get_error_message());

    return res;
  }

  virtual void notify() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "### InsertStatement::notify:::");
    _dbShutdownChecker.setShutdownInProgress();
  }
private:
  GenericBDB<RECORD, RECORD::PrimaryKey>& _db;
  const RECORD* _record;
  DbShutdownChecker _dbShutdownChecker;
};

/*
** Example of ResultSet usage:
**
** someQueryStatement.execute();
** ResultSet<SomeRecord> resultSet = someQueryStatement.getResultSet();
** SomeRecord result;
** while (resultSet.fetchNextValue(result))
**   std::cout << "found value=[" << result.toString() << "]" << std::endl;
*/
template <class T>
class ResultSet {
public:
  ResultSet() {}
  virtual bool fetchNextValue(T& result) = 0;
private:
  ResultSet(const ResultSet& rhs);
  ResultSet& operator=(const ResultSet& rhs);
};

template <class RECORD, class KEY>
class QueryStatement : public Statement {
public:
  virtual ~QueryStatement() {}
  virtual void setQueryKey(const KEY& key) { _searchKey = key; }
  virtual ResultSet<RECORD>& getResultSet() = 0;
  ResultSet<RECORD>& executeQuery() {
    int st = execute();
    return getResultSet();
  }
protected:
  KEY _searchKey;
};

template <class RECORD>
class QueryByPrimaryKey : public QueryStatement<RECORD,typename RECORD::PrimaryKey>,
                          public DBCloseEvent_Iface {
public:
  QueryByPrimaryKey(GenericBDB<RECORD, RECORD::PrimaryKey>& db) : _db(db), _exactResultSet(new ExactResultSet()) {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "QueryByPrimaryKey statement was created to the address=%p", this);
  }
  ~QueryByPrimaryKey() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "QueryByPrimaryKey statement was destroyed to the address=%p", this);
    delete _exactResultSet;
  }

  virtual int execute() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "QueryByPrimaryKey::execute::: execute query for key=[%s]", _searchKey.toString().c_str());
    uint8_t unmarshalBuffer[RECORD::MAX_RECORD_SIZE];
    size_t realValueSize;

    std::vector<uint8_t> key_rawbuf;
    _searchKey.marshal(key_rawbuf);

    if ( _dbShutdownChecker.checkShutdowInProgressAndSetFlg(DbShutdownChecker::OPERATION_IS_ACTIVE) )
      throw smsc::util::CustomException("QueryByPrimaryKey::execute::: db shutdown in progress");

    int res = find_at_most_one_record_by_key(_db.getDbHandle(),
                                             &key_rawbuf[0], key_rawbuf.size(),
                                             &unmarshalBuffer[0], RECORD::MAX_RECORD_SIZE, &realValueSize);

    if ( _dbShutdownChecker.checkShutdowInProgressAndSetFlg(DbShutdownChecker::OPERATION_IS_INACTIVE) )
      throw smsc::util::CustomException("QueryByPrimaryKey::execute::: db shutdown in progress");

    if ( res < 0 )
      throw smsc::util::CustomException("QueryByPrimaryKey::execute::: Error occured: %s", get_error_message());
    if ( res ) {
      RECORD foundValue;
      foundValue.unmarshal(unmarshalBuffer, realValueSize);
      delete _exactResultSet;
      _exactResultSet = new ExactResultSet(foundValue);
    }
    return res;
  }
  virtual ResultSet<T>& getResultSet() {
    return *_exactResultSet;
  }

  virtual void notify() {
    _dbShutdownChecker.setShutdownInProgress();
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "### QueryByPrimaryKey::notify::: QueryByPrimaryKey statement was notified to the address=%p", this);
  }
private:
  GenericBDB<RECORD, RECORD::PrimaryKey>& _db;
  DbShutdownChecker _dbShutdownChecker;

  class ExactResultSet : public ResultSet<T> {
  public:
    ExactResultSet() : _noMoreValues(true) {}
    ExactResultSet(T& foundValue) : _noMoreValues(false), _foundValue(foundValue) {}
    virtual bool fetchNextValue(T& result) {
      if ( !_noMoreValues ) {
        _noMoreValues = true;
        result = _foundValue;
        return true;
      } else
        return false;
    }
  private:
    bool _noMoreValues;
    T _foundValue;
  };

  ExactResultSet* _exactResultSet;
};

template <class RECORD, class KEY>
class QueryByNonuniqKey : public QueryStatement<RECORD, KEY>,
                          public DBCloseEvent_Iface {
public:
  QueryByNonuniqKey(typename GenericBDB<RECORD, typename RECORD::PrimaryKey>::SecondaryIndex<KEY>& db_idx)
    : _db_idx(db_idx), _cursorp(NULL), _manyRowsResultSet(*this), _realValueSize(0) {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "QueryByNonuniqKey statement was created to the address=%p", this);
    _db_idx.registerOpenedStatement(this);
  }
  ~QueryByNonuniqKey() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "QueryByNonuniqKey statement was destroyed to the address=%p", this);
    close();
  }

  virtual int execute() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "QueryByNonuniqKey::execute::: execute query for key=[%s]", _searchKey.toString().c_str());
    _searchKey.marshal(_key_rawbuf);
    
    if ( _dbShutdownChecker.checkShutdowInProgressAndSetFlg(DbShutdownChecker::OPERATION_IS_ACTIVE) )
      throw smsc::util::CustomException("QueryByNonuniqKey::execute::: db shutdown in progress");

    int res = fetch_first_record_by_key(_db_idx.getDbHandle(),
                                        &_cursorp,
                                        &_key_rawbuf[0], _key_rawbuf.size(),
                                        _unmarshalBuffer, sizeof(_unmarshalBuffer), &_realValueSize);

    if ( _dbShutdownChecker.checkShutdowInProgressAndSetFlg(DbShutdownChecker::OPERATION_IS_INACTIVE) )
      throw smsc::util::CustomException("QueryByNonuniqKey::execute::: db shutdown in progress");

    if ( res < 0 )
      throw smsc::util::CustomException("QueryByNonuniqKey::execute::: Error occured: %s", get_error_message());

    if ( res == 1 )
      _manyRowsResultSet.setResultIsNotEmpty();

    return res;
  }

  virtual ResultSet<RECORD>& getResultSet() {
    return _manyRowsResultSet;
  }

  typedef KEY KeyType;
  typedef RECORD RecordType;
  //protected:
  DBC* positionCursorToFirstRecord() {
    std::vector<uint8_t> key_rawbuf;
    _searchKey.marshal(key_rawbuf);
    DBC *cursorp;
    if ( _dbShutdownChecker.checkShutdowInProgressAndSetFlg(DbShutdownChecker::OPERATION_IS_ACTIVE) )
      throw smsc::util::CustomException("QueryByNonuniqKey::positionCursorToFirstRecord::: db shutdown in progress");
    int res = position_cursor_to_first_record(_db_idx.getDbHandle(),
                                              &cursorp,
                                              &key_rawbuf[0], key_rawbuf.size());
    if ( _dbShutdownChecker.checkShutdowInProgressAndSetFlg(DbShutdownChecker::OPERATION_IS_INACTIVE) )
      throw smsc::util::CustomException("QueryByNonuniqKey::positionCursorToFirstRecord::: db shutdown in progress");
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "QueryByNonuniqKey::positionCursorToFirstRecord::: try find first record for key=[%s], res=%d", _searchKey.toString().c_str(), res);
    if ( res == 1 ) return cursorp;
    else return NULL;
  }

  virtual void notify() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "### QueryByNonuniqKey::notify::: QueryByNonuniqKey statement was notified to the address=%p", this);
    _dbShutdownChecker.setShutdownInProgress();
    close();
  }
private:
  typename GenericBDB<RECORD, typename RECORD::PrimaryKey>::SecondaryIndex<KEY>& _db_idx;
  DBC *_cursorp;
  std::vector<uint8_t> _key_rawbuf;
  uint8_t _unmarshalBuffer[RECORD::MAX_RECORD_SIZE];
  size_t _realValueSize;
  DbShutdownChecker _dbShutdownChecker;

  void close() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    _db_idx.unregisterOpenedStatement(this);
    smsc_log_debug(logger, "QueryByNonuniqKey statement was closed to the address=%p", this);
    if ( _cursorp ) {
      _cursorp->c_close(_cursorp);
      _cursorp = NULL;
    }
  }

  class ManyRowsResultSet : public ResultSet<RECORD> {
  public:
    ManyRowsResultSet(QueryByNonuniqKey<RECORD,KEY>& query)
      : _resultIsNotEmpty(false), _query(query) {}

    virtual bool fetchNextValue(RECORD& result) {
      if ( _resultIsNotEmpty ) {
        result.unmarshal(_query._unmarshalBuffer, _query._realValueSize);
        if ( _query._dbShutdownChecker.checkShutdowInProgressAndSetFlg(DbShutdownChecker::OPERATION_IS_ACTIVE) )
          throw smsc::util::CustomException("ManyRowsResultSet::fetchNextValue::: db shutdown in progress");

        int res =  fetch_next_record_by_key(_query._db_idx.getDbHandle(),
                                            &_query._cursorp,
                                            &_query._key_rawbuf[0], _query._key_rawbuf.size(),
                                            _query._unmarshalBuffer,
                                            sizeof(QueryByNonuniqKey<RECORD,KEY>::_unmarshalBuffer),
                                            &_query._realValueSize);

        if ( _query._dbShutdownChecker.checkShutdowInProgressAndSetFlg(DbShutdownChecker::OPERATION_IS_INACTIVE) )
          throw smsc::util::CustomException("ManyRowsResultSet::fetchNextValue::: db shutdown in progress");
        if ( res < 0 )
          throw smsc::util::CustomException("QueryByNonuniqKey::ManyRowsResultSet::execute::: Error occured: %s", get_error_message());
        if ( res == 0 )
          _resultIsNotEmpty = false;
        return true;
      } else return false;
    }

    void setResultIsNotEmpty() { _resultIsNotEmpty = true; }
  private:
    bool _resultIsNotEmpty;
    QueryByNonuniqKey<RECORD,KEY>& _query;
  };

  ManyRowsResultSet _manyRowsResultSet;
  friend class ManyRowsResultSet;
};

template <class RECORD>
class Base_Join : public DBCloseEvent_Iface {
public:
  Base_Join(GenericBDB<typename RECORD, typename RECORD::PrimaryKey>& db)
    : _resutSet(this), _db(db), _join_curs(NULL) {
    _db.registerOpenedStatement(this);
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "Base_Join statement was created to the address=%p", this);
  }
  virtual ~Base_Join() {
    _db.unregisterOpenedStatement(this);
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "Base_Join statement was destroyed to the address=%p", this);
    close();
  }

  void close() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "Base_Join::close::: try bypass _positionedCursors");
    for (std::vector<DBC*>::iterator iter= _positionedCursors.begin(), end_iter=_positionedCursors.end();
         iter!=end_iter && *iter!=NULL; ++iter) {
      DBC* cur = *iter;
      smsc_log_debug(logger, "Base_Join::close::: try close cursor=%p",cur);
      cur->c_close(cur);
    }
    smsc_log_debug(logger, "Base_Join::close::: try close join_curs=%p",_join_curs);
    _join_curs->c_close(_join_curs); 
    smsc_log_debug(logger, "Base_Join::close::: return from close");
  }

  virtual int execute() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    DB* dbp = _db.getDbHandle();

    smsc_log_debug(logger, "Base_Join::execute::: bypass _joinableQueries");
    for(joinable_queries_t::iterator iter=_joinableQueries.begin(), end_iter=_joinableQueries.end();
        iter != end_iter; ++iter) {
      smsc_log_debug(logger, "Base_Join::execute::: call (*iter)->getBerkeleyCursorPtr()");
      DBC* join_cursor = (*iter)->getBerkeleyCursorPtr();
      if ( !join_cursor )
        return 0;
      smsc_log_debug(logger, "Base_Join::execute::: getBerkeleyCursorPtr returned join_cursor = %p", join_cursor);
      _positionedCursors.push_back(join_cursor);
    }
    _positionedCursors.push_back(NULL); // terminate array of cursors by NULL pointer. It's required by berkeley API.
    if (dbp->join(dbp, &_positionedCursors[0], &_join_curs, 0) != 0)
      throw smsc::util::CustomException("Base_Join::execute::: Error occured: %s", get_error_message());

    _resutSet.setResultIsNotEmpty();
    return 1;
  }

  virtual ResultSet<RECORD>& getResultSet() {
    return _resutSet;
  }

  virtual void notify() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "### QueryByNonuniqKey::: Base_Join was notified to the address=%p", this);
    _dbShutdownChecker.setShutdownInProgress();
    close();
  }
protected:
  typedef std::vector<MixinClass*> joinable_queries_t;
  joinable_queries_t _joinableQueries;

  void registerJoinableQuery(MixinClass* query) {
    _joinableQueries.push_back(query);
  }

private:
  class JoinResultSet : public ResultSet<RECORD> {
  public:
    JoinResultSet(Base_Join<RECORD>* join_query)
      : _resultIsNotEmpty(false), _join_query(join_query) {}
    virtual bool fetchNextValue(RECORD& result) {
      smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
      smsc_log_debug(logger, "JoinResultSet:: _resultIsNotEmpty=%d", _resultIsNotEmpty);
      if ( _resultIsNotEmpty ) {
        uint8_t keyUnmarshalBuffer[RECORD::MAX_RECORD_SIZE], valueUnmarshalBuffer[RECORD::MAX_RECORD_SIZE];
        size_t realKeySize, realValueSize;

        int res = iterate_over_join(_join_query->_join_curs,
                                    keyUnmarshalBuffer, sizeof(keyUnmarshalBuffer), &realKeySize,
                                    valueUnmarshalBuffer, sizeof(valueUnmarshalBuffer), &realValueSize);
        smsc_log_debug(logger, "JoinResultSet:: iterate_over_join returned %d", res);
        result.unmarshal(valueUnmarshalBuffer, realValueSize);
        if ( res < 0 )
          throw smsc::util::CustomException("JoinResultSet::fetchNextValue::: Error occured: %s", get_error_message());
        if ( res == 0 )
          _resultIsNotEmpty = false;
        return _resultIsNotEmpty;
      } else return false;
    }
    void setResultIsNotEmpty() { _resultIsNotEmpty = true; }
  private:
    bool _resultIsNotEmpty;
    Base_Join<RECORD>* _join_query;
  };

  JoinResultSet _resutSet;

  GenericBDB<typename RECORD, typename RECORD::PrimaryKey>& _db;
  DBC* _join_curs;

  std::vector<DBC*> _positionedCursors;
  DbShutdownChecker _dbShutdownChecker;
};

class MixinClass
{
public:
  virtual DBC* getBerkeleyCursorPtr() = 0;
};

template <class QUERY_STATEMENT>
class Join : virtual public Base_Join<QUERY_STATEMENT::RecordType>,
             public MixinClass {
public:
  Join(GenericBDB<typename QUERY_STATEMENT::RecordType, typename QUERY_STATEMENT::RecordType::PrimaryKey>& db,
       typename GenericBDB<typename QUERY_STATEMENT::RecordType, typename QUERY_STATEMENT::RecordType::PrimaryKey>::SecondaryIndex<typename QUERY_STATEMENT::KeyType>& db_idx)
    : Base_Join<typename QUERY_STATEMENT::RecordType>(db), _query(db_idx), _db(db) {
    _db.registerOpenedStatement(&_query);
    registerJoinableQuery(this);
  }
  virtual ~Join() {
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
    smsc_log_debug(logger, "Join:: try unregister &_query=%p", &_query);
    _db.unregisterOpenedStatement(&_query);
    smsc_log_debug(logger, "Join:: destroy object=%p, &_query=%p", this, &_query);
  }

  void setQueryKey(const typename QUERY_STATEMENT::KeyType& key) { _query.setQueryKey(key); }
  virtual DBC* getBerkeleyCursorPtr() {
    return _query.positionCursorToFirstRecord();
  }

private:
  QUERY_STATEMENT _query;
  GenericBDB<typename QUERY_STATEMENT::RecordType, typename QUERY_STATEMENT::RecordType::PrimaryKey>& _db;
};

}}}
#endif
