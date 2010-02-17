#ifndef _SCAG_UTIL_STORAGE_INDEXEDSTORAGE2_H
#define _SCAG_UTIL_STORAGE_INDEXEDSTORAGE2_H

#include "util/Exception.hpp"
#include "KeyLogger.h"

namespace scag2 {
namespace util {
namespace storage {

namespace details {

template < class IS, bool UPD = false > struct IndexedStorageUpdater
{
    typedef typename IS::key_type   key_type;
    typedef typename IS::index_type index_type;
    typedef typename IS::buffer_type buffer_type;
    
    IndexedStorageUpdater( IS& s ) : is(s) {}

    inline bool update( const key_type& k,
                        index_type i,
                        buffer_type& b,
                        buffer_type* oldbuf = 0 )
    {
        index_type j = is.data_->append(b);
        if ( j != is.invalidIndex_ ) {
            if (i==j) return true;
            is.index_->setIndex(k,j);
        } else {
            is.index_->removeIndex(k);
        }
        is.data_->remove(i);
        return (j != is.invalidIndex_);
    }

    IS& is;
};


template < class IS > struct IndexedStorageUpdater< IS, true >
{
    typedef typename IS::key_type    key_type;
    typedef typename IS::index_type  index_type;
    typedef typename IS::buffer_type buffer_type;

    IndexedStorageUpdater( IS& s ) : is(s) {}

    inline bool update( const key_type& k, index_type i,
                        buffer_type& b,
                        buffer_type* oldbuf = 0 )
    {
        index_type j = is.data_->update(i,b,oldbuf);
        if ( j != is.invalidIndex_ ) {
            if (j!=i) is.index_->setIndex(k,j);
            return true;
        } else {
            is.index_->removeIndex(k);
            return false;
        }
    }

    IS& is;
};

} // namespace details


template < class IStorage, class DStorage > class IndexedStorage2
{
public:
    typedef IndexedStorage2<IStorage,DStorage> Self;
    typedef typename IStorage::key_type       key_type;
    typedef typename IStorage::index_type     index_type;
    typedef typename DStorage::buffer_type    buffer_type;

private:
    friend class details::IndexedStorageUpdater<Self,DStorage::updatable>;
    typedef details::IndexedStorageUpdater<Self,DStorage::updatable> Updater;

public:
    class Iterator {
        friend class IndexedStorage2<IStorage,DStorage>;
    public:
        void reset() { if (s_) iter_ = s_->index_->begin(); }
        bool next( key_type& k, buffer_type& buf ) {
            if (!s_) return false;
            while (iter_.next()) {
                k = iter_.key();
                index_type i = iter_.idx();
                if ( i != s_->invalidIndex_ ) {
                    s_->keylogger_.setKey(k);
                    if ( s_->data_->read(i,buf) ) { return true; }
                }
            }
            return false;
        }
        Iterator() : s_(0) {}
    private:
        Iterator( const Self& s ) : s_(&s), iter_(s.index_->begin()) {}
    private:
        const    Self*                   s_;
        typename IStorage::iterator_type iter_;
    };
    typedef Iterator         iterator_type;

public:
    IndexedStorage2( IStorage* is, DStorage* ds ) :
    index_(is), data_(ds), updater_(*this) {
        if (!is || !ds ) {
            delete is;
            delete ds;
            throw smsc::util::Exception("IndexedStorage: both storages should be provided");
        }
        invalidIndex_ = data_->invalidIndex();
        data_->setKeyLogger(keylogger_);
        index_->setInvalidIndex(invalidIndex_);
    }

    ~IndexedStorage2() {
        delete index_;
        delete data_;
    }

    inline unsigned long size() const {
        return index_->size();
    }

    inline unsigned long filledSize() const {
        return index_->filledSize();
    }

    bool set( const key_type& k, buffer_type& buf, buffer_type* oldbuf = 0 ) {
        keylogger_.setKey(k);
        index_type i = index_->getIndex(k);
        if (i != invalidIndex_) {
            return updater_.update(k,i,buf,oldbuf);
        } else {
            index_type j = data_->append(buf);
            if (j != invalidIndex_) return index_->setIndex(k,j);
        }
        return false;
    }

    bool get( const key_type& k, buffer_type& buf ) const {
        index_type i = index_->getIndex(k);
        if ( i != invalidIndex_ ) {
            keylogger_.setKey(k);
            if ( data_->read(i,buf) ) return true;
        }
        return false;
    }

    bool has( const key_type& k ) const {
        return (index_->getIndex(k) != invalidIndex_);
    }
    
    bool remove( const key_type& k, buffer_type* oldbuf = 0 ) {
        index_type i = index_->removeIndex(k);
        if ( i != invalidIndex_ ) {
            keylogger_.setKey(k);
            data_->remove(i,oldbuf);
            return true;
        }
        return false;
    }

    inline void packBuffer( buffer_type& buf, buffer_type* hdr = 0 ) {
        data_->packBuffer(buf,hdr);
    }
    inline void unpackBuffer( buffer_type& buf, buffer_type* hdr = 0 ) {
        data_->unpackBuffer(buf,hdr);
    }
    inline size_t headerSize() const { return data_->headerSize(); }

private:
    IStorage*                                       index_;
    DStorage*                                       data_;
    Updater                                         updater_;
    mutable KeyLoggerT<key_type>                    keylogger_;
    index_type                                      invalidIndex_;
};

}
}
}

#endif
