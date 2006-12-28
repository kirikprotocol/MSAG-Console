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
  std::map<K,V> _index;
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
  std::map<K,V>::iterator iter = _index.find(key);
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

template <typename K, typename V>
class NonUniqueStorageIndex {
public:
  NonUniqueStorageIndex() : _findBeingExecuted(false) {}
  bool isExist(const K& key) const;
  bool insertIndexedValue(const K& key, const V& value);

  bool findFirstIndexedValueByKey(const K& key, V* value);
  bool findNextIndexedValueByKey(const K& key, V* value);

  bool eraseIndexedValue(const K& key, const V& value);
  void eraseIndexedIntervalValue(const K& firstKey, const K& lastKey, std::vector<V>* removedValues);
private:
  std::multimap<K,V> _index;
  typename std::multimap<K,V>::iterator _searchIter;
  bool _findBeingExecuted;
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
  _searchIter = _index.lower_bound(key);

  if ( _searchIter != _index.end() ) {
    *value = _searchIter->second;
    ++_searchIter; _findBeingExecuted = true;
    return true;
  } else
    return false;
}

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::findNextIndexedValueByKey(const K& key, V* value)
{
  if ( _searchIter != _index.upper_bound(key) ) {
    *value = _searchIter->second;
    ++_searchIter;
    return true;
  } else {
    _findBeingExecuted = false;
    return false;
  }
}

template <typename K, typename V>
bool
NonUniqueStorageIndex<K,V>::eraseIndexedValue(const K& key, const V& value)
{
  typename std::multimap<K,V>::iterator
    indexIter = _index.lower_bound(key);
  if ( indexIter == _index.end() )
    return false;

  while ( indexIter != _index.upper_bound(key) ) {
    if ( indexIter->second == value ) {
      if ( _findBeingExecuted && indexIter == _searchIter ) ++_searchIter;
      _index.erase(indexIter++);
    } else
      indexIter++;
  }
  return true;
}

#include <list>

template <typename K, typename V>
void
NonUniqueStorageIndex<K,V>::eraseIndexedIntervalValue(const K& firstKey, const K& lastKey, std::vector<V>* removedValues)
{
  typename std::multimap<K,V>::iterator iter = _index.lower_bound(firstKey);

  std::list<std::multimap<K,V>::iterator> iterList;
  while (iter != _index.upper_bound(lastKey)) {
    iterList.push_back(iter);
    ++iter;
  }
  for(std::list<std::multimap<K,V>::iterator>::iterator i=iterList.begin();
      i!=iterList.end();i++) {
    removedValues->push_back((*i)->second);
    _index.erase(*i);
  }
}
#endif
