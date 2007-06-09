#ifndef __SMSC_DB_BDB_MMS_RECORD_HPP__
# define __SMSC_DB_BDB_MMS_RECORD_HPP__ 1

# include <sys/types.h>
# include <string>
# include <vector>

# include <db.h>
# include <db/bdb_framework/StorableDbRecord_Iface.hpp>

namespace mmbox {
namespace app_specific_db {

class MmsDbPrimaryKey : public smsc::db::bdb_framework::RecordKey {
public:
  MmsDbPrimaryKey(const std::string& mmsId="");
  virtual void marshal(std::vector<uint8_t>& buf) const;
  virtual void unmarshal(const std::vector<uint8_t>& buf);
  virtual void unmarshal(const uint8_t* buf, size_t bufSz);
  virtual const std::string toString() const;
  virtual size_t getKeySizeInStorage() const;

  static const std::string getKeyName() { return std::string("PrimaryKey"); }

  const std::string& getMmsId() const;
private:
  std::string _mmsId;
  enum { MIN_KEY_SIZE = 2 /*1 + 1*/ };
};

class MmsDb_AbonentAddr_NonUniqKey : public smsc::db::bdb_framework::RecordKey {
public:
  MmsDb_AbonentAddr_NonUniqKey(const std::string& abonentAddress="");
  virtual void marshal(std::vector<uint8_t>& buf) const;
  virtual void unmarshal(const uint8_t* buf, size_t bufSz);
  virtual void unmarshal(const std::vector<uint8_t>& buf);

  static int keyValueExtractor(DB *secondary, /* secondary db handle */
                               const DBT *primary_key, /* primary db record's key */
                               const DBT *record_data, /* primary db record's data */
                               DBT *secondary_key); /* secondary db record's key */
  virtual const std::string toString() const;
  virtual size_t getKeySizeInStorage() const;

  static const std::string getKeyName() { return std::string("AbonentAddr_NonUniqKey"); }

  const std::string& getAbonentAddress() const;
private:
  std::string _abonentAddress;
  enum { MIN_KEY_SIZE = 2 /*1 + 1*/ };
};

class MmsDb_Status_NonUniqKey : public smsc::db::bdb_framework::RecordKey {
public:
  typedef uint32_t mms_status_t;

  MmsDb_Status_NonUniqKey(mms_status_t mmsStatus=0);
  virtual void marshal(std::vector<uint8_t>& buf) const;
  virtual void unmarshal(const uint8_t* buf, size_t bufSz);
  virtual void unmarshal(const std::vector<uint8_t>& buf);

  static int keyValueExtractor(DB *secondary, /* secondary db handle */
                               const DBT *primary_key, /* primary db record's key */
                               const DBT *record_data, /* primary db record's data */
                               DBT *secondary_key); /* secondary db record's key */
  virtual const std::string toString() const;
  virtual size_t getKeySizeInStorage() const;

  static const std::string getKeyName() { return std::string("Status_NonUniqKey"); }

  mms_status_t getMmsStatus() const;
private:
  mms_status_t _mmsStatus;
  enum { MIN_KEY_SIZE = sizeof(mms_status_t) };
};

class MmsDbRecord : public smsc::db::bdb_framework::StorableDbRecord_Iface {
public:
  MmsDbRecord();
  MmsDbRecord(const std::string& mmsId, const std::string& abonentAddress, MmsDb_Status_NonUniqKey::mms_status_t mmsStatus, uint32_t sentTime);
  virtual void marshal(std::vector<uint8_t>& buf) const;
  virtual void unmarshal(const std::vector<uint8_t>& buf);
  virtual void unmarshal(const uint8_t* buf, size_t bufSz);
  virtual const MmsDbPrimaryKey& getPrimaryKey() const;

  const std::string& getMmsId() const;
  const std::string& getAbonentAddress() const;
  MmsDb_Status_NonUniqKey::mms_status_t getMmsStatus() const;
  uint32_t getSentTime() const;

  std::string toString() const;

  static const int MAX_RECORD_SIZE = 65535;
  typedef MmsDbPrimaryKey PrimaryKey;
private:
  MmsDbPrimaryKey              _primaryKey;
  MmsDb_AbonentAddr_NonUniqKey _abonentAddrNonUniqKey;
  MmsDb_Status_NonUniqKey      _mmsStatusNonUniqKey;
  uint32_t                     _sentTime;
};

}}

#endif
