#ifndef __DBENTITYSTORAGE_DBENTITYSTORAGEIFACE_HPP__
# define __DBENTITYSTORAGE_DBENTITYSTORAGEIFACE_HPP__ 1

/*
** It's interface for searching value by key in DB
*/
template <typename K, typename V>
class DbEntity_SearchIface {
public:
  // return true if record was found
  virtual bool findValue(const K& key, V* result) = 0;
};

template <typename K, typename V>
class DbEntity_NonUniqueSearchIface {
public:
  virtual bool findFirstValue(const K& key, V* resultValue) = 0;
  virtual bool findNextValue(const K& key, V* resultValue) = 0;
};

/*
** It's interface for storing value to DB
*/
template <typename V>
class DbEntity_StorageIface {
public:
  // return true if value was stored
  virtual bool putValue(const V& value) = 0;
};

template <typename V> class DbIterator;

/*
** It's inteface for bypassing of all DB records
*/
template <typename V>
class DbEntity_FullTableScanIface {
public:
  /*
  ** Get iterator for data storage sequential bypassing
  */
  virtual DbIterator<V>* getIterator() = 0;
};

template <typename V>
class DbIterator {
public:
  /*
  ** Get next value from DB during sequential bypassing.
  ** Return true if the resultValue contains next DB record value,
  ** else return false.
  */
  virtual bool nextValue(V* resultValue) = 0;
};

/*
** It's interface for deleting element by key value.
*/
template <typename K>
class DbEntity_EraseIface {
public:
  // return -1 if error was occured else return num of records being deleted
  virtual int eraseValue(const K& key) = 0;
};

template <typename PRIMARY_KEY, typename V>
class DbEntity_UpdateIface {
public:
  // return -1 if error was occured or num of rows (0 or 1) being deleted,
  virtual int updateValue(const PRIMARY_KEY& key, const V& oldValue, const V& newValue) = 0;
};

#endif
