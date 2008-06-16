#ifndef __DBENTITYSTORAGE_STORAGEINDEX_HPP__
# define __DBENTITYSTORAGE_STORAGEINDEX_HPP__ 1

# include <map>
# include <utility>

template <typename K, typename V>
class UniqueStorageIndex {
public:
  bool isExist(const K& key) const;
  bool insertIndexedValue(const K& key, const V& value);

  bool findIndexedValueByKey(const K& key, V* value);

  bool eraseIndexedValue(const K& key);
private:
  //typedef std::map<K,V,std::less<K>,reclaim_mem_alloc<std::pair<const K,V> > >  index_t;
  typedef std::map<K,V>  index_t;
  index_t _index;
};

template <typename K, typename V>
bool
UniqueStorageIndex<K,V>::isExist(const K& key) const
{
  if ( _index.find(key) != _index.end() )
    return true;
  else
    return false;
}

template <typename K, typename V>
bool
UniqueStorageIndex<K,V>::insertIndexedValue(const K& key, const V& value)
{
  if ( !isExist(key) ) {
    _index.insert(std::make_pair(key, value));
    return true;
  } else
    return false;
}

template <typename K, typename V>
bool
UniqueStorageIndex<K,V>::findIndexedValueByKey(const K& key, V* value)
{
  typename index_t::iterator iter = _index.find(key);
  if ( iter != _index.end() ) {
    *value = iter->second;
    return true;
  } else
    return false;
}

template <typename K, typename V>
bool
UniqueStorageIndex<K,V>::eraseIndexedValue(const K& key)
{
  _index.erase(key);
  return true;
}

#include <vector>
#include <list>

template <typename K, typename V>
class NonUniqueStorageIndex {
private:
  typedef typename std::multimap<K, V> index_t;
public:
  class IndexPosition {
  private:
    void savePosition(const typename index_t::iterator& iter) { _searchIter=iter; }
    typename index_t::iterator restorePosition() { return _searchIter; }

    typename index_t::iterator _searchIter;
    friend class NonUniqueStorageIndex<K,V>;
  };

  NonUniqueStorageIndex() : _findBeingExecuted(false) {}
  bool isExist(const K& key) const;
  bool insertIndexedValue(const K& key, const V& value);

  bool findFirstIndexedValueByKey(const K& key, V* value);
  bool findFirstIndexedValueByKey(const K& key, K* foundKey, V* value);
  bool findNextIndexedValueByKey(const K& key, V* value);
  bool findNextIndexedValueByKey(const K& key, K* foundKey, V* value);

  bool findFirstIndexedValueByKey(const K& key, K* foundKey, V* value, IndexPosition& indexPos);
  bool findNextIndexedValueByKey(const K& key, K* foundKey, V* value, IndexPosition& indexPos);

  bool eraseIndexedValue(const K& key, const V& value);
  void eraseIndexedIntervalValue(const K& firstKey, const K& lastKey, std::vector<V>* removedValues);

private:
  index_t _index;
  typename index_t::iterator _searchIter;
  bool _findBeingExecuted;

  typedef std::list<IndexPosition*> registredIters_t;
  registredIters_t _registredIters;

  void registerSearchIterator(IndexPosition* activeIterator)
  {
    _registredIters.push_back(activeIterator);
  }

  void unregisterSearchIterator(IndexPosition* activeIterator)
  {
    typename registredIters_t::iterator i=_registredIters.begin();
    typename registredIters_t::iterator the_end = _registredIters.end();
    while( i != the_end ) {
      //    for(typename registredIters_t::iterator i=_registredIters.begin();
      //i != _registredIters.end(); ++i) {
      if ( *i == activeIterator ) {
        _registredIters.erase(i); break;
      }
      ++i;
    }
  }

  void notifySearchIterators(typename index_t::iterator& indexIter)
  {
    typename registredIters_t::iterator i=_registredIters.begin();
    typename registredIters_t::iterator the_end=_registredIters.end();
    while( i != the_end ) {
      //    for(typename registredIters_t::iterator i=_registredIters.begin();
      //        i != _registredIters.end(); ++i) {
      typename index_t::iterator savedIter = (*i)->restorePosition();
      if ( savedIter == indexIter ) {
        ++savedIter;
        (*i)->savePosition(savedIter);
      }
      ++i;
    }
  }
};

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::isExist(const K& key) const
{
  if ( _index.find(key) != _index.end() )
    return true;
  else
    return false;
}

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::insertIndexedValue(const K& key, const V& value)
{
  _index.insert(std::make_pair(key, value));
  return true;
}

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::findFirstIndexedValueByKey(const K& key, V* value)
{
  K foundKey(key);
  return findFirstIndexedValueByKey(key, &foundKey, value);
}

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::findFirstIndexedValueByKey(const K& key, K* foundKey, V* value)
{
  _searchIter = _index.lower_bound(key);

  if ( _searchIter != _index.end() ) {
    *value = _searchIter->second;
    *foundKey = _searchIter->first;
    ++_searchIter; _findBeingExecuted = true;
    return true;
  } else
    return false;
}

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::findFirstIndexedValueByKey(const K& key, K* foundKey, V* value, IndexPosition& indexPos)
{
  typename index_t::iterator searchIter = _index.lower_bound(key);

  if ( searchIter != _index.end() ) {
    *value = searchIter->second;
    *foundKey = searchIter->first;
    ++searchIter; indexPos.savePosition(searchIter);
    registerSearchIterator(&indexPos);
    return true;
  } else
    return false;
}

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::findNextIndexedValueByKey(const K& key, V* value)
{
  K foundKey(key);
  return findNextIndexedValueByKey(key, &foundKey, value);
}

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::findNextIndexedValueByKey(const K& key, K* foundKey, V* value)
{
  if ( _searchIter != _index.upper_bound(key) ) {
    *value = _searchIter->second;
    *foundKey = _searchIter->first;
    ++_searchIter;
    return true;
  } else {
    _findBeingExecuted = false;
    return false;
  }
}

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::findNextIndexedValueByKey(const K& key, K* foundKey, V* value, IndexPosition& indexPos)
{
  typename index_t::iterator searchIter(indexPos.restorePosition());

  if ( searchIter != _index.end() &&
       searchIter != _index.upper_bound(key) ) {
    *value = searchIter->second;
    *foundKey = searchIter->first;
    ++searchIter; indexPos.savePosition(searchIter);
    return true;
  } else {
    unregisterSearchIterator(&indexPos);
    return false;
  }
}

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::eraseIndexedValue(const K& key, const V& value)
{
  typename index_t::iterator indexIter = _index.lower_bound(key);

  if ( indexIter == _index.end() )
    return false;
  typename index_t::iterator upperBoundIndexIter = _index.upper_bound(key);
  while ( indexIter !=  upperBoundIndexIter ) {
    if ( indexIter->second == value ) {
      if ( _findBeingExecuted && indexIter == _searchIter ) ++_searchIter;
      notifySearchIterators(indexIter);
      _index.erase(indexIter++);
    } else {
      indexIter++;
    }
  }
  return true;
}

#include <list>

template <typename K, typename V>
void
NonUniqueStorageIndex<K,V>::eraseIndexedIntervalValue(const K& firstKey, const K& lastKey, std::vector<V>* removedValues)
{
  typename index_t::iterator iter = _index.lower_bound(firstKey);
  typename index_t::iterator theEndIter = _index.upper_bound(lastKey);
  std::list<typename index_t::iterator> iterList;
  while (iter != theEndIter) {
    iterList.push_back(iter);
    ++iter;
  }
  //  for(std::list<typename index_t::iterator>::iterator i=iterList.begin();
  //      i!=iterList.end();i++) {
  {
    typename std::list<typename index_t::iterator>::iterator i=iterList.begin();
    typename std::list<typename index_t::iterator>::iterator theEndIter=iterList.end();
    while ( i != theEndIter ) {
      removedValues->push_back((*i)->second);
      _index.erase(*i);
      ++i;
    }
  }
}
#endif
