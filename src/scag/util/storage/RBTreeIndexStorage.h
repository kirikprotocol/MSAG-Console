#ifndef _SCAG_UTIL_STORAGE_RBTREEINDEXSTORAGE_H
#define _SCAG_UTIL_STORAGE_RBTREEINDEXSTORAGE_H

#include <memory>
#include <string>

#include "RBTree.h"
#include "RBTreeHSAllocator.h"

namespace scag {
namespace util {
namespace storage {

/// index disk storage
template < class Key, typename Idx, typename StoredKey = Key >
class RBTreeIndexStorage
{
private:
    typedef RBTree<StoredKey,Idx>                 IndexStorage;    // transient
    typedef RBTreeHSAllocator<StoredKey,Idx>      IndexAllocator;
    typedef typename IndexStorage::RBTreeNode     IndexNode;

public:
    typedef Key key_type;
    typedef Idx index_type;

    RBTreeIndexStorage( const std::string& dbname,
                        const std::string& dbpath,
                        int indexGrowth = 1000000,
                        bool cleanup = false,
                        smsc::logger::Logger* thelog = 0 ) :
    index_(0,0,thelog),
    cache_(0),
    invalid_(0)
    {
        allocator_.reset( new IndexAllocator(thelog) );
        if ( allocator_->Init( dbpath + '/' + dbname + "-index",
                               indexGrowth,
                               cleanup) < 0 ) {
            throw smsc::util::Exception( "cannot initialize RBTreeAllocator" );
        }

        index_.SetAllocator( allocator_.get() );
        index_.SetChangesObserver( allocator_.get() );
    }

    ~RBTreeIndexStorage() {
    }


    /// number of items
    unsigned long size() const {
        return allocator_->getSize() - 1; // for nilnode
    }


    /// set invalid index
    void setInvalidIndex( index_type i ) {
        invalid_ = i;
    }


    /// @return the index of the key, or 0 if not found
    index_type getIndex( const key_type& k ) const {
        IndexNode* node = getNode( k );
        return node ? node->value : invalid_;
    }
    
    /// set the index of the key with possible replacement.
    bool setIndex( const key_type& k, index_type i ) {
        IndexNode* node = getNode( k );
        if ( node ) {
            index_.setNodeValue( node, i );
        } else {
            // the tree may be changed by reallocation
            invalidateCache();
            index_.Insert( k, i );
        }
        return true;
    }

    /// remove index from the storage.
    /// @return the value of removed index.
    index_type removeIndex( const key_type& k ) {
        IndexNode* node = getNode( k );
        if ( node ) {
            index_type i = node->value;
            index_.setNodeValue( node, invalid_ );
            return i;
        }
        return invalid_;
    }


    class Iterator {
    public:
        void reset() {
            if (s_) s_->index_.Reset();
        }
        bool next( key_type& k, index_type& i ) {
            return (s_ ? s_->index_.Next( k, i ) : false);
        }
        Iterator() : s_(NULL) {}
        Iterator( const Iterator& s ) : s_(s.s_) {
            const_cast<Iterator&>(s).s_ = NULL;
        }
        Iterator& operator = ( const Iterator& s ) {
            if ( &s != this ) {
                Iterator& ss = const_cast< Iterator& >( s );
                s_ = ss.s_;
                ss.s_ = NULL;
            }
            return *this;
        }

    private:
        friend class RBTreeIndexStorage<Key,Idx>;

        Iterator( RBTreeIndexStorage<Key,Idx>* s ) :
        s_(s) { reset(); }

    private:
        RBTreeIndexStorage<Key,Idx>* s_;
    };
    typedef Iterator iterator_type;
    friend struct Iterator;

    Iterator begin() {
        return Iterator( const_cast< RBTreeIndexStorage<Key,Idx>* >( this ) );
    }

private:
    inline IndexNode* getNode( const key_type& k ) const
    {
        if ( cache_ && cache_->key == k ) {
            return cache_;
        } else if ( negativekey_ == k ) {
            return NULL;
        } else {
            IndexNode* node = const_cast< IndexStorage& >(index_).Get( k );
            if ( node )
                cache_ = node;
            else
                negativekey_ = k;
            return node;
        }
    }

    inline void invalidateCache() const {
        cache_ = NULL;
        key_type k;
        negativekey_ = k;
    }

private:
    IndexStorage                    index_;
    std::auto_ptr< IndexAllocator > allocator_;
    mutable IndexNode*              cache_;       // for successive get/set
    mutable key_type                negativekey_; // for absent key
    index_type                      invalid_;
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_RBTREEINDEXSTORAGE_H */
