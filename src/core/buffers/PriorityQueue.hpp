#ifndef __SMSC_CORE_BUFFERS_PRIORITY_QUEUE_HPP__
#define __SMSC_CORE_BUFFERS_PRIORITY_QUEUE_HPP__

#include "Array.hpp"

namespace smsc{
namespace core{
namespace buffers{

template <class T,class C,int min,int max>
class PriorityQueue{
public:
  PriorityQueue()
  {
    queue.Init(max-min+1);
    count=0;
    minuse=0;
    maxuse=0;
    curidx=0;
    curcnt=0;
  }
  ~PriorityQueue()
  {
  }

  PriorityQueue& Push(const T& item,int priority)
  {
    if(priority<min)priority=min;
    if(priority>max)priority=max;
    int idx=priority-min;
    if(count)
    {
      if(idx<minuse)minuse=idx;
      if(idx>maxuse)maxuse=idx;
    }else
    {
      minuse=maxuse=idx;
      curidx=idx;
      curcnt=0;
    }
    queue[idx].Push(item);
    count++;
    return *this;
  }

  bool Peek(T& item)
  {
    if(count==0)return false;
    item=queue[curidx].Front();
    return true;
  }

  bool Pop(T& item)
  {
    if(count==0)return false;
    queue[curidx].Pop(item);
    count--;
    curcnt++;
    if(count>0)
    {
      while(queue[minuse].Count()==0)minuse++;
      while(queue[maxuse].Count()==0)maxuse--;
      if(curcnt>curidx || queue[curidx].Count()==0)
      {
        curidx++;
        if(curidx>maxuse)curidx=minuse;
        while(queue[curidx].Count()==0)
        {
          curidx++;
        }
        curcnt=0;
      }
    }
    return true;
  }
  int Count(){return count;}
protected:
  Array<C> queue;
  int minuse,maxuse;
  int count;
  int curidx;
  int curcnt;
};

}//buffers
}//core
}//smsc

#endif
