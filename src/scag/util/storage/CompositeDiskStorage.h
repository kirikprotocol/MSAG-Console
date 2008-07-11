#ifndef _SCAG_UTIL_STORAGE_COMPOSITEDISKSTORAGE_H
#define _SCAG_UTIL_STORAGE_COMPOSITEDISKSTORAGE_H

#include <vector>
#include <memory>

#include "StorageNumbering.h"
#include "util/Exception.hpp"

namespace scag {
namespace util {
namespace storage {

/// this template class combine several homogeneous disk storages into one.
/// the additional requirement on the key type is:
///   integral toIndex() const
template < class DStorage >
class CompositeDiskStorage
{
public:
    typedef typename DStorage::key_type   key_type;
    typedef typename DStorage::value_type value_type;


    class Iterator {
    protected:
        Iterator( const CompositeDiskStorage< DStorage >& s ) : s_(&s), idx_(0) {}
        friend class CompositeDiskStorage< DStorage >;
    public:
        Iterator() : s_(0), idx_(0) {}
        void reset() {
            idx_ = 0;
            iter_ = typename DStorage::iterator_type();
        }
        bool next( key_type& k, value_type& v ) {
            if ( iter_.next(k,v) ) return true;
            if ( !s_ ) return false;
            for ( ; idx_ < s_->storages_.size(); ++idx_ ) {
                if ( ! s_->storages_[idx_] ) continue;
                iter_ = s_->storages_[idx_]->begin();
                if ( iter_.next(k,v) ) {
                    ++idx_;
                    return true;
                }
            }
            return false;
        }
    private:
        const CompositeDiskStorage< DStorage >*  s_;
        typename DStorage::iterator_type         iter_;
        unsigned                                 idx_;
    };
    friend class Iterator;
    typedef Iterator iterator_type;


    CompositeDiskStorage()
    {
        const StorageNumbering& n = StorageNumbering::instance();
        storages_.resize( n.storages(), NULL );
    }


    // store gets owned
    void addStorage( unsigned idx, DStorage* store )
    {
        std::auto_ptr< DStorage > x(store);
        if ( idx >= storages_.size() ) {
            throw smsc::util::Exception( "CompositeDiskStorage: idx=%u is too big", idx );
        } else if ( storages_[idx] ) {
            throw smsc::util::Exception( "CompositeDiskStorage: storage replacement is not allowed (idx=%u)", idx );
        }
        storages_[idx] = x.release();
    }


    ~CompositeDiskStorage()
    {
        for ( typename std::vector< DStorage* >::iterator i = storages_.begin();
              i != storages_.end();
              ++i ) {
            delete *i;
            *i = NULL;
        }
    }


    unsigned long size() const {
        unsigned long s = 0;
        for ( typename std::vector< DStorage* >::const_iterator i = storages_.begin();
              i != storages_.end();
              ++i ) {
            if ( *i ) s += (*i)->size();
        }
        return s;
    }


    bool set( const key_type& k, const value_type& v ) {
        return storage(k)->set(k,v);
    }
        

    bool get( const key_type& k, value_type& v ) const {
        return storage(k)->get(k,v);
    }


    bool has( const key_type& k ) const {
        return storage(k)->has(k);
    }
    

    bool remove( const key_type& k ) {
        return storage(k)->remove(k);
    }
    

    iterator_type begin() const {
        return iterator_type(*this);
    }

private:
    inline DStorage* storage( const key_type& k ) const {
        const unsigned n = StorageNumbering::instance().storage( k.toIndex() );
        DStorage* ret = storages_[n];
        if ( !ret ) {
            throw smsc::util::Exception
                ( "CompositeDiskStorage: storage not found key=%s number=%llu idx=%u", k.toString().c_str(), k.toIndex(), n );
        }
        return ret;
    }

private:
    std::vector< DStorage* >  storages_;  // owned
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* !_SCAG_UTIL_STORAGE_COMPOSITEDISKSTORAGE_H */
