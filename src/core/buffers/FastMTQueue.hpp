#ifndef __SMSC_CORE_BUFFERS_FASTMTQUEUE_HPP__
#define __SMSC_CORE_BUFFERS_FASTMTQUEUE_HPP__
#include "CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include <algorithm>

namespace smsc{
namespace core{
namespace buffers{

using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::MutexGuard;

template <class T,class Q=CyclicQueue<T> >
class FastMTQueue{
public:
  FastMTQueue()
  {
    wr=new Q;
    rd=new Q;
  }
  FastMTQueue(const FastMTQueue& that)
  {
    MutexGuard g(that.mtx);
    wr=new Q(*that.wr);
    rd=new Q(*that.rd);
  }
  ~FastMTQueue()
  {
    delete wr;
    wr=0;
    delete rd;
    rd=0;
  }

  void Push(const T& item)
  {
    MutexGuard g(mtx);
    wr->Push(item);
    mtx.notify();
  }

  bool Pop(T& item)
  {
    if(rd->Count()>0)
    {
      rd->Pop(item);
      return true;
    }
    {
      MutexGuard g(mtx);
      std::swap(rd,wr);
    }
    if(rd->Count()==0)return false;
    rd->Pop(item);
    return true;
  }

  void Wait()const
  {
    MutexGuard g(mtx);
    mtx.wait();
  }

  int Count()const
  {
    MutexGuard g(mtx);
    return wr->Count();
  }

protected:
  Q *wr,*rd;
  mutable EventMonitor mtx;
};

}
}
}

#endif //__SMSC_CORE_BUFFERS_FASTMTQUEUE_HPP__
