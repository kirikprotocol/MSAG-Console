#ifndef __SCAG_UTIL_STORAGE_DISKHASHINDEXSTORAGE_H__
#define __SCAG_UTIL_STORAGE_DISKHASHINDEXSTORAGE_H__

#include <string>

#include "core/buffers/DiskHash.hpp"
#include "core/buffers/File.hpp"
#include "logger/Logger.h"

namespace scag { namespace util { namespace storage {

using smsc::core::buffers::DiskHash;
using smsc::core::buffers::File;
using std::string;
using smsc::logger::Logger;

template < class Key, class Idx >
class DiskHashIndexStorage {

public:
  typedef Key key_type;
  typedef Idx index_type;

public:
  DiskHashIndexStorage(const string& dbName, const string& dbPath, uint32_t initRecCnt) {
    logger_ = Logger::getInstance("dhindex");
    string fn = dbPath + "/" + dbName + ".idx";
    if (!File::Exists(fn.c_str())) {
      index_.Create(fn.c_str(), initRecCnt, false);
    } else {
      index_.Open(fn.c_str());
    }
  }

  ~DiskHashIndexStorage() {
  }

  //TODO: init iterator_type
  //iterator_type begin() const {
  //}

  unsigned long size() const {
    return index_.Count();
  }

  index_type getIndex(const key_type& key) const {
      OffsetValue off;
      return index_.LookUp(key, off) ? off.value : invalid_;
  }

  index_type removeIndex(const key_type& key) {
      OffsetValue off;
    if (!index_.LookUp(key, off)) {
      smsc_log_info(logger_, "Attempt to delete record that doesn't exists:%s", key.toString().c_str());
      return invalid_;
    }
    index_.Delete(key);
    smsc_log_debug(logger_, "delRecord:%s:%08llx", key.toString().c_str(), off.value);
    return off.value;
  }

  bool setIndex(const key_type& key, index_type index) {
    OffsetValue off(index);
    index_.Insert(key, off, true);
    return true;
  } 

  void setInvalidIndex(index_type i) {
    invalid_ = i;
  }

private:

    // NOTE: we dont need another template Idx here
    struct OffsetValue {

        // OffsetValue() : value(0){}
        OffsetValue( index_type argValue = 0 ) : value(argValue) {}
        // default ctor is fine
        // OffsetValue(const OffsetValue& src) : value(src.value) {}

        static uint32_t Size() {
            return sizeof(index_type);
        }

        void Read(File& f) {
            value = index_type(f.ReadNetInt64());
        }

        void Write(File& f) const {
            f.WriteNetInt64(int64_t(value));
        }

        index_type value;
    };

private:
    typedef DiskHash < Key, OffsetValue > IndexStorage;

private:
  mutable IndexStorage index_;
  Logger* logger_;
  index_type invalid_;
};

}//storage
}//util
}//scag

#endif
