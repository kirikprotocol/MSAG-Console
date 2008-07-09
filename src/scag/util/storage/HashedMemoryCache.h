#ifndef _SCAG_UTIL_STORAGE_HASHEDMEMORYCACHE_H
#define _SCAG_UTIL_STORAGE_HASHEDMEMORYCACHE_H

// This storage meets the requirements on the
// template class parameter MemStorage of CachedDiskStorage
// interface.

#include "logger/Logger.h"
#include "core/buffers/XHash.hpp"

namespace scag {
namespace util {
namespace storage {

template < class Val >
class MemoryCacheSimpleTypeJuggling
{
public:
    typedef Val         value_type;
    typedef value_type* stored_type;
    typedef value_type& ref_type;    

    inline value_type* store2val( stored_type v ) const {
        return v;
    }
    inline stored_type val2store( value_type* v ) const {
        return v;
    }
    // arg should be ref to store but since it is a ptr we use pass by value
    inline ref_type store2ref( stored_type v ) const {
        return *v;
    }
    inline void dealloc( stored_type v ) const {
        delete v;
    }

protected:
    /// release the value_type part only
    inline void releaseval( stored_type& v ) const {
        v = NULL;
    }
};


template < class Key, class Val,
        template <class> class TypeJuggling = MemoryCacheSimpleTypeJuggling,
        class HF = Key >
class HashedMemoryCache : public TypeJuggling< Val >
{
public:
    typedef Key key_type;
    typedef typename TypeJuggling< Val >::value_type  value_type;
    typedef typename TypeJuggling< Val >::stored_type stored_type;

public:
    HashedMemoryCache( unsigned int cachesize = 10000 ) :
    hash_(cachesize),
    cachelog_(NULL) {
        cachelog_ = smsc::logger::Logger::getInstance( "cache" );
    }

    ~HashedMemoryCache() {
        clean();
    }
    
    bool set( const key_type& k, stored_type v ) {
        smsc_log_debug( cachelog_, "set: %s", k.toString().c_str() );
        stored_type* vv = hash_.GetPtr(k);
        if (vv) {
            if (store2val(*vv) && store2val(v)) {
                dealloc( v );
                // FIXME: should we replace with warning?
                throw std::runtime_error
                    ( "Itemlist: two items with the same keys found\n"
                      "It may mean that you issue set() w/o prior get()" );
            }
            dealloc(*vv);
            *vv = v;
            return true;
        }
        hash_.Insert(k,v);
        return false;
    }

    stored_type get( const key_type& k ) const {
        const stored_type* vv = hash_.GetPtr(k);
        stored_type v( vv ? *vv : this->val2store(NULL) );
        smsc_log_debug( cachelog_, "get: %s %s", k.toString().c_str(), store2val(v) ? "hit" : "miss" );
        return v;
    }

    value_type* release( const key_type& k ) {
        smsc_log_debug( cachelog_, "clr: %s", k.toString().c_str() );
        stored_type* vv = hash_.GetPtr(k);
        stored_type v = vv ? *vv : this->val2store(NULL);
        if ( vv ) hash_.Delete( k );
        value_type* vvv = store2val(v);
        releaseval(v); // release value_type
        dealloc(v);    // and free all other resources
        return vvv;
    }


    void clean() {
        hash_.First();
        key_type k;
        stored_type v;
        while ( hash_.Next(k,v) ) {
            dealloc( v );
        }
        hash_.Clean();
    }


private:
    typedef smsc::core::buffers::XHash<key_type,stored_type,HF>  hash_type;

public:
    class Iterator 
    {
    protected:
        Iterator( const hash_type& h ) : iter_(&h) {}
    public:
        ~Iterator() {}
        void reset() { iter_.First(); }
        bool next(key_type& k, stored_type& v) {
            return iter_.Next( k, v );
        }
        friend class HashedMemoryCache<Key,Val,TypeJuggling,HF>;
    private:
        typename hash_type::Iterator iter_;
    };
    typedef Iterator iterator_type;

    iterator_type begin() const {
        return Iterator(hash_);
    }

private:
    hash_type             hash_;
    smsc::logger::Logger* cachelog_;
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_HASHEDMEMORYCACHE_H */
