#include "MmsRecord.hpp"

#include <algorithm>
#include <iterator>
#include <string.h>
#include <sstream>

#include <util/Exception.hpp>
#include <util/BinDump.hpp>
#include <logger/Logger.h>

namespace mmbox {
namespace app_specific_db {

MmsDbPrimaryKey::MmsDbPrimaryKey(const std::string& mmsId) : _mmsId(mmsId) {}

void
MmsDbPrimaryKey::marshal(std::vector<uint8_t>& buf) const
{
  std::copy(_mmsId.c_str(), _mmsId.c_str() + _mmsId.size() + 1,
            std::back_inserter(buf));
}

void
MmsDbPrimaryKey::unmarshal(const std::vector<uint8_t>& buf)
{
  unmarshal(&buf[0], buf.size());
}

void
MmsDbPrimaryKey::unmarshal(const uint8_t* buf, size_t bufSz)
{
  if ( bufSz < MIN_KEY_SIZE )
    throw smsc::util::CustomException("MmsDbPrimaryKey::unmarshal::: buffer size is less than MIN_KEY_SIZE=%d", MIN_KEY_SIZE);
  _mmsId.assign((char*)buf);
}

const std::string
MmsDbPrimaryKey::toString() const
{
  std::ostringstream obuf;
  obuf << "mmsId=[" << getMmsId()
       << "]";
  return obuf.str();
}

size_t
MmsDbPrimaryKey::getKeySizeInStorage() const
{
  return getMmsId().size() + 1;
}

const
std::string& MmsDbPrimaryKey::getMmsId() const
{
  return _mmsId;
}

MmsDb_AbonentAddr_NonUniqKey::MmsDb_AbonentAddr_NonUniqKey(const std::string& abonentAddress)
  : _abonentAddress(abonentAddress) {}

const std::string&
MmsDb_AbonentAddr_NonUniqKey::getAbonentAddress() const { return _abonentAddress; }

void
MmsDb_AbonentAddr_NonUniqKey::marshal(std::vector<uint8_t>& buf) const {
  std::copy(_abonentAddress.c_str(), _abonentAddress.c_str() + _abonentAddress.size() + 1,
            std::back_inserter(buf));
}

void
MmsDb_AbonentAddr_NonUniqKey::unmarshal(const uint8_t* buf, size_t bufSz) {
  if ( bufSz < MIN_KEY_SIZE )
    throw smsc::util::CustomException("MmsDb_AbonentAddr_NonUniqKey::unmarshal::: buffer size is less than MIN_KEY_SIZE=%d", MIN_KEY_SIZE);
  _abonentAddress.assign((char*)buf);
}

void
MmsDb_AbonentAddr_NonUniqKey::unmarshal(const std::vector<uint8_t>& buf) {
  unmarshal(&buf[0], buf.size());
}

int
MmsDb_AbonentAddr_NonUniqKey::keyValueExtractor(DB *secondary, /* secondary db handle */
                                                const DBT *primary_key, /* primary db record's key */
                                                const DBT *record_data, /* primary db record's data */
                                                DBT *secondary_key) /* secondary db record's key */
{         
  /* First, extract the structure contained in the primary's data */
  MmsDbRecord record;
  record.unmarshal((uint8_t*)record_data->data, record_data->size);

  std::string abonentAddress (record.getAbonentAddress());

  memset(secondary_key, 0, sizeof(DBT));
  secondary_key->data = strdup(abonentAddress.c_str()); // MEMORY LEAK!!! Fix it in the future!!!!
  secondary_key->size = abonentAddress.size() + 1;

  /* Return 0 to indicate that the record can be created/updated. */
  return 0;
}

const std::string
MmsDb_AbonentAddr_NonUniqKey::toString() const
{
  std::ostringstream obuf;
  obuf << "abonentAddress=[" << getAbonentAddress()
       << "]";
  return obuf.str();
}

size_t
MmsDb_AbonentAddr_NonUniqKey::getKeySizeInStorage() const
{
  return getAbonentAddress().size() + 1;
}

// --------
MmsDb_Status_NonUniqKey::MmsDb_Status_NonUniqKey(mms_status_t mmsStatus)
  : _mmsStatus(mmsStatus) {}

MmsDb_Status_NonUniqKey::mms_status_t
MmsDb_Status_NonUniqKey::getMmsStatus() const { return _mmsStatus; }

void
MmsDb_Status_NonUniqKey::marshal(std::vector<uint8_t>& buf) const {
  std::copy(reinterpret_cast<const uint8_t*>(&_mmsStatus), reinterpret_cast<const uint8_t*>(&_mmsStatus) + sizeof(_mmsStatus),
            std::back_inserter(buf));
}

void
MmsDb_Status_NonUniqKey::unmarshal(const uint8_t* buf, size_t bufSz) {
  if ( bufSz < MIN_KEY_SIZE )
    throw smsc::util::CustomException("MmsDb_Status_NonUniqKey::unmarshal::: buffer size is less than MIN_KEY_SIZE=%d", MIN_KEY_SIZE);
  memcpy(reinterpret_cast<uint8_t*>(&_mmsStatus), buf, sizeof(_mmsStatus));
}

void
MmsDb_Status_NonUniqKey::unmarshal(const std::vector<uint8_t>& buf) {
  unmarshal(&buf[0], buf.size());
}

int
MmsDb_Status_NonUniqKey::keyValueExtractor(DB *secondary, /* secondary db handle */
                                           const DBT *primary_key, /* primary db record's key */
                                           const DBT *record_data, /* primary db record's data */
                                           DBT *secondary_key) /* secondary db record's key */
{         
  /* First, extract the structure contained in the primary's data */
  MmsDbRecord record;
  record.unmarshal((uint8_t*)record_data->data, record_data->size);

  memset(secondary_key, 0, sizeof(DBT));
  secondary_key->data = ::malloc(sizeof(mms_status_t)); // MEMORY LEAK!!! Fix it in the future!!!!
  *(reinterpret_cast<mms_status_t*>(secondary_key->data)) = record.getMmsStatus();
  secondary_key->size = sizeof(mms_status_t);

  /* Return 0 to indicate that the record can be created/updated. */
  return 0;
}

const std::string
MmsDb_Status_NonUniqKey::toString() const
{
  std::ostringstream obuf;
  obuf << "mmsStatus=[" << getMmsStatus()
       << "]";
  return obuf.str();
}

size_t
MmsDb_Status_NonUniqKey::getKeySizeInStorage() const
{
  return sizeof(_mmsStatus);
}

const int MmsDbRecord::MAX_RECORD_SIZE;

MmsDbRecord::MmsDbRecord() {}

MmsDbRecord::MmsDbRecord(const std::string& mmsId, const std::string& abonentAddress,
                         MmsDb_Status_NonUniqKey::mms_status_t mmsStatus, uint32_t sentTime)
  : _primaryKey(mmsId), _abonentAddrNonUniqKey(abonentAddress),
    _mmsStatusNonUniqKey(mmsStatus),  _sentTime(sentTime) {}

void MmsDbRecord::marshal(std::vector<uint8_t>& buf) const
{
  buf.reserve(_primaryKey.getKeySizeInStorage()+
              _abonentAddrNonUniqKey.getKeySizeInStorage()+
              _mmsStatusNonUniqKey.getKeySizeInStorage()+
              sizeof(_sentTime));
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
  smsc_log_debug(logger,"MmsDbRecord::marshal::: record=[%s]", this->toString().c_str());

  _primaryKey.marshal(buf);
  _abonentAddrNonUniqKey.marshal(buf);
  _mmsStatusNonUniqKey.marshal(buf);
  std::copy((uint8_t*)&_sentTime, (uint8_t*)&_sentTime + sizeof(_sentTime),
            std::back_inserter(buf));
  smsc_log_debug(logger,"MmsDbRecord::marshal::: result buffer=[%s]", smsc::util::DumpHex(buf.size(), &buf[0]).c_str());
}

void MmsDbRecord::unmarshal(const std::vector<uint8_t>& buf)
{
  unmarshal(&buf[0], buf.size());
}

void MmsDbRecord::unmarshal(const uint8_t* buf, size_t bufSz)
{
  size_t bufPos=0;

  _primaryKey.unmarshal(buf,bufSz);

  bufSz -= _primaryKey.getKeySizeInStorage();
  bufPos = _primaryKey.getKeySizeInStorage();
  _abonentAddrNonUniqKey.unmarshal(buf+bufPos, bufSz);

  bufSz  -= _abonentAddrNonUniqKey.getKeySizeInStorage();
  bufPos += _abonentAddrNonUniqKey.getKeySizeInStorage();
  _mmsStatusNonUniqKey.unmarshal(buf+bufPos, bufSz);

  bufSz  -= _mmsStatusNonUniqKey.getKeySizeInStorage();
  bufPos += _mmsStatusNonUniqKey.getKeySizeInStorage();
  if ( bufSz < sizeof(_sentTime) )
    throw smsc::util::CustomException("MmsDbRecord::unmarshal::: can't unmarshal sentTine - wrong buffer size=%d ", bufSz);
  memcpy(reinterpret_cast<uint8_t*>(&_sentTime), buf+bufPos, bufSz);
}

#include <sstream>

std::string MmsDbRecord::toString() const
{
  std::ostringstream obuf;
  obuf << "mmsId=[" << getMmsId()
       << "],abonentAddress=[" << getAbonentAddress()
       << "],mmsStatus=[" << getMmsStatus()
       << "],sentTime=[" << getSentTime()
       << "]";
  return obuf.str();
}

const MmsDbPrimaryKey& MmsDbRecord::getPrimaryKey() const
{
  return _primaryKey;
}

const std::string&
MmsDbRecord::getMmsId() const { return _primaryKey.getMmsId(); }

const std::string&
MmsDbRecord::getAbonentAddress() const { return _abonentAddrNonUniqKey.getAbonentAddress(); }

MmsDb_Status_NonUniqKey::mms_status_t
MmsDbRecord::getMmsStatus() const { return _mmsStatusNonUniqKey.getMmsStatus(); }

uint32_t
MmsDbRecord::getSentTime() const { return _sentTime; }

}}
