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
    typedef typename IndexNode::nodeptr_type      nodeptr_type;

public:
    typedef StoredKey storedkey_type;
    typedef Key       key_type;
    typedef Idx       index_type;

    RBTreeIndexStorage( const std::string& dbname,
                        const std::string& dbpath,
                        int indexGrowth = 1000000,
                        bool cleanup = false,
                        smsc::logger::Logger* thelog = 0,
                        bool fullRecovery = false ) :
    index_(0,0,thelog),
    cacheaddr_(0),
    cache_(0),
    invalid_(0),
    goodNodesCount_(0)
    {
        allocator_.reset( new IndexAllocator(thelog,fullRecovery) );
        if ( allocator_->Init( dbpath + '/' + dbname + "-index",
                               indexGrowth,
                               cleanup) < 0 ) {
            throw smsc::util::Exception( "cannot initialize RBTreeAllocator" );
        }

        index_.SetAllocator( allocator_.get() );
        index_.SetChangesObserver( allocator_.get() );
        invalidateCache();
    }

    ~RBTreeIndexStorage() {
        flush();
    }


    unsigned checkTree()
    {
        return index_.checkTree();
    }


    /// flush index allocator in case of fullRecovery
    void flush( unsigned maxSpeedkBPerSec = 0 )
    {
        allocator_->flush( maxSpeedkBPerSec );
    }


    /// number of items
    unsigned long size() const {
        return allocator_->getSize() - 1; // for nilnode
    }

    /// number of good items (filled with non-invalid value)
    unsigned long filledSize() const {
        return goodNodesCount_;
    }


    /// set invalid index
    void setInvalidIndex( index_type i ) {
        invalid_ = i;
        invalidateCache();
        // recalculate the number of good nodes count
        goodNodesCount_ = 0;
        for ( Iterator iter(begin()); iter.next(); ) {
            if ( iter.idx() != invalid_ ) ++goodNodesCount_;
        }
    }


    index_type invalidIndex() const {
        return invalid_;
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
            index_.setNodeValue( cacheaddr_, i );
            if ( i == invalid_ ) --goodNodesCount_;
        } else {
            // the tree may be changed by reallocation
            invalidateCache();
            index_.Insert( k, i );
            if ( i != invalid_ ) ++goodNodesCount_;
        }
        return true;
    }

    /// remove index from the storage.
    /// @return the value of removed index.
    index_type removeIndex( const key_type& k ) {
        IndexNode* node = getNode( k );
        if ( node ) {
            index_type i = node->value;
            index_.setNodeValue( cacheaddr_, invalid_ );
            --goodNodesCount_;
            return i;
        }
        return invalid_;
    }


    class Iterator {
    public:
        void reset() {
            if (s_) {
                s_->index_.Reset();
                key_ = StoredKey();
                idx_ = s_->invalid_;
            }
        }
        bool next() {
            // key_type& k, index_type& i ) {
            return (s_ ? s_->index_.Next( key_, idx_ ) : false);
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

        const StoredKey& key() const {
            return key_;
        }

        const index_type idx() const {
            return idx_;
        }

    private:
        friend class RBTreeIndexStorage<Key,Idx,StoredKey>;

        Iterator( RBTreeIndexStorage<Key,Idx,StoredKey>* s ) :
        s_(s) { reset(); }

    private:
        StoredKey                    key_;
        index_type                   idx_;
        RBTreeIndexStorage<Key,Idx,StoredKey>* s_;
    };
    typedef Iterator iterator_type;
    friend struct Iterator;

    Iterator begin() {
        return Iterator( const_cast< RBTreeIndexStorage<Key,Idx,StoredKey>* >( this ) );
    }

private:
    // NOTE: the side effect is filling the cache in case an item is found in index
    // So, if result is not NULL, you may rely on cache_ and cacheaddr_.
    inline IndexNode* getNode( const key_type& k ) const
    {
        if ( cache_ && cache_->key == k ) {
            return cache_;
        } else if ( negativekey_ == k ) {
            return 0;
        } else {
            nodeptr_type node = const_cast< IndexStorage& >(index_).Get( k );
            IndexNode* res;
            if ( node != allocator_->getNilNode() ) {
                cacheaddr_ = node;
                cache_ = res = allocator_->realAddr(node);
            } else {
                negativekey_ = k;
                res = 0;
            }
            return res;
        }
    }

    inline void invalidateCache() const {
        cache_ = 0;
        cacheaddr_ = allocator_->getNilNode();
        key_type k;
        negativekey_ = k;
    }

private:
    IndexStorage                    index_;
    std::auto_ptr< IndexAllocator > allocator_;
    mutable nodeptr_type            cacheaddr_;   // for successive get/set
    mutable IndexNode*              cache_;       // for successive get/set
    mutable key_type                negativekey_; // for absent key
    index_type                      invalid_;
    size_t                          goodNodesCount_; // those with non-invalid index
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_RBTREEINDEXSTORAGE_H */
