/* ************************************************************************** *
 * IAPQueriesStore: registry of IAPQueries FSM objects.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAP_QUERiIES_STORE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAP_QUERiIES_STORE_HPP

#include "core/buffers/Hash.hpp"
#include "inman/abprov/facility2/inc/IAPQuery.hpp"

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

using smsc::core::synchronization::Mutex;

/* ************************************************************************** *
 * 
 * ************************************************************************** */
class IAPQueriesStore {
protected:
  class CachedQuery {
  protected:
    mutable unsigned  _refs;
    IAPQueryAC *      _query;

    friend class IAPQueriesStore;

    explicit CachedQuery(IAPQueryAC & use_qry)
      : _refs(1), _query(&use_qry)
    { }

    void      addRef(void) const { ++_refs; }
    unsigned  unRef(void) const { if (_refs) --_refs; return _refs; }

  public:
    CachedQuery() : _refs(0), _query(NULL)
    { }
    ~CachedQuery()
    { }

    IAPQueryAC * query(void) const { return _query; }
  };
  //
  typedef smsc::core::buffers::Hash<CachedQuery> QueriesCash; //key is TonNpiAddress.signals[]

public:
  enum RecordStatus_e {
    qryNone = 0   //no query available/may be allocated
    , qryNew      //query was just allocated
    , qryActive   //query already exists
  };

  class RecordGuard {
  private:
    IAPQueriesStore * _qrsStore;
    CachedQuery *     _pRec;
    RecordStatus_e    _status;

  protected:
    friend class IAPQueriesStore;

    RecordGuard(IAPQueriesStore & use_store, const AbonentId & ab_id, bool do_alloc);

  public:
    RecordGuard() : _qrsStore(NULL), _pRec(NULL), _status(qryNone)
    { }
    RecordGuard(const RecordGuard & cp_obj);
    //
    ~RecordGuard()
    { clear(); }

    void clear(void);
    //
    void rlseQuery(void);
    //
    RecordStatus_e getStatus(void) const { return _status; }
    //
    bool isStatus(RecordStatus_e use_st) const { return (_status == use_st); }

    IAPQueryAC * get(void) const { return _pRec ? _pRec->query() : NULL; }

    IAPQueryAC * operator->(void) const { return get(); }

    IAPQueryAC & operator*(void) const { return *get(); }

    RecordGuard & operator= (const RecordGuard & cp_obj);
  };

  class iterator {
  private:
    IAPQueriesStore *     _qrsStore;
    CachedQuery *         _pRec;
    QueriesCash::Iterator _qrsIt;

  protected:
    friend class IAPQueriesStore;

    explicit iterator(IAPQueriesStore & use_store)
      : _qrsStore(&use_store), _pRec(NULL), _qrsIt(&use_store._qrsCache)
    { }

  public:
    iterator(const iterator & cp_obj);
    //
    ~iterator();

    iterator & operator++(); //preincrement

    IAPQueryAC * get(void) const { return _pRec ? _pRec->query() : NULL; }

    IAPQueryAC * operator->(void) const { return get(); }

    bool operator==(const iterator & cp_obj) const
    {
      return (_qrsStore == cp_obj._qrsStore) && (_pRec == cp_obj._pRec);
    }
  };

  IAPQueriesStore() : _qrsPool(NULL)
  { }
  ~IAPQueriesStore()
  { }

  void init(IAPQueriesPoolIface & use_pool, IAPQueryRefereeIface & use_referee)
  {
    _qrsPool = &use_pool; _qrsReferee = &use_referee;
  }
  //
  unsigned size(void) const;
  //
  bool empty(void) const { return size() == 0; }
    
  //Searches query's cache for query associated with given abonent,
  //if no query is found and 'do_alloc' is true, then allocates a new one.
  //Returns:
  //  query record guard, that prevents query from being expunged from
  //  cache until it exists.
  RecordGuard getQuery(const AbonentId & ab_id, bool do_alloc) /*throw()*/
  {
    return RecordGuard(*this, ab_id, do_alloc);
  }
  //
  iterator begin(void)
  {
    return ++iterator(*this);
  }

private:
  IAPQueriesPoolIface *   _qrsPool;
  IAPQueryRefereeIface *  _qrsReferee;

  //Decreases cached query refCounter.
  //Returns true if query is released (record is destroyed).
  bool  _unrefRecord(CachedQuery & qry_rec);

protected:
  mutable Mutex       _sync;
  QueriesCash         _qrsCache;  //active queries

  friend class RecordGuard;
  //Searches query's cache for query associated with given abonent.
  //Increases query's refCounter, so query cann't be expunged from cache.
  //Returns:
  //  NULL in no query is found
  CachedQuery *   _findQuery(const AbonentId & ab_id);
  //Searches query's cache for query associated with given abonent,
  //in no query is found allocates a new one.
  //Increases query's refCounter, so query cann't be expunged from cache.
  //Returns:
  //  RecordStatus_e
  RecordStatus_e  _getQuery(const AbonentId & ab_id, CachedQuery * & p_qry);

  friend class iterator;
  //Increases cached query refCounter.
  void  _refQuery(CachedQuery & qry_rec);
  //Decreases cached query refCounter.
  //Returns true if query is released (record is destroyed).
  bool  _unrefQuery(CachedQuery & qry_rec);
  //
  void  _advance(QueriesCash::Iterator & use_it, CachedQuery * & p_rec);
};

typedef IAPQueriesStore::RecordGuard IAPQueryGuard;

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAP_QUERiIES_STORE_HPP */

