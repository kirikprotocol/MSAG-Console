//------------------------------------
//  RBTreee.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	���� �������� �������� ������ RBTreee.
//

#include "RBTreeAllocator.h"
#include "unistd.h"
#include "logger/Logger.h"

#ifndef ___RBTREE_H
#define ___RBTREE_H

template<class Key=long, class Value=long, class DefaultAllocator = SimpleAllocator<Key, Value>, class DefaultChangesObserver = EmptyChangesObserver<Key, Value> >
class RBTree
{
public:
	typedef templRBTreeNode<Key, Value>   RBTreeNode;
    typedef typename RBTreeNode::nodeptr_type nodeptr_type;

	RBTree(RBTreeAllocator<Key, Value>* _allocator=0, RBTreeChangesObserver<Key, Value>* _changesObserver=0):defaultAllocator(false), defaultChangesObserver(false)
	{
        logger = smsc::logger::Logger::getInstance("RBTree");
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

    RBTreeNode* realAddr( nodeptr_type n ) {
        return allocator->realAddr(n);
    }


    /*
	const RBTreeAllocator<Key, Value>* SetAllocator(void)
	{
		return allocator;
	}
     */

	int Insert(const Key& k, const Value& v)
	{
        smsc_log_debug(logger, "Start Insert: %s val=%lld", k.toString().c_str(), (long long)v);
            nodeptr_type nn = allocator->allocateNode();
            RBTreeNode* newNode = allocator->realAddr(nn);
		// rootNode = allocator->getRootNode();
		// nilNode = allocator->getNilNode();
		// offset = allocator->getOffset();
		newNode->key = k;
		newNode->value = v;
        smsc_log_debug(logger, "Insert: %s val=%lld n=%ld", k.toString().c_str(), (long long)v, (long)nn);
		changesObserver->startChanges(nn, RBTreeChangesObserver<Key, Value>::OPER_INSERT);
		bstInsert(nn);
		newNode->color = RED;
		changesObserver->nodeChanged(nn);
		rbtRecovery(nn);
		changesObserver->completeChanges();
        smsc_log_debug(logger, "End Insert: %s val=%lld n=%ld", k.toString().c_str(), (long long)v, (long)nn);
		return 1;
    }

    bool Set(const Key& k, const Value& val) {
        nodeptr_type node = findNode(k);
        /*
      RBTreeNode*	node = rootNode;
      while( (node != nilNode) && (node->key != k) )
      {
          if(node->key < k)
              node = realAddr(node->right);
          else
              node = realAddr(node->left);
      }
         */
      if (node == nilNode) {
        Insert(k, val);
        return false;
      }
        setNodeValue(node,val);
        /*
      node->value = val;
      changesObserver->nodeChanged(node);
      changesObserver->completeChanges();
         */
      return true;
    }

    void setNodeValue(nodeptr_type inode, const Value& val) {
        RBTreeNode* node = allocator->realAddr(inode);
      if (!node || node->value == val) {
        return;
      }
      smsc_log_debug(logger, "Node key=%s value has changed. old=%lld new=%lld n=%ld",
                      node->key.toString().c_str(), (long long)node->value, (long long)val, (long)inode);
      node->value = val;        
      changesObserver->startChanges(inode, RBTreeChangesObserver<Key,Value>::OPER_CHANGE);
      // changesObserver->nodeChanged(node);
      changesObserver->completeChanges();
    }

	bool Get(const Key& k, Value& val)
	{
            nodeptr_type node = findNode(k);
            /*
		RBTreeNode*	node = rootNode;
		while( (node != nilNode) && (node->key != k) )
		{
			if(node->key < k)
				node = realAddr(node->right);
			else
				node = realAddr(node->left);
		}
             */
		if(node == nilNode) {
          smsc_log_debug(logger, "Get: %s. Index not found", k.toString().c_str());
          return false;
        } 
		val = allocator->realAddr(node)->value;
        smsc_log_debug(logger, "Get: %s val=%lld n=%ld", k.toString().c_str(), (long long)val, (long)node);
		return true;
	}

    nodeptr_type Get(const Key& k) {
        nodeptr_type node = findNode(k);
        /*
      RBTreeNode* node = rootNode;
      while ((node != nilNode) && (node->key != k)) {
        node = node->key < k ? realAddr(node->right) : realAddr(node->left);
      }
         */
      if(node == nilNode) {
        smsc_log_debug(logger, "Get: %s. Index not found", k.toString().c_str());
        return nilNode;
      } 
      smsc_log_debug(logger, "Get: %s val=%lld n=%ld", k.toString().c_str(), (long long)allocator->realAddr(node)->value, (long)node);
      return node;
    }


    void Reset()
    {
        // iterNode = nilNode;
        iterNode = rootNode;
        if (rootNode !=nilNode) moveLeft();
    }
	bool Next(Key& k, Value& val)
	{
        // if(rootNode == nilNode || iterNode == rootNode) return false;
        // if(iterNode == nilNode) iterNode = rootNode;
        if ( iterNode == nilNode ) return false;
        RBTreeNode* iter = allocator->realAddr(iterNode);
        
        k = iter->key;
        val = iter->value;

        nodeptr_type right = iter->right;
        if(right != nilNode) {
            iterNode = right;
            moveLeft();
        } else {
            while ( true ) {
                nodeptr_type parent = iter->parent;
                if ( allocator->realAddr(parent)->left == iterNode ) {
                    iterNode = parent;
                    break;
                }
                if ( parent == rootNode ) {
                    iterNode = nilNode;
                    break;
                }
                iterNode = parent;
                iter = allocator->realAddr(iterNode);
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


	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	
	//		��� ���������� ��������� �� RBTreeNode �������� ���������� ���������, 
	//	����� ��� ���� �������� RBTreeNode - ��� ��������� ������ ������� ���������,
	//	�.�. ��������� ������������ ���������� �� offset.
	//
	void rbtRotateLeft(nodeptr_type iNode)
	{
            RBTreeNode* node = allocator->realAddr(iNode);
            nodeptr_type iTempNode = node->right;
            RBTreeNode*	tempNode = allocator->realAddr(iTempNode);	//node->right;
            node->right = tempNode->left;				
		changesObserver->nodeChanged(iNode);

		if(tempNode->left != nilNode)
		{
			allocator->realAddr(tempNode->left)->parent = iNode;
			changesObserver->nodeChanged(tempNode->left);
		}
		tempNode->parent = node->parent;
		changesObserver->nodeChanged(iTempNode);

		if(node->parent == nilNode)
		{
			rootNode = iTempNode;
			allocator->setRootNode(rootNode);
		}
		else {
                    RBTreeNode* nodeParent = allocator->realAddr(node->parent);
                    if ( iNode == nodeParent->left ) //node == node->parent->left)
                    {
                        nodeParent->left = iTempNode;
                    }
                    else
                    {
                        nodeParent->right = iTempNode;
                    }
                    changesObserver->nodeChanged(node->parent);
                }

                tempNode->left = iNode;
		node->parent = iTempNode;

		changesObserver->nodeChanged(iTempNode);
		changesObserver->nodeChanged(iNode);

	}
	void rbtRotateRight(nodeptr_type iNode)
	{
            RBTreeNode* node = allocator->realAddr(iNode);
            nodeptr_type iTempNode = node->left;
		RBTreeNode*	tempNode = allocator->realAddr(iTempNode);
		node->left = tempNode->right;
		changesObserver->nodeChanged(iNode);

		if(tempNode->right != nilNode)
		{
			allocator->realAddr(tempNode->right)->parent = iNode;
			changesObserver->nodeChanged(tempNode->right);
		}
		tempNode->parent = node->parent;
		changesObserver->nodeChanged(iTempNode);

		if(node->parent == nilNode)
		{
			rootNode = iTempNode;
			allocator->setRootNode(rootNode);
//			changesObserver->nodeChanged(rootNode);
		}
		else {
                    RBTreeNode* nodeParent = allocator->realAddr(node->parent);
                    if ( iNode == nodeParent->right ) //node == node->parent->right
                    {
                        nodeParent->right = iTempNode;
                    }
                    else
                    {
                        nodeParent->left = iTempNode; //node->parent->left = tempNode;
                    }
                    changesObserver->nodeChanged(node->parent);
                }
                tempNode->right = iNode;
		node->parent = iTempNode;

		changesObserver->nodeChanged(iTempNode);
		changesObserver->nodeChanged(iNode);
	}
	int bstInsert(nodeptr_type iNewNode)
	{
            RBTreeNode* newNode = allocator->realAddr(iNewNode);
            nodeptr_type tempNode = rootNode;
            nodeptr_type parentNode = nilNode;
//		printf("bstInsert 111 rootNode = %p, nilNode = %p, tempNode = %p\n", rootNode, nilNode, tempNode);
		while(tempNode != nilNode)
		{
			parentNode = tempNode;
                    RBTreeNode* temp = allocator->realAddr(tempNode);
			if(newNode->key < temp->key)
				tempNode = temp->left;
			else
				tempNode = temp->right;
		}
		newNode->parent = parentNode;
		changesObserver->nodeChanged(iNewNode);
		if(parentNode == nilNode )
		{
			rootNode = iNewNode;
			allocator->setRootNode(rootNode);
		}
		else
		{
                    RBTreeNode* parent = allocator->realAddr(parentNode);
			if(newNode->key < parent->key)
				parent->left = iNewNode;
			else
				parent->right = iNewNode;
			changesObserver->nodeChanged(parentNode);
		}
		return 0;
	}

	int rbtRecovery(nodeptr_type node)
	{
		while(node != rootNode)
		{
                    RBTreeNode* theNode = allocator->realAddr(node);
                    RBTreeNode* parent = allocator->realAddr(theNode->parent);
                    if ( parent->color != RED ) break;
                    RBTreeNode* grandpa = allocator->realAddr(parent->parent);

			if(theNode->parent == grandpa->left)
			{
                            nodeptr_type iTempNode = grandpa->right;
                            RBTreeNode* tempNode = allocator->realAddr(iTempNode);
				if(tempNode->color == RED) //if(tempNode &&tempNode->color == RED)
				{
					parent->color = BLACK;
					tempNode->color = BLACK; 
					grandpa->color = RED;

					changesObserver->nodeChanged(theNode->parent);
					changesObserver->nodeChanged(iTempNode);
					changesObserver->nodeChanged(parent->parent);
                    
					node = parent->parent;
				}
				else
				{
					if( node == parent->right )
					{
						node = theNode->parent;
						rbtRotateLeft(node);
                                            theNode = allocator->realAddr(node);
                                            parent = allocator->realAddr(theNode->parent);
					}
                                        parent->color = BLACK;
					allocator->realAddr(parent->parent)->color = RED;
					rbtRotateRight(parent->parent);

					changesObserver->nodeChanged(theNode->parent);
					changesObserver->nodeChanged(parent->parent);
				}
			}
			else
			{
                            nodeptr_type iTempNode = grandpa->left;
                            RBTreeNode* tempNode = allocator->realAddr(iTempNode);
				if(tempNode->color == RED)
				{
					parent->color = BLACK;
					tempNode->color = BLACK;
					grandpa->color = RED;
					
					changesObserver->nodeChanged(theNode->parent);
					changesObserver->nodeChanged(iTempNode);
					changesObserver->nodeChanged(parent->parent);
					node = parent->parent;
				}
				else
				{
					if( node == parent->left )
					{
						node = theNode->parent;
						rbtRotateRight(node);
                                            theNode = allocator->realAddr(node);
                                            parent = allocator->realAddr(theNode->parent);
					}
                                        parent->color = BLACK;
					allocator->realAddr(parent->parent)->color = RED;
					rbtRotateLeft(parent->parent);
					changesObserver->nodeChanged(theNode->parent);
					changesObserver->nodeChanged(parent->parent);
				}
			}
		}
                allocator->realAddr(rootNode)->color = BLACK;
		changesObserver->nodeChanged(rootNode);
		return 1;
	}


protected:
    smsc::logger::Logger* logger;
	RBTreeAllocator<Key, Value>*	allocator;
	RBTreeChangesObserver<Key, Value>*	changesObserver;
	nodeptr_type		rootNode;
	nodeptr_type		nilNode;
    nodeptr_type                iterNode;
	bool			defaultAllocator;
	bool			defaultChangesObserver;

};


#endif


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

