#ifndef __CYCLIC_QUEUE_HPP__
#define __CYCLIC_QUEUE_HPP__

#include <stdexcept>

namespace smsc{
namespace core{
namespace buffers{


template <class T>
class CyclicQueue{
public:
  explicit CyclicQueue(int prealloc=0)
  {
    if(prealloc>0)
    {
      data=new T[prealloc];
      size=prealloc;
      end=data+size;
    }else
    {
      data=0;
      size=0;
      end=0;
    }
    head=data;
    tail=data;
    count=0;
  }
  CyclicQueue(const CyclicQueue& src)
  {
    if(src.size==0)
    {
      data=0;
      end=0;
      head=0;
      tail=0;
      size=0;
      count=0;
    }else
    {
      data=new T[src.size];
      head=data;
      tail=data;
      size=src.size;
      end=data+size;
      count=0;
      T* ptr=src.tail;
      for(int i=0;i<src.count;i++)
      {
        Push(*ptr);
        ptr++;
        if(ptr==src.end)ptr=src.data;
      }
    }
  }
  ~CyclicQueue()
  {
    if(data)delete [] data;
  }
  void Push(const T& item)
  {
    if(count==size)Realloc(size*2);
    if(head==end)head=data;
    *head=item;
    head++;
    count++;
  }
  T& Front()
  {
    if(count==0)throw std::runtime_error("CQ: attempt to use Front() on empty queue");
    return *tail;
  }
  void Pop(T& item)
  {
    if(count==0)throw std::runtime_error("CQ: attempt to call Pop() on empty queue");
    item=*tail;
    tail++;
    if(tail==end)tail=data;
    count--;
  }
  void Pop()
  {
    if(count==0)throw std::runtime_error("CQ: attempt to call Pop() on empty queue");
    tail++;
    if(tail==end)tail=data;
    count--;
  }
  int Count()const{return count;}

protected:
  void Realloc(int sz)
  {
    if(sz==0)sz=16;
    T *newdata=new T[sz];
    T *ptr=tail;
    T *out=newdata;
    for(int i=0;i<count;i++)
    {
      *out=*ptr;
      ptr++;
      if(ptr==end)ptr=data;
      out++;
    }
    if(data)delete [] data;
    size=sz;
    data=newdata;
    end=data+size;
    tail=data;
    head=data+count;
  }
  int count,size;
  T* data;
  T* end;
  T* head;
  T* tail;
}; //class CyclicQueue

}//namespace buffers
}//namespace core
}//namespace smsc

#endif
