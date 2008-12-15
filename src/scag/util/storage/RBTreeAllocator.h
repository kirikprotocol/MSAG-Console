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

namespace scag {
namespace util {
namespace storage {

const char RED = 1;
const char BLACK = 0;

// transient representation of the node
template < class Key, class Value > struct templRBTreeNode
{
    typedef long nodeptr_type;
    nodeptr_type parent;         // also is used as a next_free_cell
    nodeptr_type left;
    nodeptr_type right;
    int32_t      color;
    Key		 key;
    Value	 value;
};


template< class Key=long, class Value=long > class RBTreeAllocator
{
protected:
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

template<class Key=long, class Value=long>
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

template< class Key=long, class Value=long >
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

template<class Key=long, class Value=long>
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
