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

template<class Key=long, class Value=long, class DefaultAllocator = SimpleAllocator<Key, Value>, class DefaultChangesObserver = EmptyChangesObserver<Key, Value> >
class RBTree
{
public:
    typedef templRBTreeNode<Key, Value>       RBTreeNode;
    typedef typename RBTreeNode::nodeptr_type nodeptr_type;

    RBTree( RBTreeAllocator<Key, Value>* _allocator=0,
            RBTreeChangesObserver<Key, Value>* _changesObserver=0,
            smsc::logger::Logger* thelog = 0 ) : 
    defaultAllocator(false), defaultChangesObserver(false), logger(thelog)
    {
        if (!_allocator) {
            allocator = new DefaultAllocator;
            defaultAllocator = true;
        } else {
            allocator = _allocator;
        }
        rootNode = allocator->getRootNode();
        nilNode = allocator->getNilNode();
        // size = allocator->getSize();
		
        if(!_changesObserver) {
            changesObserver = new DefaultChangesObserver;
            defaultChangesObserver = true;
        } else {
            changesObserver = _changesObserver;
        }
    }

    virtual ~RBTree()
    {
        if (defaultAllocator)
            delete allocator;
        if (defaultChangesObserver)
            delete  changesObserver;
    }

    void SetAllocator( RBTreeAllocator<Key, Value>* _allocator )
    {
        if (!_allocator) return;
        if (defaultAllocator)
        {
            delete allocator;
            defaultAllocator = false;
        }
        allocator = _allocator;
        rootNode = allocator->getRootNode();
        nilNode = allocator->getNilNode();
        // offset = allocator->getOffset();
        // size = allocator->getSize();
        /*
        if ( ! rootNode || ! nilNode ) {
            fprintf( stderr, "SetAllocator: rootNode=%lx, nilNode=%lx\n", (long)relativeAddr(rootNode), (long)relativeAddr(nilNode) );
            if (logger) smsc_log_error( logger, "SetAllocator: rootNode=%lx, nilNode=%lx", (long)relativeAddr(rootNode), (long)relativeAddr(nilNode) );
        }
         */
        // All checking has been moved to allocator
        // const std::vector< nodeptr_type > freenodes = allocator->freenodes();
        // int cnt = dumpcheck( freenodes, rootNode, nilNode, 0, "" );
        // if (logger) smsc_log_info( logger, "SetAllocator: rbtree nodes=%d", cnt );
    }


    unsigned checkTree()
    {
        const std::vector< nodeptr_type > freenodes = allocator->freenodes();
        return dumpcheck( freenodes, rootNode, nilNode, 0, "" );
    }
    
    
    void SetChangesObserver( RBTreeChangesObserver<Key, Value>* _changesObserver)
    {
        if (!_changesObserver) return;
        if (defaultChangesObserver)
        {
            delete changesObserver;
            defaultChangesObserver = false;
        }
        changesObserver = _changesObserver;
    }
    
    /*
    const RBTreeAllocator<Key, Value>* SetAllocator(void)
    {
        return allocator;
    }
     */

    int Insert( const Key& k, const Value& v )
    {
        if (logger) smsc_log_debug(logger, "Start Insert: k=%s val=%lld", k.toString().c_str(), (long long)v);
        nodeptr_type nn = allocator->allocateNode();
        RBTreeNode* newNode = allocator->realAddr(nn);
        // rootNode = allocator->getRootNode();
        // nilNode = allocator->getNilNode();
        // offset = allocator->getOffset();
        newNode->key = k;
        newNode->value = v;
        if (logger) smsc_log_debug(logger, "Insert: %lx k=%s val=%lld", (long)nn, k.toString().c_str(), (long long)v);
        changesObserver->startChanges( nn, RBTreeChangesObserver<Key, Value>::OPER_INSERT );
        bstInsert( nn );
        newNode->color = RED;
        /*
        if ( logger ) {
            smsc_log_debug( logger, "after bstInsert" );
            RBTreeChecker< Key, Value > checker( *allocator,
                                                 0,
                                                 logger,
                                                 100 );
            checker.check( allocator->getRootNode(), allocator->getNilNode() );
        }
         */
        changesObserver->nodeChanged( nn );
        rbtRecovery( nn );
        /*
        if ( logger ) {
            smsc_log_debug( logger, "after rbtRecovery" );
            RBTreeChecker< Key, Value > checker( *allocator,
                                                 0,
                                                 logger,
                                                 100 );
            checker.check( allocator->getRootNode(), allocator->getNilNode() );
        }
         */
        changesObserver->completeChanges();
        if (logger) smsc_log_debug(logger, "End Insert: %lx k=%s val=%lld", (long)nn, k.toString().c_str(), (long long)v);
        return 1;
    }

    bool Set( const Key& k, const Value& val ) {
        nodeptr_type node = findNode(k);
        if (node == nilNode) {
            Insert( k, val );
            return false;
        }
        setNodeValue( node, val );
        return true;
    }

    void setNodeValue( nodeptr_type iNode, const Value& val ) 
    {
        RBTreeNode* node = allocator->realAddr(iNode);
        if (!node || node->value == val) {
            return;
        }
        if (logger) smsc_log_debug( logger, "Node %lx k=%s value has changed. old=%lld new=%lld",
                                    (long)iNode,
                                    node->key.toString().c_str(),
                                    (long long)node->value, (long long)val);
        node->value = val;
        changesObserver->startChanges( iNode, RBTreeChangesObserver<Key, Value>::OPER_CHANGE );
        // changesObserver->nodeChanged(node);
        changesObserver->completeChanges();
        if (logger) smsc_log_debug( logger, "Node key=%s setNodeValue finished",
                                    node->key.toString().c_str() );
    }

    bool Get( const Key& k, Value& val )
    {
        nodeptr_type node = findNode(k);
        if ( node == nilNode ) {
            if (logger) smsc_log_debug(logger, "Get: k=%s. Index not found", k.toString().c_str());
            return false;
        } 
        val = allocator->realAddr(node)->value;
        if (logger) smsc_log_debug(logger, "Get: %lx k=%s val=%lld", (long)node, k.toString().c_str(), (long long)val);
        return true;
    }

    nodeptr_type Get(const Key& k) 
    {
        nodeptr_type node = findNode(k);
        if (node == nilNode ) {
            if (logger) smsc_log_debug(logger, "Get: k=%s. Index not found", k.toString().c_str());
            return nilNode;
        }
        if (logger) smsc_log_debug(logger, "Get: %lx k=%s val=%lld", (long)node, k.toString().c_str(),
                                   (long long)allocator->realAddr(node)->value );
        return node;
    }

    void Reset()
    {
        // iterNode = nilNode;
        // move the the far left end
        iterNode = rootNode;
        if ( rootNode != nilNode ) moveLeft();
    }

    bool Next( Key& k, Value& val )
    {
        if ( iterNode == nilNode ) return false;
        RBTreeNode* iter = allocator->realAddr(iterNode);
        k = iter->key;
        val = iter->value;

        // shownode( "iter at Next", iterNode );

        nodeptr_type right = iter->right;
        // shownode( "iter->right", right );
        if (right != nilNode) {
            iterNode = right;
            moveLeft();
        } else {
            while ( true ) {
                nodeptr_type parent = iter->parent;
                // shownode( "iter->parent", parent );
                if ( allocator->realAddr(parent)->left == iterNode ) {
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
                iter = allocator->realAddr(iterNode);
                // smsc_log_debug( logger, "move up" );
            }
        }
        return true;
    }

protected:

    nodeptr_type findNode( const Key& k )
    {
        nodeptr_type nn = rootNode;
        while ( nn != nilNode ) {
            RBTreeNode* node = allocator->realAddr(nn);
            if ( node->key == k ) break;
            if ( node->key < k ) {
                nn = node->right;
            } else {
                nn = node->left;
            }
        }
        return nn;
    }


    // move iterator to a far left node starting from current one
    void moveLeft()
    {
        while ( true ) {
            // shownode( "moving left", iterNode );
            nodeptr_type left = allocator->realAddr(iterNode)->left;
            if ( left == nilNode ) break;
            iterNode = left;
        }
        // shownode( "after moveLeft", iterNode );
    }


    int dumpcheck( const std::vector< nodeptr_type >& freenodes,
                   nodeptr_type node,
                   nodeptr_type parent,
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
        RBTreeNode* theNode = allocator->realAddr(node);
        nodeptr_type realparent = theNode->parent;
        nodeptr_type left = theNode->left;
        nodeptr_type right = theNode->right;
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
                if ( ! (allocator->realAddr(left)->key < theNode->key) ) {
                    fail = "order/left";
                    break;
                }
                res += dumpcheck( freenodes, left, node, depth, path + "l" );
            }
            if ( right != nilNode ) {
                if ( ! (theNode->key < allocator->realAddr(right)->key) ) {
                    fail = "order/right";
                    break;
                }
                res += dumpcheck( freenodes, right, node, depth, path + "r" );
            }
            
        } while ( false );

        if ( fail ) {
            smsc_log_error( logger,
                            "ERROR: corruption (%s) at depth=%d path=%s node=(%lx:%s) parent=(%lx:%s) node->parent=(%lx:%s) node->left=(%lx:%s) node->right=(%lx:%s)",
                            fail,
                            depth,
                            path.c_str(),
                            (long)node, nodekey(node).c_str(),
                            (long)parent, nodekey(parent).c_str(),
                            (long)realparent, nodekey(realparent).c_str(),
                            (long)left, nodekey(left).c_str(),
                            (long)right, nodekey(right).c_str() );
            throw std::runtime_error("RBTree structure corrupted");
        }
        return res+1;
    }


    void shownode( const char* text, nodeptr_type node ) const
    {
        if ( !logger ) return;
        if ( node == nilNode ) {
            smsc_log_debug( logger, "%s: nilnode" );
        } else {
            RBTreeNode* theNode = allocator->realAddr(node);
            smsc_log_debug( logger, "%s: (%lx:%s) left=(%lx:%s) right=(%lx:%s) parent=(%lx:%s)",
                            text,
                            (long)node, nodekey(node).c_str(),
                            (long)theNode->left, nodekey(theNode->left).c_str(),
                            (long)theNode->right, nodekey(theNode->right).c_str(),
                            (long)theNode->parent, nodekey(theNode->parent).c_str() );
        }
    }

    std::string nodekey( nodeptr_type node ) const
    {
        if ( node == nilNode ) return "nil";
        return allocator->realAddr(node)->key.toString();
    }

    /*
    inline RBTreeNode* set(RBTreeNode** to, RBTreeNode* from)
    {
        *to = from;
        return *to;
    }
     */
	
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	
	//		��� ���������� ��������� �� RBTreeNode �������� ���������� ���������, 
	//	����� ��� ���� �������� RBTreeNode - ��� ��������� ������ ������� ���������,
	//	�.�. ��������� ������������ ���������� �� offset.
	//
    void rbtRotateLeft( nodeptr_type iNode )
    {
        RBTreeNode* node = allocator->realAddr(iNode);
        nodeptr_type iTempNode = node->right;
        RBTreeNode* tempNode = allocator->realAddr(iTempNode);
        node->right = tempNode->left;				
        changesObserver->nodeChanged(iNode);

        if( tempNode->left != nilNode ) {
            allocator->realAddr(tempNode->left)->parent = iNode;
            changesObserver->nodeChanged(tempNode->left);
        }
        tempNode->parent = node->parent;
        changesObserver->nodeChanged(iTempNode);

        if ( node->parent == nilNode ) {
            allocator->setRootNode(rootNode = iTempNode );
        } else {
            RBTreeNode* nodeParent = allocator->realAddr(node->parent);
            if ( iNode == nodeParent->left ) {
                nodeParent->left = iTempNode;
            } else {
                nodeParent->right = iTempNode;
            }
            changesObserver->nodeChanged(node->parent);
        }
        
        tempNode->left = iNode;
        node->parent = iTempNode;

        changesObserver->nodeChanged(iTempNode);
        changesObserver->nodeChanged(iNode);
    }

    void rbtRotateRight( nodeptr_type iNode)
    {
        RBTreeNode* node = allocator->realAddr(iNode);
        nodeptr_type iTempNode = node->left;
        RBTreeNode* tempNode = allocator->realAddr(iTempNode);
        node->left = tempNode->right;
        changesObserver->nodeChanged(iNode);

        if ( tempNode->right != nilNode ) {
            allocator->realAddr(tempNode->right)->parent = iNode;
            changesObserver->nodeChanged(tempNode->right);
        }
        tempNode->parent = node->parent;
        changesObserver->nodeChanged(iTempNode);

        if ( node->parent == nilNode ) {
            allocator->setRootNode(rootNode = iTempNode);
        } else {
            RBTreeNode* nodeParent = allocator->realAddr(node->parent);
            if ( iNode == nodeParent->right ) {
                nodeParent->right = iTempNode;
            } else {
                nodeParent->left = iTempNode;
            }
            changesObserver->nodeChanged(node->parent);
        }
        tempNode->right = iNode;
        node->parent = iTempNode;

        changesObserver->nodeChanged(iTempNode);
        changesObserver->nodeChanged(iNode);
    }


    void bstInsert( nodeptr_type iNewNode )
    {
        RBTreeNode* newNode = allocator->realAddr(iNewNode);
        nodeptr_type tempNode = rootNode;
        nodeptr_type parentNode = nilNode;
        //		printf("bstInsert 111 rootNode = %p, nilNode = %p, tempNode = %p\n", rootNode, nilNode, tempNode);
        while (tempNode != nilNode)
        {
            parentNode = tempNode;
            RBTreeNode* temp = allocator->realAddr(tempNode);
            if (newNode->key < temp->key)
                tempNode = temp->left;
            else
                tempNode = temp->right;
        }
        newNode->parent = parentNode;
        changesObserver->nodeChanged(iNewNode);
        if (parentNode == nilNode ) {
            allocator->setRootNode(rootNode = iNewNode);
        } else {
            RBTreeNode* parent = allocator->realAddr(parentNode);
            if (newNode->key < parent->key)
                parent->left = iNewNode;
            else
                parent->right = iNewNode;
            changesObserver->nodeChanged(parentNode);
        }
    }


    void rbtRecovery( nodeptr_type node )
    {
        while ( node != rootNode ) {
            RBTreeNode* theNode = allocator->realAddr(node);
            RBTreeNode* parent = allocator->realAddr(theNode->parent);
            if ( parent->color != RED ) break;
            RBTreeNode* grandpa = allocator->realAddr(parent->parent);

            if ( theNode->parent == grandpa->left ) {
                nodeptr_type iTempNode = grandpa->right;
                RBTreeNode* tempNode = allocator->realAddr(iTempNode);
                if (tempNode->color == RED ) {
                    parent->color = BLACK;
                    tempNode->color = BLACK; 
                    grandpa->color = RED;
                    changesObserver->nodeChanged(theNode->parent);
                    changesObserver->nodeChanged(iTempNode);
                    changesObserver->nodeChanged(parent->parent);
                    node = parent->parent;
                } else {
                    if( node == parent->right ) {
                        node = theNode->parent;
                        rbtRotateLeft(node);
                        theNode = allocator->realAddr(node);
                        parent = allocator->realAddr(theNode->parent);
                    }
                    parent->color = BLACK;
                    allocator->realAddr(parent->parent)->color = RED;
                    rbtRotateRight( parent->parent );
                    changesObserver->nodeChanged(theNode->parent);
                    changesObserver->nodeChanged(parent->parent);
                }

            } else { // parent is at right

                nodeptr_type iTempNode = grandpa->left;
                RBTreeNode* tempNode = allocator->realAddr(iTempNode);
                if (tempNode->color == RED) {
                    parent->color = BLACK;
                    tempNode->color = BLACK;
                    grandpa->color = RED;
                    changesObserver->nodeChanged(theNode->parent);
                    changesObserver->nodeChanged(iTempNode);
                    changesObserver->nodeChanged(parent->parent);
                    node = parent->parent;
                } else {
                    if ( node == parent->left ) {
                        node = theNode->parent;
                        rbtRotateRight(node);
                        theNode = allocator->realAddr(node);
                        parent = allocator->realAddr(theNode->parent);
                    }
                    parent->color = BLACK;
                    allocator->realAddr(parent->parent)->color = RED;
                    rbtRotateLeft( parent->parent );
                    changesObserver->nodeChanged( theNode->parent );
                    changesObserver->nodeChanged( parent->parent );
                }
            }
        }
        allocator->realAddr(rootNode)->color = BLACK;
        changesObserver->nodeChanged(rootNode);
    }

private:
    smsc::logger::Logger*               logger;
    RBTreeAllocator<Key, Value>*	allocator;
    RBTreeChangesObserver<Key, Value>*	changesObserver;
    nodeptr_type	                rootNode;
    nodeptr_type	                nilNode;
    nodeptr_type                        iterNode;
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
