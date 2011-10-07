#ifndef __SMSC_CORE_BUFFERS_INTRLIST_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_CORE_BUFFERS_INTRLIST_HPP__

#include <sys/types.h>

namespace smsc{
namespace core{
namespace buffers{


struct IntrListEmptyStruct{};

template <class T,class Base=IntrListEmptyStruct>
struct IntrListNodeBase : public Base {
  typedef Base IntrListNodeBaseParent;
  T* ilPrevNode;
  T* ilNextNode;
};

template <class T /* : public IntrListNodeBase<T> */>
class IntrList{
public:
  typedef T* value_type;
  IntrList():count(0)
  {
    endNode.ilPrevNode=endPtr();
    endNode.ilNextNode=endPtr();
  }
  void clear()
  {
    endNode.ilPrevNode=endPtr();
    endNode.ilNextNode=endPtr();
    count=0;
  }

  T* front()
  {
    return endNode.ilNextNode;
  }
  const T* front() const
  {
    return endNode.ilNextNode;
  }

  T* back()
  {
    return endNode.ilPrevNode;
  }
  const T* back() const
  {
    return endNode.ilPrevNode;
  }

  class iterator{
  public:
      typedef ptrdiff_t                        difference_type;
      typedef std::bidirectional_iterator_tag  iterator_category;
      typedef T                                value_type;
      typedef T*                               pointer;
      typedef T&                               reference;

    iterator() : node(0)
    { }
    ~iterator()
    { }

    iterator& operator=(const iterator& argOther)
    {
      node=argOther.node;
      return *this;
    }
    bool operator==(const iterator& argOther)const
    {
      return node==argOther.node;
    }
    bool operator!=(const iterator& argOther)const
    {
      return node!=argOther.node;
    }
    iterator operator++(int)
    {
      T* rv=node;
      node=node->ilNextNode;
      return iterator(rv);
    }
    iterator operator++()
    {
      node=node->ilNextNode;
      return iterator(node);
    }
    iterator operator--(int)
    {
      T* rv=node;
      node=node->ilPrevNode;
      return iterator(rv);
    }
    iterator operator--()
    {
      node=node->ilPrevNode;
      return iterator(node);
    }

    T& operator*()
    {
      return *node;
    }
    T* operator->()
    {
      return node;
    }
    const T& operator*()const
    {
      return *node;
    }
    const T* operator->()const
    {
      return node;
    }

  protected:
    T* node;

    friend class IntrList;
    explicit iterator(T* argNode) : node(argNode)
    { }
  };

  class const_iterator{
  public:
    const_iterator() : node(0)
    { }
    const_iterator(const iterator & use_it) : node(use_it.node)
    {}
    ~const_iterator()
    { }

    const_iterator& operator=(const iterator& argOther)
    {
      node=argOther.node;
      return *this;
    }

    const_iterator& operator=(const const_iterator& argOther)
    {
      node=argOther.node;
      return *this;
    }
    bool operator==(const const_iterator& argOther)const
    {
      return node==argOther.node;
    }
    bool operator!=(const const_iterator& argOther)const
    {
      return node!=argOther.node;
    }
    const_iterator operator++(int)
    {
      const T* rv=node;
      node=node->ilNextNode;
      return const_iterator(rv);
    }
    const_iterator operator++()
    {
      node=node->ilNextNode;
      return const_iterator(node);
    }

    const_iterator operator--(int)
    {
      T* rv=node;
      node=node->ilPrevNode;
      return const_iterator(rv);
    }
    const_iterator operator--()
    {
      node=node->ilPrevNode;
      return const_iterator(node);
    }

    const T& operator*()const
    {
      return *node;
    }
    const T* operator->()const
    {
      return node;
    }

  protected:
    const T* node;

    friend class IntrList;
    explicit const_iterator(const T* argNode) : node(argNode)
    { }
  };


  iterator begin()
  {
    return iterator(endNode.ilNextNode);
  }

  iterator end()
  {
    return iterator(endPtr());
  }

  const_iterator begin()const
  {
    return const_iterator(endNode.ilNextNode);
  }

  const_iterator end()const
  {
    return const_iterator(endPtr());
  }

  iterator insert(iterator pos,T* value)
  {
    if(pos==begin())
    {
      push_front(value);
    }else if(pos==end())
    {
      push_back(value);
    }else
    {
      T* prev=pos->ilPrevNode;
      prev->ilNextNode=value;
      pos->ilPrevNode=value;
      value->ilNextNode=&*pos;
      value->ilPrevNode=prev;
      count++;
    }
    return iterator(value);
  }

  void push_back(T* value)
  {
    if(!count)
    {
      value->ilNextNode=endPtr();
      value->ilPrevNode=endPtr();
      endNode.ilPrevNode=value;
      endNode.ilNextNode=value;
      count=1;
      return;
    }
    endNode.ilPrevNode->ilNextNode=value;
    value->ilNextNode=endPtr();
    value->ilPrevNode=endNode.ilPrevNode;
    endNode.ilPrevNode=value;
    count++;
  }

  void push_front(T* value)
  {
    if(!count)
    {
      push_back(value);
      return;
    }
    value->ilPrevNode=endPtr();
    value->ilNextNode=endNode.ilNextNode;
    endNode.ilNextNode->ilPrevNode=value;
    endNode.ilNextNode=value;
    count++;
  }

  void pop_back()
  {
    if(!count)return;
    T* node=endNode.ilPrevNode;
    if(count>1)
    {
      node->ilPrevNode->ilNextNode=node->ilNextNode;
      node->ilNextNode->ilPrevNode=node->ilPrevNode;
      count--;
    }else
    {
      clear();
    }
    node->ilNextNode = node->ilPrevNode = 0;
  }

  void pop_front()
  {
    if(!count)return;
    T * frontNode = endNode.ilNextNode;
    endNode.ilNextNode->ilNextNode->ilPrevNode=endPtr();
    endNode.ilNextNode=endNode.ilNextNode->ilNextNode;
    frontNode->ilNextNode = frontNode->ilPrevNode = 0;
    count--;
  }

  void erase(iterator it)
  {
    erase(it.node);
  }
  void erase(T* value)
  {
    value->ilPrevNode->ilNextNode=value->ilNextNode;
    value->ilNextNode->ilPrevNode=value->ilPrevNode;
    value->ilPrevNode = value->ilNextNode = 0;
    count--;
  }

  void join(IntrList<T>& argOther)
  {
    if(!argOther.count)
    {
      return;
    }
    if(count)
    {
      endNode.ilPrevNode->ilNextNode=argOther.endNode.ilNextNode;
      argOther.endNode.ilNextNode->ilPrevNode=endNode.ilPrevNode;
      endNode.ilPrevNode=argOther.endNode.ilPrevNode;
      endNode.ilPrevNode->ilNextNode=endPtr();
      count+=argOther.count;
    }else
    {
      endNode.ilPrevNode=argOther.endNode.ilPrevNode;
      endNode.ilPrevNode->ilNextNode=endPtr();
      endNode.ilNextNode=argOther.endNode.ilNextNode;
      endNode.ilNextNode->ilPrevNode=endPtr();
      count=argOther.count;
    }
    argOther.clear();
  }

  size_t size()const
  {
    return count;
  }

  bool empty()const
  {
    return count==0;
  }


protected:
  struct EndNode:IntrListNodeBase<T,typename T::IntrListNodeBaseParent>{
  };
  EndNode endNode;
  T* endPtr()
  {
    return (T*)&endNode;
  }
  const T* endPtr()const
  {
    return (const T*)&endNode;
  }
  typedef T Node;
  size_t count;
};

}
}
}


#endif /* __SMSC_CORE_BUFFERS_INTRLIST_HPP__ */

