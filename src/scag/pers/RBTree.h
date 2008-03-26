//------------------------------------
//  RBTreee.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	���� �������� �������� ������ RBTreee.
//

#include "RBTreeAllocator.h"
#include "unistd.h"

#ifndef ___RBTREE_H
#define ___RBTREE_H


const char RED = 1;
const char BLACK = 0;
			
template<class Key, class Value>
struct templRBTreeNode
{
	templRBTreeNode<Key, Value>*	parent;
	templRBTreeNode<Key, Value>*	left;
	templRBTreeNode<Key, Value>*	right;

	int		color;

	Key		key;
	Value	value;

};

//typedef SimpleAllocator DefaultAllocator;

template<class Key=long, class Value=long, class DefaultAllocator = SimpleAllocator<Key, Value>, class DefaultChangesObserver = EmptyChangesObserver<Key, Value> >
class RBTree
{
	typedef templRBTreeNode<Key, Value> RBTreeNode;
public:
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
		size = allocator->getSize();
		
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
		offset = allocator->getOffset();
		size = allocator->getSize();
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
		//printf("Insert\n");
// 		sleep(1);
      smsc_log_debug(logger, "Start Insert: %s val=%d", k.toString().c_str(), (int)v);
		
		RBTreeNode* newNode = allocator->allocateNode();
		rootNode = allocator->getRootNode();
		nilNode = allocator->getNilNode();
		offset = allocator->getOffset();
       
		newNode->key = k;
		newNode->value = v;
        smsc_log_debug(logger, "Insert: %s val=%d", k.toString().c_str(), (int)v);
		changesObserver->startChanges(newNode, RBTreeChangesObserver<Key, Value>::OPER_INSERT);
		bstInsert(newNode);
		newNode->color = RED;
		changesObserver->nodeChanged(newNode);
		rbtRecovery(newNode);
		changesObserver->completeChanges();
        smsc_log_debug(logger, "End Insert: %s val=%d", k.toString().c_str(), (int)v);
		return 1;
    }

    bool Set(const Key& k, const Value& val) {

      RBTreeNode*	node = rootNode;
      while( (node != nilNode) && (node->key != k) )
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
      node->value = val;
      changesObserver->nodeChanged(node);
      changesObserver->completeChanges();
      return true;
    }

	bool Get(const Key& k, Value& val)
	{
		//printf("Get\n");
		RBTreeNode*	node = rootNode;
		while( (node != nilNode) && (node->key != k) )
		{
			if(node->key < k)
				node = realAddr(node->right);
			else
				node = realAddr(node->left);
		}
		
		if(node == nilNode) {
          smsc_log_debug(logger, "Get: %s. Index not found", k.toString().c_str());
          return false;
        } 
		val = node->value;
        smsc_log_debug(logger, "Get: %s val=%d", k.toString().c_str(), (int)val);
		return true;
	}
    void Reset()
    {
        iterNode = nilNode;
    }
	bool Next(Key& k, Value& val)
	{
		//printf("Get\n");
        if(rootNode == nilNode || iterNode == rootNode) return false;
        if(iterNode == nilNode) iterNode = rootNode;
        
        k = iterNode->key;
        val = iterNode->value;

        RBTreeNode *left = realAddr(iterNode->left);
        RBTreeNode *right = realAddr(iterNode->right);
        if(left != nilNode)
            iterNode = realAddr(iterNode->left);
        else if(right != nilNode)
            iterNode = realAddr(iterNode->right);
        else
        {
            while(iterNode != rootNode && realAddr(realAddr(iterNode->parent)->right) == iterNode)
                iterNode = realAddr(iterNode->parent);
            if(iterNode != rootNode)
                iterNode = realAddr(realAddr(iterNode->parent)->right);
        }
        return true;
	}

protected:
    smsc::logger::Logger* logger;
	RBTreeAllocator<Key, Value>*	allocator;
	RBTreeChangesObserver<Key, Value>*	changesObserver;
	RBTreeNode*		rootNode;
	RBTreeNode*		nilNode;
    RBTreeNode*     iterNode;
	long			offset;
	long			size;
	bool			defaultAllocator;
	bool			defaultChangesObserver;
inline
	RBTreeNode* realAddr(RBTreeNode* node)
	{
		return (RBTreeNode*)((long)node + allocator->getOffset());
	}
inline
	RBTreeNode* relativeAddr(RBTreeNode* node)
	{
		return (RBTreeNode*)((long)node - allocator->getOffset());
	}
inline
	RBTreeNode* set(RBTreeNode** to, RBTreeNode* from)
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

