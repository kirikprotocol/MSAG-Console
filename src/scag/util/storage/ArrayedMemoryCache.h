#ifndef _SCAG_UTIL_STORAGE_ARRAYEDMEMORYCACHE_H
#define _SCAG_UTIL_STORAGE_ARRAYEDMEMORYCACHE_H

// This storage meets the requirements on the
// template class parameter MemStorage of CachedDiskStorage
// interface.

#include <stdexcept>
#include "logger/Logger.h"
#include "core/buffers/IntHash.hpp"
#include "MemoryCacheTypeJuggling.h"

namespace scag {
namespace util {
namespace storage {

template < class Key, class Val,
        template <class> class TypeJuggling = MemoryCacheSimpleTypeJuggling,
        class HF = Key >
class ArrayedMemoryCache : public TypeJuggling< Val >
{
public:
    typedef Key key_type;
    typedef typename TypeJuggling< Val >::value_type  value_type;
    typedef typename TypeJuggling< Val >::stored_type stored_type;

private:
  struct CacheItem {
    CacheItem(const key_type& k, stored_type v) { key = k; vv = v; };
    CacheItem() {};
    key_type key;
    stored_type vv;
  };

private:
   typedef smsc::core::buffers::IntHash<CacheItem>  hash_type;

public:
    ArrayedMemoryCache( smsc::logger::Logger* thelog = 0,
                       unsigned int cachesize = 10000 ) :
    hash_(cachesize),
    cachelog_(thelog),
    cachesize_(cachesize) {
    }

    ~ArrayedMemoryCache() {
        clean();
    }

    bool set( const key_type& k, stored_type v ) {
        if (cachelog_) smsc_log_debug( cachelog_, "set: %s", k.toString().c_str() );
        uint32_t index = getIndex(k);
        CacheItem* item = hash_.GetPtr(index);
        if (!item) {
          hash_.Insert(index, CacheItem(k, v));
          return false;
        }
        if ( ! (item->key == k)) {
          if (cachelog_) smsc_log_debug( cachelog_, "set: replace %s by %s", item->key.toString().c_str(), k.toString().c_str() );
          //if (store2val(item->vv)) dealloc(item->vv);
          dealloc(item->vv);
          item->vv = v;
          item->key = k;
        } else if (store2val(item->vv) && store2val(v)) {
          dealloc(v);
          // FIXME: should we replace with warning?
          throw std::runtime_error
              ( "Itemlist: two items with the same keys found\n"
                "It may mean that you issue set() w/o prior get()" );
        }
        return true;
    }


    unsigned size() const {
        return unsigned(hash_.Count());
    }


    stored_type* get( const key_type& k ) const {
        CacheItem* item = getCacheItem(k);
        if (item && item->key == k && store2val(item->vv)) {
          if (cachelog_) smsc_log_debug( cachelog_, "get: %s hit", k.toString().c_str());
          return &(item->vv);
        } else {
          if (cachelog_) smsc_log_debug( cachelog_, "get: %s miss", k.toString().c_str());
          return 0;
        }
    }


    value_type* release( const key_type& k ) {
        if (cachelog_) smsc_log_debug( cachelog_, "clr: %s", k.toString().c_str() );
        uint32_t index = getIndex(k);
        CacheItem* item = hash_.GetPtr(index);
        if ( !item || !(item->key == k)) {
          return store2val(this->val2store(NULL));
        }
        stored_type v = item->vv;
        hash_.Delete( index );
        value_type* vvv = store2val(v);
        releaseval(v); // release value_type
        dealloc(v);    // and free all other resources
        return vvv;
    }


    void clean() {
        Iterator it(hash_);
        while ( it.next() ) {
            dealloc(it.value());
        }
        hash_.Empty();
    }

private: 

  uint32_t getIndex( const key_type& k ) const {
      return HF::CalcHash(k) % cachesize_;
  }

  CacheItem* getCacheItem(  const key_type& k ) const {
    uint32_t index = getIndex(k);
    return const_cast<hash_type&>(hash_).GetPtr(index);
  }

public:
    class Iterator 
    {
    protected:
        Iterator( const hash_type& h ) : iter_(h), key_(0) {}

    public:
        ~Iterator() {}
        bool next() {
            return iter_.Next( key_, val_ );
        }
        const key_type& key() const {
            return val_.key;
        }
        stored_type& value() {
            return val_.vv;
        }
        friend class ArrayedMemoryCache<Key,Val,TypeJuggling,HF>;
    private:
        typename hash_type::Iterator iter_;
        int                          key_;
        CacheItem                    val_;
    };
    typedef Iterator iterator_type;

    iterator_type begin() const {
        return Iterator(hash_);
    }

private:
    hash_type             hash_;
    smsc::logger::Logger* cachelog_;
    uint32_t              cachesize_;
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_HASHEDMEMORYCACHE_H */
