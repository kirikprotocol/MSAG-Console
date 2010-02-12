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

    typedef DiskHash < Key, OffsetValue > IndexStorage;

public:
  DiskHashIndexStorage(const string& dbName,
                       const string& dbPath,
                       uint32_t initRecCnt,
                       smsc::logger::Logger* thelog = 0 ) :
    logger_(thelog) {
    string fn = dbPath + "/" + dbName + ".idx";
    if (!File::Exists(fn.c_str())) {
      index_.Create(fn.c_str(), initRecCnt, false);
    } else {
      index_.Open(fn.c_str());
    }
  }

  ~DiskHashIndexStorage() {
  }

  unsigned long size() const {
    return index_.Count();
  }

    unsigned long filledSize() const {
        return index_.Count();
    }

    void recalcSize() {
        index_.recalcCount();
    }

  void setInvalidIndex(index_type i) {
    invalid_ = i;
      /*
      // recalculate good nodes
      goodNodesCount_ = 0;
      for ( Iterator iter(begin()); iter.next(); ) {
          if ( iter.idx() != invalid_ ) ++goodNodesCount_;
      }
       */
  }


    index_type invalidIndex() const { return invalid_; }


  index_type getIndex(const key_type& key) const {
      OffsetValue off;
      const index_type i = index_.LookUp(key, off) ? off.value : invalid_;
      smsc_log_debug(logger_,"getIndex key=%s index=%llx",
                     key.toString().c_str(),
                     static_cast<unsigned long long>(i));
      return i;
  }

  bool setIndex(const key_type& key, index_type index) {
    OffsetValue off(index);
      smsc_log_debug(logger_,"setIndex key=%s index=%llx",
                     key.toString().c_str(),
                     static_cast<unsigned long long>(index));
    index_.Insert(key, off, true);
    return true;
  } 

  index_type removeIndex(const key_type& key) {
      OffsetValue off;
    if (!index_.LookUp(key, off)) {
      if (logger_) {
          smsc_log_info(logger_, "Attempt to delete record that doesn't exists:%s", key.toString().c_str());
      }
      return invalid_;
    }
    index_.Delete(key);
    if (logger_) {
        smsc_log_debug(logger_, "delRecord:%s:%08llx", key.toString().c_str(), off.value);
    }
    return off.value;
  }

    class Iterator
    {
    public:
        void reset() {
            if (s_) {
                iter_ = typename IndexStorage::Iterator(s_->index_);
                key_ = Key();
                idx_ = OffsetValue(s_->invalid_);
            }
        }
        bool next() {
            return (s_ ? iter_.Next(key_,idx_) : false);
        }
        Iterator() : s_(NULL) {}
        Iterator( const Iterator& s ) : s_(s.s_), iter_(s.iter_) {
            const_cast<Iterator&>(s).s_ = NULL;
        }
        Iterator& operator = ( const Iterator& s ) {
            if ( &s != this ) {
                Iterator& ss = const_cast< Iterator& >( s );
                s_ = ss.s_;
                iter_ = ss.iter_;
                ss.s_ = NULL;
            }
            return *this;
        }

        const Key& key() const {
            return key_;
        }

        const index_type idx() const {
            return idx_.value;
        }

    private:
        friend class DiskHashIndexStorage<Key,Idx>;

        Iterator( DiskHashIndexStorage<Key,Idx>* s ) :
        s_(s) { reset(); }

    private:
        DiskHashIndexStorage<Key,Idx>*  s_;
        typename IndexStorage::Iterator iter_;
        Key                             key_;
        OffsetValue                     idx_;
    };

    typedef Iterator iterator_type;
    friend struct Iterator;
    Iterator begin() {
        return Iterator(this);
    }


private:
  mutable IndexStorage index_;
  Logger* logger_;
  index_type invalid_;
};

}//storage
}//util
}//scag

#endif
