#ifndef __SMSC_CORE_BUFFERS_PRIORITY_QUEUE_HPP__
#define __SMSC_CORE_BUFFERS_PRIORITY_QUEUE_HPP__

#include "core/buffers/Array.hpp"
#include "util/debug.h"

namespace smsc{
namespace core{
namespace buffers{

template <class T,class C,int min,int max>
class PriorityQueue{
public:
  PriorityQueue()
  {
    counts.Init(max-min+1);
    queue.Init(max-min+1);
    for(int i=0;i<=max-min;i++)
    {
      counts[i]=0;
    }
    count=0;
    minuse=0;
    maxuse=0;
    processed=0;
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
    }
    __require__(minuse>=0 && minuse<=max-min+1 && maxuse>=0 && maxuse<=max-min+1);
    //__trace2__("PQ: push - idx=%d, prio=%d, count=%d, minuse=%d, maxuse=%d",idx,priority,count,minuse,maxuse);
    queue[idx].Push(item);
    count++;
    return *this;
  }

  bool Pop(T& item)
  {
    if(count==0)return false;
    __require__(minuse>=0 && minuse<=max-min+1 && maxuse>=0 && maxuse<=max-min+1);
    //__trace2__("PQ: enter pop - minuse=%d, maxuse=%d",minuse,maxuse);
    int best=minuse,i;
    double bestval=(double)counts[minuse]/((minuse+min)>0?(minuse+min):1);

    for(i=minuse+1;i<=maxuse;i++)
    {
      if(queue[i].Count()==0)
      {
        continue;
      }
      double val=(double)counts[i]/((i+min)>0?(i+min):1);
      if(val<bestval)
      {
        best=i;
        bestval=val;
      }
    }
    //__trace2__("PQ: pop - best=%d",best);
    counts[best]++;
    //if(counts[best]>=best)counts[best]=0;
    queue[best].Shift(item);
    count--;
    if(count>0)
    {
      while(queue[minuse].Count()==0)minuse++;
      while(queue[maxuse].Count()==0)maxuse--;
    }
    __require__(minuse>=0 && minuse<=max-min+1 && maxuse>=0 && maxuse<=max-min+1);
    //__trace2__("PQ: enter pop - count=%d, minuse=%d, maxuse=%d",count, minuse,maxuse);
    processed++;
    if(processed>=(max-min)*100)
    {
      for(i=0;i<=max-min;i++)
      {
        counts[i]=0;
      }
      processed=0;
    }
    return true;
  }
  int Count(){return count;}
protected:
  Array<C> queue;
  Array<int> counts;
  int minuse,maxuse;
  int count;
  int processed;
};

};//buffers
};//core
};//smsc

#endif
