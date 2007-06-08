#ifndef __SMSC_DB_BDB_STORABLEDBRECORD_IFACE__
# define __SMSC_DB_BDB_STORABLEDBRECORD_IFACE__ 1

//# include <memory>
# include <vector>
# include <string>

# include <db.h>

# include <stdexcept>
# include <db/bdb_framework/bdb_api_wrapper.h>

namespace smsc {
namespace db {
namespace bdb_framework {

/*
** Class makes bytes array from primary key data
*/
class RecordKey {
public:
  virtual ~RecordKey() {}
  virtual void marshal(std::vector<uint8_t>& buf) const = 0;
  virtual void unmarshal(const std::vector<uint8_t>& buf) = 0;
  virtual void unmarshal(const uint8_t* buf, size_t bufSz) = 0;
  virtual const std::string toString() const=0;
  virtual size_t getKeySizeInStorage() const=0;
};

/*
** Class responsibilities: present abstraction for DB Data Record. Data Record is indetified by primary key.
** Data record can be write to bytes array and can be restored from bytes array.
*/
class StorableDbRecord_Iface {
public:
  virtual ~StorableDbRecord_Iface() {}
  // pack a record to bytes array suitable for storing to DB
  virtual void marshal(std::vector<uint8_t>& buf) const = 0;
  // fill in record fields from bytes array have been read from DB
  virtual void unmarshal(const std::vector<uint8_t>& buf) = 0;
  virtual void unmarshal(const uint8_t* buf, size_t bufSz) = 0;

  // get primary key for record
  virtual const RecordKey& getPrimaryKey() const = 0;

  static const size_t EMPTY_STRING_SIZE = 1;
};

}}}

#endif
