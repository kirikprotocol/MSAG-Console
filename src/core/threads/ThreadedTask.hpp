#ifndef __CORE_THREADS_THREADEDTASK_HPP__
#define __CORE_THREADS_THREADEDTASK_HPP__
#ident "$Id$"

#include "core/buffers/MemoryManager.hpp"

namespace smsc{
namespace core{
namespace threads{

using namespace smsc::core::buffers;

class ThreadedTask {
public:
  ThreadedTask():/*heap(NULL),*/isStopping(false), isReleased(false), delTask(true) {}
  virtual ~ThreadedTask(){}

  //set task completion mode: deleting or callback calling
  void setDelOnCompletion(bool del_task = true) { delTask = del_task; }
  //tells whether the task destructor or onRelease() callback should be called on completion
  bool delOnCompletion(void) const { return delTask; }

  virtual int Execute()=0;
  virtual const char* taskName()=0;

  /*

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
  */

  virtual void stop(){isStopping=true;}
  virtual void onRelease(void) { isReleased = true; }

  inline bool stopping() const { return isStopping; }

protected:
  //smsc::core::buffers::MemoryHeap *heap;
  volatile bool isStopping;
  volatile bool isReleased;

private:
  bool          delTask; //indicates that task should be deleted on completion
};//ThreadedTask

}//threads
}//core
}//smsc

#endif
