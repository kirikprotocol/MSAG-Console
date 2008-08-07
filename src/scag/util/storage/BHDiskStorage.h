#ifndef _SCAG_UTIL_STORAGE_BHDISKSTORAGE_H
#define _SCAG_UTIL_STORAGE_BHDISKSTORAGE_H

#include "BlocksHSStorage.h"

namespace scag {
namespace util {
namespace storage {

/**
 * BlocksHSStorage wrapper
 */
template < class Key, class Val, class Storage = BlocksHSStorage< Key, Val > >
class BHDiskStorage
{
public:
    static const bool                         updatable = false;
    typedef Storage                           storage_type;
    typedef Key                               key_type;
    typedef typename storage_type::index_type index_type;
    typedef DataBlockBackup< Val >            value_type;

    BHDiskStorage( storage_type* hs ) : store_(hs), v_(NULL)
    {
        i_ = invalidIndex();
        if ( !hs ) {
            throw std::runtime_error("BlocksHSStorage should be provided" );
        }
    }


    ~BHDiskStorage() { delete store_; }


    inline bool setKey( const key_type& k ) const 
    {
        key_ = k;
        return true;
    }


    inline index_type invalidIndex() const {
        return -1;
    }


    void serialize( const value_type& v )
    {
        // no serialization is done here, we just store ptr to v
        i_ = invalidIndex();
        v_ = const_cast<value_type*>(&v);
    }


    index_type append() 
    {
        index_type blockIndex;
        if ( !v_ || !store_->Add( *v_, key_, blockIndex ) ) return 0;
        return blockIndex;
    }
    

    bool read( index_type i ) const
    {
        i_ = i;
        v_ = NULL;
        return true;
    }


    bool deserialize( value_type& v ) const
    {
        if ( i_ == invalidIndex() || !store_->Get(i_, v) ) return false;
        return true;
    }


    index_type update( index_type i )
    {
        if ( !v_ || !store_->Change( *v_, key_, i ) ) return invalidIndex();
        return i;
    }


    void remove( index_type i )
    {
        typename value_type::value_type vv;
        typename value_type::backup_type bb;
        value_type v(&vv,&bb);
        if ( store_->Get(i,v) ) store_->Remove(key_,i,v);
    }

private:
    storage_type*       store_;
    mutable key_type    key_;
    mutable index_type  i_;
    mutable value_type* v_; // not owned
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_BHDISKSTORAGE_H */
