//------------------------------------
//  RBTreeAllocator.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	Файл содержит описание класса RBTreeAllocator.
//

#ifndef _SCAG_UTIL_STORAGE_RBTREEALLOCATOR_H
#define _SCAG_UTIL_STORAGE_RBTREEALLOCATOR_H

#include <vector>
#include "logger/Logger.h"

namespace scag {
namespace util {
namespace storage {

const char RED = 1;
const char BLACK = 0;

// transient representation of the node
template < class Key, typename Value > struct templRBTreeNode
{
    typedef unsigned nodeptr_type;
    nodeptr_type parent;         // also is used as a next_free_cell
    nodeptr_type left;
    nodeptr_type right;
    int8_t       color;
    Key		 key;
    Value	 value;
};


template< class Key, typename Value=long > class RBTreeAllocator
{
public:
    typedef templRBTreeNode<Key, Value>       RBTreeNode;
    typedef typename RBTreeNode::nodeptr_type nodeptr_type;

public:
    RBTreeAllocator() {}
    virtual ~RBTreeAllocator(){}
    virtual nodeptr_type allocateNode(void) = 0;
    virtual void releaseNode(nodeptr_type node) = 0;
    virtual nodeptr_type getNilNode(void) = 0;
    virtual nodeptr_type getRootNode(void) = 0;
    virtual void setRootNode(nodeptr_type node) = 0;

    // the number of nodes
    virtual long getSize(void) const = 0;

    // obtain the real node
    virtual RBTreeNode* realAddr( nodeptr_type addr ) const = 0;
    // virtual long relativeAddr( RBTreeNode* node ) const = 0;

    // a temporary method (for debugging)
    // return the list of the free nodes and check their integrity:
    // address correctness, uniqueness, freecell count, etc.
    // NOTE: it may be quite resources consuming.
    virtual std::vector< nodeptr_type > freenodes() = 0;

    //virtual void resetChanges(void) = 0;
    //virtual void nodeChanged(RBTreeNode* node) = 0;
    //virtual void completeChanges(void) = 0;
};

template< class Key, typename Value=long>
class RBTreeChangesObserver
{
protected:
    typedef templRBTreeNode<Key, Value>       RBTreeNode;
    typedef typename RBTreeNode::nodeptr_type nodeptr_type;
public:
    static const int OPER_INSERT = 1;
    static const int OPER_DELETE = 2;
    static const int OPER_CHANGE = 3;

    virtual void startChanges( nodeptr_type node, int operation ) = 0;
    virtual void nodeChanged( nodeptr_type node ) = 0;
    virtual void completeChanges(void) = 0;
};


// a special class to check the rbtree
// NOTE: it needs an allocator (realAddr)
template < class Key, typename Value = long >
    class RBTreeChecker
{
public:
    typedef RBTreeAllocator< Key, Value >      Allocator;
    typedef typename Allocator::RBTreeNode     RBTreeNode;
    typedef typename RBTreeNode::nodeptr_type  nodeptr_type;

public:
    /// main ctor
    /// \param a -- an allocator of rbtree
    /// \param totalcells -- a number of cells in rbtree (upper limit on cell index)
    /// \param logger -- logger to debug nodes
    /// \param maxlogdepth -- limit on logging by depth (only if logger != 0)
    RBTreeChecker( Allocator& a,
                   nodeptr_type totalcells,
                   smsc::logger::Logger* logger = 0,
                   unsigned maxlogdepth = 6 ) :
    a_(a),
    totalcells_(totalcells),
    maxusedcell_(0),
    usedcells_(1), // for a nilnode
    failedmsg_(0),
    failednode_(0),
    faileddepth_(0),
    failedpath_(0),
    log_(logger),
    maxlogdepth_(maxlogdepth) {
        pathbuf[64] = '\0';
    }

    /// check the whole tree
    /// \return false if the tree is corrupted.  In that case the details could be
    ///  retrieved via failed() and failedNode() methods.
    inline bool check( nodeptr_type rootnode, nodeptr_type nilnode ) {
        return subcheck( rootnode, nilnode, 0, 0, 0 );
    }

    /// return the reason of failure after check() returned false
    inline const char* failedMsg() const {
        return failedmsg_;
    }

    /// return the node where the failure occurs after check() returned false
    inline nodeptr_type failedNode() const {
        return failednode_;
    }

    inline const char* failedPath() {
        return makepath(faileddepth_,failedpath_);
    }

    inline unsigned failedDepth() const {
        return faileddepth_;
    }

    inline nodeptr_type maxUsedCell() const {
        return maxusedcell_;
    }

    inline unsigned usedCells() const {
        return usedcells_;
    }

protected:
    /// a recursive method, checking the tree starting from given node.
    /// \param node -- current node
    /// \param parent -- should be the parent of the node
    /// \param depth -- current depth of the node.
    /// \param path -- the binary path to a given node:
    ///    - 'depth' lower bits are used
    ///    - higher bit is more significant 
    ///    - 0 means 'go to left', 1 means 'go to right'
    bool subcheck( nodeptr_type node,
                   nodeptr_type parent,
                   RBTreeNode* parentcell,
                   unsigned depth, 
                   uint64_t path )
    {
        do {
            const nodeptr_type nilnode = a_.getNilNode();
            if ( node == nilnode ) break;
            if ( totalcells_ > 0 && node >= totalcells_ ) {
                setfail( node, depth, path, "too great cell index" );
                break;
            }
            if ( node > maxusedcell_ ) maxusedcell_ = node;
            if ( depth > 1000 ) {
                setfail( node, depth, path, "too deep, cyclic links?" );
                break;
            }
            RBTreeNode* cell = a_.realAddr(node);
            ++usedcells_;

            if ( log_ && depth < maxlogdepth_ ) {
                // show the node
                smsc_log_debug( log_, "node=(%ld:%s) value=%lld c=%c parent=%ld left=%ld right=%ld depth=%d path=%s",
                                (long)node,
                                cell->key.toString().c_str(),
                                (long long)cell->value,
                                (cell->color == RED ) ? 'R' : 'B',
                                cell->parent,
                                cell->left,
                                cell->right,
                                depth,
                                makepath(depth,path) );
            }

            if ( cell->parent != parent ) {
                setfail( node, depth, path, "parent link is broken" );
                break;
            }

            if ( parent != nilnode && parentcell ) {
                if ( parentcell->left == node ) {
                    if ( ! (cell->key < parentcell->key) ) {
                        setfail( node, depth, path, "key misorder node=parent->left" );
                        break;
                    }
                } else {
                    if ( ! (parentcell->key < cell->key) ) {
                        setfail( node, depth, path, "key misorder node=parent->right" );
                        break;
                    }
                }
            }

            ++depth;
            path *= 2;
            if ( cell->left != nilnode ) {
                if ( ! subcheck(cell->left,node,cell,depth,path) ) break;
            }
            if ( cell->right != nilnode ) {
                if ( ! subcheck(cell->right,node,cell,depth,path+1) ) break;
            }
        } while ( false );
        return ( failedmsg_ == 0 );
    }


    void setfail( nodeptr_type node, unsigned depth, uint64_t path, const char* msg )
    {
        failednode_ = node;
        faileddepth_ = depth;
        failedpath_ = path;
        failedmsg_ = msg;
    }

    const char* makepath( unsigned depth, uint64_t path )
    {
        if ( depth > 64 ) return "too deep";
        char* p = &(pathbuf[64]);
        while ( depth-- > 0 ) {
            unsigned pathbit = path & 0x1;
            *(--p) = pathbit ? 'r' : 'l';
            path >>= 1;
        }
        return p;
    }
    
private:
    Allocator&            a_;
    nodeptr_type          totalcells_;
    nodeptr_type          maxusedcell_;
    unsigned              usedcells_;
    const char*           failedmsg_;
    nodeptr_type          failednode_;
    unsigned              faileddepth_;
    uint64_t              failedpath_;
    char                  pathbuf[65];
    smsc::logger::Logger* log_;
    unsigned              maxlogdepth_;
};




template< class Key, typename Value=long >
    class SimpleAllocator: public RBTreeAllocator<Key, Value>
{
    typedef typename RBTreeAllocator<Key,Value>::RBTreeNode RBTreeNode;
    typedef typename RBTreeNode::nodeptr_type               nodeptr_type;

public:
    SimpleAllocator() : count(0)
    {
        //create nil cell
        RBTreeNode* nn = new RBTreeNode();
        nilNode = (nodeptr_type) nn;
        nn->parent = nilNode;
        nn->left = nilNode;
        nn->right = nilNode;
        nn->color = BLACK;
        rootNode = nilNode;
    }

    virtual ~SimpleAllocator()
    {
        delete (RBTreeNode*) nilNode;
    }

    virtual nodeptr_type allocateNode(void)
    {
        RBTreeNode* newNode = new RBTreeNode();
        newNode->parent = newNode->left = newNode->right = nilNode;
        count++;
        return (nodeptr_type) newNode;
    }

    virtual void releaseNode( nodeptr_type node)
    {
        count--;
        delete (RBTreeNode*) node;
    }

    virtual nodeptr_type getNilNode(void) {return nilNode;}
    virtual nodeptr_type getRootNode(void){return rootNode;}
    virtual void setRootNode( nodeptr_type node ) {rootNode = node;}
    virtual long getSize(void) const {return count;}
    virtual RBTreeNode* realAddr( nodeptr_type addr ) const {
        return (RBTreeNode*)addr;
    }

    virtual std::vector< nodeptr_type > freenodes() {
        return std::vector< nodeptr_type >();
    }

private:
    long count;
    nodeptr_type nilNode;
    nodeptr_type rootNode;
};

template< class Key, typename Value=long>
    class EmptyChangesObserver: public RBTreeChangesObserver<Key, Value>
{
    typedef typename RBTreeChangesObserver<Key,Value>::RBTreeNode RBTreeNode;
    typedef typename RBTreeNode::nodeptr_type                     nodeptr_type;
public:
    void startChanges( nodeptr_type /*node*/, int /*operation*/ ) {}
    void nodeChanged( nodeptr_type /*node*/) {}
    void completeChanges(void) {}
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* ! _SCAG_UTIL_STORAGE_RBTREE_ALLOCATOR_H */
