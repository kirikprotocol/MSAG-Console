//------------------------------------
//  RBTreee.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	‘айл содержит описание класса RBTreee.
//

#include "RBTreeAllocator.h"


#ifndef ___RBTREE_H
#define ___RBTREE_H


const char RED = 0;
const char BLACK = 1;

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

template<class Key=long, class Value=long, class DefaultAllocator = SimpleAllocator<Key, Value> >
class RBTree
{
	typedef templRBTreeNode<Key, Value> RBTreeNode;
public:
	RBTree(RBTreeAllocator<Key, Value>* _allocator=0):defaultAllocator(false)
	{
		if(!_allocator)
		{
			_allocator = new DefaultAllocator;
			defaultAllocator = true;
		}
		allocator = _allocator;
		rootNode = allocator->getRootNode();
		nilNode = allocator->getNilNode();
		offset = allocator->getOffset();
		size = allocator->getSize();
	}
	virtual ~RBTree()
	{
		if(defaultAllocator)
			delete allocator;
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
	const RBTreeAllocator<Key, Value>* SetAllocator(void)
	{
		return allocator;
	}

	int Insert(const Key& k, const Value& v)
	{
		RBTreeNode* newNode = allocator->allocateNode();
		newNode->key = k;
		newNode->value = v;
		bstInsert(newNode);
		newNode->color = RED;
		rbtRecovery(newNode);
		return 1;
	}
	bool Get(const Key& k, Value& val)
	{
		RBTreeNode*	node = rootNode;
		while( (node != nilNode) && (node->key != k) )
		{
			if(node->key < k)
				node = realAddr(node->right);
			else
				node = realAddr(node->left);
		}
		
		if(node == nilNode) return false;
		val = node->value;
		return true;
	}

protected:
	RBTreeAllocator<Key, Value>*	allocator;
	RBTreeNode*		rootNode;
	RBTreeNode*		nilNode;
	long			offset;
	long			size;
	bool			defaultAllocator;

inline
	RBTreeNode* realAddr(RBTreeNode* node)
	{
		return (RBTreeNode*)((long)node + offset);
	}
inline
	RBTreeNode* relativeAddr(RBTreeNode* node)
	{
		return (RBTreeNode*)((long)node - offset);
	}
	
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	
	//		¬се переменные указатели на RBTreeNode содержат абсолютные указатели, 
	//	тогда как пол€ объектов RBTreeNode - это указатели внутри области хранилища,
	//	т.е. смещенные относительно абсолютных на offset.
	//
	void rbtRotateLeft(RBTreeNode* node)
	{
		RBTreeNode*	tempNode = realAddr(node->right);	//node->right;
		node->right = tempNode->left;

		if(realAddr(tempNode->left) != nilNode)
			realAddr(tempNode->left)->parent = relativeAddr(node); //tempNode->left->parent = node;
		tempNode->parent = node->parent;

		if(realAddr(node->parent) == nilNode)
		{
			rootNode = tempNode;
			allocator->setRootNode(rootNode);
		}
		else
			if( node == realAddr(realAddr(node->parent)->left) ) //node == node->parent->left)
				realAddr(node->parent)->left = relativeAddr(tempNode); //node->parent->left = tempNode;
			else
				realAddr(node->parent)->right = relativeAddr(tempNode); //node->parent->right = tempNode;
		tempNode->left = relativeAddr(node);
		node->parent = relativeAddr(tempNode);
	}
	void rbtRotateRight(RBTreeNode* node)
	{
		RBTreeNode*	tempNode = realAddr(node->left);
		node->left = tempNode->right;

		if(realAddr(tempNode->right) != nilNode)
			realAddr(tempNode->right)->parent = relativeAddr(node);
		tempNode->parent = node->parent;

		if(realAddr(node->parent) == nilNode)
		{
			rootNode = tempNode;
			allocator->setRootNode(rootNode);
		}
		else
			if(node == realAddr(realAddr(node->parent)->right) ) //node == node->parent->right
				realAddr(node->parent)->right = relativeAddr(tempNode); //node->parent->right = tempNode;
			else
				realAddr(node->parent)->left = relativeAddr(tempNode); //node->parent->left = tempNode;
		tempNode->right = relativeAddr(node);
		node->parent = relativeAddr(tempNode);
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
		if(parentNode == nilNode )
		{
			rootNode = newNode;
			allocator->setRootNode(rootNode);
		}
		else
			if(newNode->key < parentNode->key)
				parentNode->left = relativeAddr(newNode);
			else
				parentNode->right = relativeAddr(newNode);
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
				}
			}
			else
			{
				tempNode = realAddr(realAddr(realAddr(node->parent)->parent)->left);
				if(tempNode->color == RED) //if(tempNode && (tempNode->color == RED))
				{
					realAddr(node->parent)->color = BLACK;
					tempNode->color = BLACK;
					realAddr(realAddr(node->parent)->parent)->color = RED;
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
				}
			}
		}
		rootNode->color = BLACK;
		return 1;
	}
};


#endif

