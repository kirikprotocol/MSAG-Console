//------------------------------------
//  RBTreee.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	���� �������� �������� ������ RBTreee.
//

#include <stdexcept>

#include "RBTreeAllocator.h"
#include "unistd.h"
#include "logger/Logger.h"

#ifndef _SCAG_UTIL_STORAGE_RBTREE_H
#define _SCAG_UTIL_STORAGE_RBTREE_H

namespace scag {
namespace util {
namespace storage {

// transient representation of the node
template<class Key, class Value>
struct templRBTreeNode
{
    templRBTreeNode<Key, Value>* parent; // also is used as a next_free_cell (if the cell is empty)
    templRBTreeNode<Key, Value>* left;
    templRBTreeNode<Key, Value>* right;
    int32_t     color;
    Key		key;
    Value	value;
};


template<class Key=long, class Value=long, class DefaultAllocator = SimpleAllocator<Key, Value>, class DefaultChangesObserver = EmptyChangesObserver<Key, Value> >
class RBTree
{
public:
    typedef templRBTreeNode<Key, Value> RBTreeNode;
    RBTree( RBTreeAllocator<Key, Value>* _allocator=0,
            RBTreeChangesObserver<Key, Value>* _changesObserver=0,
            smsc::logger::Logger* thelog = 0 ) : 
    defaultAllocator(false), defaultChangesObserver(false)
    {
        logger = thelog;
        if(!_allocator)
        {
            allocator = new DefaultAllocator;
            defaultAllocator = true;
        }
        else
            allocator = _allocator;
        rootNode = allocator->getRootNode();
        nilNode = allocator->getNilNode();
        // size = allocator->getSize();
		
        if(!_changesObserver)
        {
            changesObserver = new DefaultChangesObserver;
            defaultChangesObserver = true;
        }
        else
            changesObserver = _changesObserver;
    }

    virtual ~RBTree()
    {
        if(defaultAllocator)
            delete allocator;
        if(defaultChangesObserver)
            delete  changesObserver;
    }

    void SetAllocator(RBTreeAllocator<Key, Value>* _allocator)
    {
        if(!_allocator)	return;
        if(defaultAllocator)
        {
            delete allocator;
            defaultAllocator = false;
        }
        allocator = _allocator;
        rootNode = allocator->getRootNode();
        nilNode = allocator->getNilNode();
        // offset = allocator->getOffset();
        // size = allocator->getSize();
        if ( ! rootNode || ! nilNode ) {
            fprintf( stderr, "SetAllocator: rootNode=%ld, nilNode=%ld\n", (long)relativeAddr(rootNode), (long)relativeAddr(nilNode) );
            if (logger) smsc_log_error( logger, "SetAllocator: rootNode=%ld, nilNode=%ld", (long)relativeAddr(rootNode), (long)relativeAddr(nilNode) );
        }
        const std::vector< RBTreeNode* > freenodes = allocator->freenodes();
        int cnt = dumpcheck( freenodes, rootNode, nilNode, 0, "" );
        if (logger) smsc_log_info( logger, "SetAllocator: rbtree nodes=%d", cnt );
    }

    void SetChangesObserver(RBTreeChangesObserver<Key, Value>* _changesObserver)
    {
        if(!_changesObserver)	return;
        if(defaultChangesObserver)
        {
            delete changesObserver;
            defaultChangesObserver = false;
        }
        changesObserver = _changesObserver;
    }
    
    const RBTreeAllocator<Key, Value>* SetAllocator(void)
    {
        return allocator;
    }

    int Insert(const Key& k, const Value& v)
    {
        if (logger) smsc_log_debug(logger, "Start Insert: k=%s val=%lld", k.toString().c_str(), (long long)v);
        RBTreeNode* newNode = allocator->allocateNode();
        rootNode = allocator->getRootNode();
        nilNode = allocator->getNilNode();
        // offset = allocator->getOffset();
        newNode->key = k;
        newNode->value = v;
        if (logger) smsc_log_debug(logger, "Insert: %ld k=%s val=%lld", (long)relativeAddr(newNode), k.toString().c_str(), (long long)v);
        changesObserver->startChanges(newNode, RBTreeChangesObserver<Key, Value>::OPER_INSERT);
        bstInsert(newNode);
        newNode->color = RED;
        changesObserver->nodeChanged(newNode);
        rbtRecovery(newNode);
        changesObserver->completeChanges();
        if (logger) smsc_log_debug(logger, "End Insert: %ld k=%s val=%lld", (long)relativeAddr(newNode), k.toString().c_str(), (long long)v);
        return 1;
    }

    bool Set(const Key& k, const Value& val) {
        RBTreeNode*	node = rootNode;
        while( (node != nilNode) && !(node->key == k) )
        {
            if(node->key < k)
                node = realAddr(node->right);
            else
                node = realAddr(node->left);
        }
        if (node == nilNode) {
            Insert(k, val);
            return false;
        }
        setNodeValue( node, val );
        // node->value = val;
        // changesObserver->completeChanges();
        return true;
    }

    void setNodeValue(RBTreeNode* node, const Value& val) {
        if (!node || node->value == val) {
            return;
        }
        if (logger) smsc_log_debug(logger, "Node %ld k=%s value has changed. old=%lld new=%lld",
                                   (long)relativeAddr(node),
                                   node->key.toString().c_str(),
                                   (long long)node->value, (long long)val);
        node->value = val;
        changesObserver->startChanges(node, RBTreeChangesObserver<Key, Value>::OPER_CHANGE);
        // changesObserver->nodeChanged(node);
        changesObserver->completeChanges();
        if (logger) smsc_log_debug( logger, "Node key=%s setNodeValue finished",
                                    node->key.toString().c_str() );
    }

    bool Get(const Key& k, Value& val)
    {
        RBTreeNode*	node = rootNode;
        while( (node != nilNode) && !(node->key == k) )
        {
            if(node->key < k)
                node = realAddr(node->right);
            else
                node = realAddr(node->left);
        }
        
        if(node == nilNode) {
            if (logger) smsc_log_debug(logger, "Get: k=%s. Index not found", k.toString().c_str());
            return false;
        } 
        val = node->value;
        if (logger) smsc_log_debug(logger, "Get: %ld k=%s val=%lld", (long)relativeAddr(node), k.toString().c_str(), (long long)val);
        return true;
    }

    RBTreeNode* Get(const Key& k) {
        RBTreeNode* node = rootNode;
        while ((node != nilNode) && !(node->key == k)) {
            node = node->key < k ? realAddr(node->right) : realAddr(node->left);
        }
        if(node == nilNode) {
            if (logger) smsc_log_debug(logger, "Get: k=%s. Index not found", k.toString().c_str());
            return 0;
        }
        if (logger) smsc_log_debug(logger, "Get: %ld k=%s val=%lld", (long)relativeAddr(node), k.toString().c_str(), (long long)node->value);
        return node;
    }

    void Reset()
    {
        // iterNode = nilNode;
        // move the the far left end
        iterNode = rootNode;
        if ( rootNode != nilNode ) moveLeft();
    }

    bool Next(Key& k, Value& val)
    {
        if ( iterNode == nilNode ) return false;
        k = iterNode->key;
        val = iterNode->value;

        // shownode( "iter at Next", iterNode );

        RBTreeNode *right = realAddr(iterNode->right);
        // shownode( "iter->right", right );
        if (right != nilNode) {
            iterNode = right;
            moveLeft();
        } else {
            while ( true ) {
                RBTreeNode* parent = realAddr(iterNode->parent);
                // shownode( "iter->parent", parent );
                if ( realAddr(parent->left) == iterNode ) {
                    // smsc_log_debug( logger, "right parent found" );
                    iterNode = parent;
                    break;
                }
                if ( parent == rootNode ) {
                    // smsc_log_debug( logger, "iteration finished" );
                    iterNode = nilNode;
                    break;
                }
                iterNode = parent;
                // smsc_log_debug( logger, "move up" );
            }
        }
        return true;
    }

protected:

    void moveLeft()
    {
        while ( true ) {
            // shownode( "moving left", iterNode );
            RBTreeNode* left = realAddr( iterNode->left );
            if ( left == nilNode ) break;
            iterNode = left;
        }
        // shownode( "after moveLeft", iterNode );
    }

    int dumpcheck( const std::vector< RBTreeNode* >& freenodes,
                   RBTreeNode* node,
                   RBTreeNode* parent,
                   int depth = 0,
                   const std::string& path = "" ) const
    {
        if ( !logger ) return 0;
        if ( node == nilNode ) return 0;

        int res = 0;
        if ( logger->isDebugEnabled() && depth < 6 ) {
            char buf[100];
            snprintf( buf, sizeof(buf), "%d ", depth );
            shownode( (std::string(buf) + path).c_str(), node );
        }
        RBTreeNode* realparent = realAddr(node->parent);
        RBTreeNode* left = realAddr(node->left);
        RBTreeNode* right = realAddr(node->right);
        const char* fail = NULL;
        do {

            if ( depth > 500 ) {
                fail = "too deep";
                break;
            }

            if ( realparent != parent ) {
                fail = "parentlink";
                break;
            }

            // check if the node is in the list of free nodes
            if ( binary_search( freenodes.begin(), freenodes.end(), node ) ) {
                fail = "used freecell";
                break;
            }

            ++depth;
            if ( left != nilNode ) {
                if ( ! (left->key < node->key) ) {
                    fail = "order/left";
                    break;
                }
                res += dumpcheck( freenodes, left, node, depth, path + "l" );
            }
            if ( right != nilNode ) {
                if ( ! (node->key < right->key) ) {
                    fail = "order/right";
                    break;
                }
                res += dumpcheck( freenodes, right, node, depth, path + "r" );
            }
            
        } while ( false );

        if ( fail ) {
            smsc_log_error( logger,
                            "ERROR: corruption (%s) at depth=%d path=%s node=(%ld:%s) parent=(%ld:%s) node->parent=(%ld:%s) node->left=(%ld:%s) node->right=(%ld:%s)",
                            fail,
                            depth,
                            path.c_str(),
                            (long)relativeAddr(node), nodekey(node).c_str(),
                            (long)relativeAddr(parent), nodekey(parent).c_str(),
                            (long)relativeAddr(realparent), nodekey(realparent).c_str(),
                            (long)relativeAddr(left), nodekey(left).c_str(),
                            (long)relativeAddr(right), nodekey(right).c_str() );
            throw std::runtime_error("RBTree structure corrupted");
        }
        return res+1;
    }


    void shownode( const char* text, RBTreeNode* node ) const
    {
        if ( !logger ) return;
        if ( node == nilNode ) {
            smsc_log_debug( logger, "%s: nilnode" );
        } else {
            smsc_log_debug( logger, "%s: (%ld:%s) left=(%ld:%s) right=(%ld:%s) parent=(%ld:%s)",
                            text,
                            (long)relativeAddr(node), nodekey(node).c_str(),
                            (long)node->left, nodekey(realAddr(node->left)).c_str(),
                            (long)node->right, nodekey(realAddr(node->right)).c_str(),
                            (long)node->parent, nodekey(realAddr(node->parent)).c_str() );
        }
    }

    std::string nodekey( const RBTreeNode* node ) const
    {
        if ( node == nilNode ) return "nil";
        return node->key.toString();
    }

    inline RBTreeNode* realAddr( RBTreeNode* node) const
    {
        return (RBTreeNode*)(int64_t(node) + allocator->getOffset());
    }
    inline RBTreeNode* relativeAddr( RBTreeNode* node) const
    {
        return (RBTreeNode*)(int64_t(node) - allocator->getOffset());
    }
    inline RBTreeNode* set(RBTreeNode** to, RBTreeNode* from)
    {
        *to = from;
        return *to;
    }
	
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	
	//		��� ���������� ��������� �� RBTreeNode �������� ���������� ���������, 
	//	����� ��� ���� �������� RBTreeNode - ��� ��������� ������ ������� ���������,
	//	�.�. ��������� ������������ ���������� �� offset.
	//
	void rbtRotateLeft(RBTreeNode* node)
	{
		RBTreeNode*	tempNode = realAddr(node->right);	//node->right;
		node->right = tempNode->left;				
		changesObserver->nodeChanged(node);

		if(realAddr(tempNode->left) != nilNode)
		{
			realAddr(tempNode->left)->parent = relativeAddr(node); 
			changesObserver->nodeChanged(realAddr(tempNode->left));
		}
		tempNode->parent = node->parent;
		changesObserver->nodeChanged(tempNode);

		if(realAddr(node->parent) == nilNode)
		{
			rootNode = tempNode;
			allocator->setRootNode(rootNode);
		}
		else
			if( node == realAddr(realAddr(node->parent)->left) ) //node == node->parent->left)
			{
				realAddr(node->parent)->left = relativeAddr(tempNode); //node->parent->left = tempNode;
				changesObserver->nodeChanged(realAddr(node->parent));
			}
			else
			{
				realAddr(node->parent)->right = relativeAddr(tempNode); //node->parent->right = tempNode;
				changesObserver->nodeChanged(realAddr(node->parent));
			}

		tempNode->left = relativeAddr(node);
		node->parent = relativeAddr(tempNode);

		changesObserver->nodeChanged(tempNode);
		changesObserver->nodeChanged(node);

	}
	void rbtRotateRight(RBTreeNode* node)
	{
		RBTreeNode*	tempNode = realAddr(node->left);
		node->left = tempNode->right;
		changesObserver->nodeChanged(node);

		if(realAddr(tempNode->right) != nilNode)
		{
			realAddr(tempNode->right)->parent = relativeAddr(node);
			changesObserver->nodeChanged(realAddr(tempNode->right));
		}
		tempNode->parent = node->parent;
		changesObserver->nodeChanged(tempNode);

		if(realAddr(node->parent) == nilNode)
		{
			rootNode = tempNode;
			allocator->setRootNode(rootNode);
//			changesObserver->nodeChanged(rootNode);
		}
		else
			if(node == realAddr(realAddr(node->parent)->right) ) //node == node->parent->right
			{
				realAddr(node->parent)->right = relativeAddr(tempNode);
				changesObserver->nodeChanged(realAddr(node->parent));
			}
			else
			{
				realAddr(node->parent)->left = relativeAddr(tempNode); //node->parent->left = tempNode;
				changesObserver->nodeChanged(realAddr(node->parent));
			}
		tempNode->right = relativeAddr(node);
		node->parent = relativeAddr(tempNode);

		changesObserver->nodeChanged(tempNode);
		changesObserver->nodeChanged(node);
	}
	int bstInsert(RBTreeNode* newNode)
	{
		RBTreeNode*	tempNode = rootNode;
		RBTreeNode* parentNode = nilNode;
//		printf("bstInsert 111 rootNode = %p, nilNode = %p, tempNode = %p\n", rootNode, nilNode, tempNode);
		while(tempNode != nilNode)
		{
			parentNode = tempNode;
			if(newNode->key < tempNode->key)
				tempNode = realAddr(tempNode->left);
			else
				tempNode = realAddr(tempNode->right);
		}
		newNode->parent = relativeAddr(parentNode);
		changesObserver->nodeChanged(newNode);
		if(parentNode == nilNode )
		{
			rootNode = newNode;
			allocator->setRootNode(rootNode);
		}
		else
		{
			if(newNode->key < parentNode->key)
				parentNode->left = relativeAddr(newNode);
			else
				parentNode->right =  relativeAddr(newNode);
			changesObserver->nodeChanged(parentNode);
		}
		return 0;
	}

	int rbtRecovery(RBTreeNode* node)
	{
		RBTreeNode*	tempNode;

		while( (node != rootNode) && (realAddr(node->parent)->color == RED) )
		{
			if(node->parent == realAddr(realAddr(node->parent)->parent)->left)
			{
				tempNode = realAddr(realAddr(realAddr(node->parent)->parent)->right);
				if(tempNode->color == RED) //if(tempNode &&tempNode->color == RED)
				{
					realAddr(node->parent)->color = BLACK;
					tempNode->color = BLACK; 
					realAddr(realAddr(node->parent)->parent)->color = RED;

					changesObserver->nodeChanged(realAddr(node->parent));
					changesObserver->nodeChanged(tempNode);
					changesObserver->nodeChanged(realAddr(realAddr(node->parent)->parent));
                    
					node = realAddr(realAddr(node->parent)->parent);                    
				}
				else
				{
					if( node == realAddr(realAddr(node->parent)->right) )
					{
						node = realAddr(node->parent);
						rbtRotateLeft(node);
					}
					realAddr(node->parent)->color = BLACK;
					realAddr(realAddr(node->parent)->parent)->color = RED;
					rbtRotateRight(realAddr(realAddr(node->parent)->parent));

					changesObserver->nodeChanged(realAddr(node->parent));
					changesObserver->nodeChanged(realAddr(realAddr(node->parent)->parent));
				}
			}
			else
			{
				tempNode = realAddr(realAddr(realAddr(node->parent)->parent)->left);
				if(tempNode->color == RED)
				{
					realAddr(node->parent)->color = BLACK;
					tempNode->color = BLACK;
					realAddr(realAddr(node->parent)->parent)->color = RED;
					
					changesObserver->nodeChanged(realAddr(node->parent));
					changesObserver->nodeChanged(tempNode);
					changesObserver->nodeChanged(realAddr(realAddr(node->parent)->parent));
                    
					node = realAddr(realAddr(node->parent)->parent);                    
				}
				else
				{
					if( node == realAddr(realAddr(node->parent)->left) )
					{
						node = realAddr(node->parent);
						rbtRotateRight(node);
					}
					realAddr(node->parent)->color = BLACK;
					realAddr(realAddr(node->parent)->parent)->color = RED;
					rbtRotateLeft(realAddr(realAddr(node->parent)->parent));

					changesObserver->nodeChanged(realAddr(node->parent));
					changesObserver->nodeChanged(realAddr(realAddr(node->parent)->parent));
				}
			}
		}
		rootNode->color = BLACK;
		changesObserver->nodeChanged(rootNode);
		return 1;
	}

private:
    smsc::logger::Logger*               logger;
    RBTreeAllocator<Key, Value>*	allocator;
    RBTreeChangesObserver<Key, Value>*	changesObserver;
    RBTreeNode*		                rootNode;
    RBTreeNode*		                nilNode;
    RBTreeNode*                         iterNode;
    // long			offset;
    // long			size;
    bool			        defaultAllocator;
    bool			        defaultChangesObserver;
};


} // namespace storage
} // namespace util
} // namespace scag

#endif /* ! _SCAG_UTIL_STORAGE_RBTREE_H */


//	void rbtRotateLeft(RBTreeNode* node)
//	{
//		RBTreeNode*	tempNode = realAddr(node->right);	//node->right;
//		set(&(node->right), tempNode->left);				//node->right = tempNode->left;
//
//		if(realAddr(tempNode->left) != nilNode)
//			realAddr(tempNode->left)->parent = relativeAddr(node); //tempNode->left->parent = node;
//		tempNode->parent = node->parent;
//
//		if(realAddr(node->parent) == nilNode)
//		{
//			rootNode = tempNode;
//			allocator->setRootNode(rootNode);
//		}
//		else
//			if( node == realAddr(realAddr(node->parent)->left) ) //node == node->parent->left)
//				realAddr(node->parent)->left = relativeAddr(tempNode); //node->parent->left = tempNode;
//			else
//				realAddr(node->parent)->right = relativeAddr(tempNode); //node->parent->right = tempNode;
//		tempNode->left = relativeAddr(node);
//		node->parent = relativeAddr(tempNode);
//	}
//	void rbtRotateRight(RBTreeNode* node)
//	{
//		RBTreeNode*	tempNode = realAddr(node->left);
//		node->left = tempNode->right;
//
//		if(realAddr(tempNode->right) != nilNode)
//			realAddr(tempNode->right)->parent = relativeAddr(node);
//		tempNode->parent = node->parent;
//
//		if(realAddr(node->parent) == nilNode)
//		{
//			rootNode = tempNode;
//			allocator->setRootNode(rootNode);
//		}
//		else
//			if(node == realAddr(realAddr(node->parent)->right) ) //node == node->parent->right
//				realAddr(node->parent)->right = relativeAddr(tempNode); //node->parent->right = tempNode;
//			else
//				realAddr(node->parent)->left = relativeAddr(tempNode); //node->parent->left = tempNode;
//		tempNode->right = relativeAddr(node);
//		node->parent = relativeAddr(tempNode);
//	}
//	int bstInsert(RBTreeNode* newNode)
//	{
//		RBTreeNode*	tempNode = rootNode;
//		RBTreeNode* parentNode = nilNode;
////		printf("bstInsert 111 rootNode = %p, nilNode = %p, tempNode = %p\n", rootNode, nilNode, tempNode);
//		while(tempNode != nilNode)
//		{
//			parentNode = tempNode;
//			if(newNode->key < tempNode->key)
//				tempNode = realAddr(tempNode->left);
//			else
//				tempNode = realAddr(tempNode->right);
//		}
//		newNode->parent = relativeAddr(parentNode);
//		if(parentNode == nilNode )
//		{
//			rootNode = newNode;
//			allocator->setRootNode(rootNode);
//		}
//		else
//			if(newNode->key < parentNode->key)
//				parentNode->left = relativeAddr(newNode);
//			else
//				parentNode->right = relativeAddr(newNode);
//		return 0;
//	}
//
//	int rbtRecovery(RBTreeNode* node)
//	{
//		RBTreeNode*	tempNode;
//
//		while( (node != rootNode) && (realAddr(node->parent)->color == RED) )
//		{
//			if(node->parent == realAddr(realAddr(node->parent)->parent)->left)
//			{
//				tempNode = realAddr(realAddr(realAddr(node->parent)->parent)->right);
//				if(tempNode->color == RED) //if(tempNode &&tempNode->color == RED)
//				{
//					realAddr(node->parent)->color = BLACK;
//					tempNode->color = BLACK;
//					realAddr(realAddr(node->parent)->parent)->color = RED;
//					node = realAddr(realAddr(node->parent)->parent);
//				}
//				else
//				{
//					if( node == realAddr(realAddr(node->parent)->right) )
//					{
//						node = realAddr(node->parent);
//						rbtRotateLeft(node);
//					}
//					realAddr(node->parent)->color = BLACK;
//					realAddr(realAddr(node->parent)->parent)->color = RED;
//					rbtRotateRight(realAddr(realAddr(node->parent)->parent));
//				}
//			}
//			else
//			{
//				tempNode = realAddr(realAddr(realAddr(node->parent)->parent)->left);
//				if(tempNode->color == RED) //if(tempNode && (tempNode->color == RED))
//				{
//					realAddr(node->parent)->color = BLACK;
//					tempNode->color = BLACK;
//					realAddr(realAddr(node->parent)->parent)->color = RED;
//					node = realAddr(realAddr(node->parent)->parent);
//				}
//				else
//				{
//					if( node == realAddr(realAddr(node->parent)->left) )
//					{
//						node = realAddr(node->parent);
//						rbtRotateRight(node);
//					}
//					realAddr(node->parent)->color = BLACK;
//					realAddr(realAddr(node->parent)->parent)->color = RED;
//					rbtRotateLeft(realAddr(realAddr(node->parent)->parent));
//				}
//			}
//		}
//		rootNode->color = BLACK;
//		return 1;
//	}
//};

