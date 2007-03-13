#ifndef __SMSC_CORE_BUFFERS_FRONTMTQUEUE_HPP__
#define __SMSC_CORE_BUFFERS_FRONTMTQUEUE_HPP__
//==================================================
// производная от FastMTQueue 
// + добавлена функция Front, возвращающая первый элемент без удаления его из очереди
//   и функция WaitTime, реализующая ожидание ограниченное по времени
//
#include "core/buffers/FastMTQueue.hpp"

namespace smsc{
namespace core{
namespace buffers{

using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::MutexGuard;
/**/

template <class T,class Q=CyclicQueue<T> >
class FrontMTQueue:  public FastMTQueue <T, Q>
{
  public:

    bool Front(T& value)
    {
         if(rd->Count()>0)
        {
            value = rd->Front();
            return true;
        }
        {
          MutexGuard g(mtx);
          std::swap(rd,wr);
        }
        if(rd->Count()==0) return false;
        value = rd->Front();
        return true;
   }

  void Notify()        {  mtx.notify();  }

  // чтобы не перебивать FastMTQueue::Wait() (т.e. для сохранения обеих)
  // названа по другому
  void WaitTime(int time)
    {
        MutexGuard g(mtx);
        mtx.wait(time);
    }
  
};
/**/
}
}
}

#endif //__SMSC_CORE_BUFFERS_FRONTMTQUEUE_HPP__
