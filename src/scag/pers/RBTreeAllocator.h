//------------------------------------
//  RBTreeAllocator.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	Файл содержит описание класса RBTreeAllocator.
//

#ifndef ___RBTREE_ALLOCATOR_H
#define ___RBTREE_ALLOCATOR_H


template<class Key=long, class Value=long>
class RBTreeAllocator
{
protected:
	typedef templRBTreeNode<Key, Value> RBTreeNode;
public:
	RBTreeAllocator(){}
	virtual ~RBTreeAllocator(){}
	virtual RBTreeNode* allocateNode(void) = 0;
	virtual void releaseNode(RBTreeNode* node) = 0;
	virtual RBTreeNode* getRootNode(void) = 0;
	virtual void setRootNode(RBTreeNode* node) = 0;
	virtual RBTreeNode* getNilNode(void) = 0;
	virtual long getSize(void) = 0;
	virtual long getOffset(void) = 0;
};

template<class Key=long, class Value=long>
class SimpleAllocator: public RBTreeAllocator<Key, Value>
{
	long count;
	RBTreeNode*	nilNode;
	RBTreeNode*	rootNode;
public:
	SimpleAllocator():count(0)
	{
		//create nil cell
		nilNode = new RBTreeNode();
		nilNode->parent = nilNode;
		nilNode->left = nilNode;
		nilNode->right = nilNode;
		nilNode->color = BLACK;
		rootNode = nilNode;
	}
	virtual ~SimpleAllocator()
	{
		delete nilNode;
	}
	virtual RBTreeNode* allocateNode(void)
	{
		RBTreeNode* newNode = new RBTreeNode();
		newNode->parent = newNode->left = newNode->right = nilNode;
		count++;
		return newNode;
	}
	virtual void releaseNode(RBTreeNode* node)
	{
		count--;
		delete node;
	}
	virtual RBTreeNode* getRootNode(void){return rootNode;}
	virtual void setRootNode(RBTreeNode* node){rootNode = node;}
	virtual RBTreeNode* getNilNode(void){return nilNode;};
	virtual long getSize(void){return count;};
	virtual long getOffset(void){return 0;}
};

#endif
