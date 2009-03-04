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
      return rd->Pop(item);
    }
    {
      MutexGuard g(mtx);
      std::swap(rd,wr);
    }
    if(rd->Count()==0)return false;
    return rd->Pop(item);
  }

    /// useful to wait for items (or notification) to arrive, prior to call to Pop.
    /// This method is better than: Pop+Wait as Wait only waits for monitor notification
    /// but it may come when mutex is unlocked.
    void waitForItem()
    {
        if (rd->Count() > 0) return;
        MutexGuard mg(mtx);
        if (wr->Count() == 0) mtx.wait();
        std::swap(rd,wr);
    }

  void Wait()const
  {
    MutexGuard g(mtx);
    mtx.wait();
  }

  // useful when stopping
  void notify() {
    MutexGuard g(mtx);
    mtx.notify();
  }

  int Count()const
  {
    MutexGuard g(mtx);
    return wr->Count();
  }

    /// items count evaluation w/o locking.
    /// NOTE: it is not accurate!
    int evaluateCount() const
    {
        return wr->Count() + rd->Count();
    }
    
protected:
  Q *wr,*rd;
  mutable EventMonitor mtx;
};

}
}
}

#endif //__SMSC_CORE_BUFFERS_FASTMTQUEUE_HPP__
