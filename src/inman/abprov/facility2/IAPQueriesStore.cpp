#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/facility2/IAPQueriesStore.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
/* ************************************************************************** *
 * class IAPQueriesStore::RecordGuard implementation:
 * ************************************************************************** */
IAPQueriesStore::RecordGuard::RecordGuard(IAPQueriesStore & use_store,
                                          const AbonentId & ab_id, bool do_alloc)
  : _qrsStore(&use_store), _pRec(NULL), _status(IAPQueriesStore::qryNone)
{
  if (do_alloc)
    _status = _qrsStore->_getQuery(ab_id, _pRec);
  else
    _status = (_pRec = _qrsStore->_findQuery(ab_id))
              ? IAPQueriesStore::qryActive : IAPQueriesStore::qryNone;
}

IAPQueriesStore::RecordGuard::RecordGuard(const IAPQueriesStore::RecordGuard & cp_obj)
  : _qrsStore(cp_obj._qrsStore), _pRec(cp_obj._pRec), _status(cp_obj._status)
{
  if (_pRec)
    _qrsStore->_refQuery(*_pRec);
}

void IAPQueriesStore::RecordGuard::clear(void)
{
  if (_pRec) {
    _qrsStore->_unrefQuery(*_pRec);
    _pRec = NULL;
  }
}
//
void IAPQueriesStore::RecordGuard::rlseQuery(void)
{
  if (_pRec && _qrsStore->_unrefQuery(*_pRec))
    _pRec = NULL;
}

IAPQueriesStore::RecordGuard &
  IAPQueriesStore::RecordGuard::operator= (const RecordGuard & cp_obj)
{
  clear();
  _qrsStore = cp_obj._qrsStore;
  _status = cp_obj._status;
  if ((_pRec = cp_obj._pRec))
    _qrsStore->_refQuery(*_pRec);
  return *this;
}

/* ************************************************************************** *
 * class IAPQueriesStore::iterator implementation:
 * ************************************************************************** */
IAPQueriesStore::iterator::iterator(const IAPQueriesStore::iterator & cp_obj)
  : _qrsStore(cp_obj._qrsStore), _pRec(cp_obj._pRec), _qrsIt(cp_obj._qrsIt)
{
  if (_pRec)
    _qrsStore->_refQuery(*_pRec);
}

IAPQueriesStore::iterator::~iterator()
{
  if (_pRec) {
    _qrsStore->_unrefQuery(*_pRec);
    _pRec = NULL;
  }
}

IAPQueriesStore::iterator &
  IAPQueriesStore::iterator::operator++() //preincrement
{
   _qrsStore->_advance(_qrsIt, _pRec);
   return *this;
}

/* ************************************************************************** *
 * class IAPQueriesStore implementation:
 * ************************************************************************** */
//Decreases cached query refCounter.
//Returns true if query is released (record is destroyed).
bool IAPQueriesStore::_unrefRecord(IAPQueriesStore::CachedQuery & qry_rec)
{
  if (!qry_rec.unRef()) {
    IAPQueryAC * pQry = qry_rec.query();

    _qrsCache.Delete(pQry->getAbonentId().getSignals()); //erases qry_rec
    pQry->cleanup();
    _qrsPool->rlseQuery(*pQry);
    return true;
  }
  return false;
}

unsigned IAPQueriesStore::size(void) const
{
  MutexGuard  grd(_sync);
  return _qrsCache.GetCount();
}

//Searches query's cache for query associated with given abonent.
//Increases query's refCounter, so query cann't be expunged from cache.
//Returns:
//  NULL in no query is found
IAPQueriesStore::CachedQuery * IAPQueriesStore::_findQuery(const AbonentId & ab_id)
{
  MutexGuard  grd(_sync);
  CachedQuery * qRec = _qrsCache.GetPtr(ab_id.getSignals());
  if (qRec)
    qRec->addRef();
  return qRec;
}

//Searches query's cache for query associated with given abonent,
//in no query is found allocates a new one.
//Increases query's refCounter, so query cann't be expunged from cache.
//Returns:
//  RecordStatus_e
IAPQueriesStore::RecordStatus_e
  IAPQueriesStore::_getQuery(const AbonentId & ab_id, IAPQueriesStore::CachedQuery * & p_rec)
{
  MutexGuard  grd(_sync);
  RecordStatus_e rval = qryActive;

  if (!(p_rec = _qrsCache.GetPtr(ab_id.getSignals()))) {
    IAPQueryAC * pObj = _qrsPool->allcQuery();
    if (!pObj)
      return qryNone;

    pObj->init(*_qrsReferee, ab_id);
    p_rec = _qrsCache.SetItem(ab_id.getSignals(), CachedQuery(*pObj));
    rval = qryNew;
  }
  p_rec->addRef();
  return rval;
}

//Increases cached query refCounter.
void IAPQueriesStore::_refQuery(IAPQueriesStore::CachedQuery & qry_rec)
{
  MutexGuard  grd(_sync);
  qry_rec.addRef();
}

//Decreases cached query refCounter.
//Returns true if query is released.
bool IAPQueriesStore::_unrefQuery(IAPQueriesStore::CachedQuery & qry_rec)
{
  MutexGuard  grd(_sync);
  return _unrefRecord(qry_rec);
}

void IAPQueriesStore::_advance(IAPQueriesStore::QueriesCash::Iterator & use_it, 
                               IAPQueriesStore::CachedQuery * & p_rec)
{
  MutexGuard  grd(_sync);
  if (p_rec) {
    _unrefRecord(*p_rec);
    p_rec = NULL;
  }
  char * abId = NULL;
  if (use_it.Next(abId, p_rec))
    p_rec->addRef();
}

} //iaprvd
} //inman
} //smsc


