#ifndef __CORE_THREADS_THREADEDTASK_HPP__
#define __CORE_THREADS_THREADEDTASK_HPP__

#include "core/buffers/MemoryManager.hpp"

namespace smsc{
namespace core{
namespace threads{

using namespace smsc::core::buffers;

class ThreadedTask{
public:
  ThreadedTask():heap(NULL),isStopping(false){}
  virtual ~ThreadedTask(){}
  virtual int Execute()=0;
  virtual const char* taskName()=0;
  virtual void getMemoryInfo(int& rawheapsize,int& blocksheapquantum);
  void assignHeap(MemoryHeap* newheap)
  {
    heap=newheap;
  }
  void releaseHeap()
  {
    if(heap)heap->releaseHeap();
  }

  void *getRawMem(int size)
  {
    return heap->getRawMem(size);
  }

  template <class T>
  void getRawMem(int size,T*& ptr)
  {
    ptr=(T*)getRawMem(size);
  }

  int setCheckPoint()
  {
    return heap->setCheckPoint();
  }
  void doRollBack()
  {
    heap->doRollBack();
  }


  void* getMem(int size)
  {
    return heap->getMem(size);
  }

  template <class T>
  void getMem(int size,T*& ptr)
  {
    ptr=(T*)getMem(size);
  }

  void freeMem(void*& ptr)
  {
    heap->freeMem(ptr);
  }

  void stop(){isStopping=true;}

protected:
  smsc::core::buffers::MemoryHeap *heap;
  bool isStopping;
};//ThreadedTask

};//threads
};//core
};//smsc

#endif
