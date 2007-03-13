#ifndef __SMSC_CORE_BUFFERS_FRONTMTQUEUE_HPP__
#define __SMSC_CORE_BUFFERS_FRONTMTQUEUE_HPP__
//==================================================
// ����������� �� FastMTQueue 
// + ��������� ������� Front, ������������ ������ ������� ��� �������� ��� �� �������
//   � ������� WaitTime, ����������� �������� ������������ �� �������
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

  // ����� �� ���������� FastMTQueue::Wait() (�.e. ��� ���������� �����)
  // ������� �� �������
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
