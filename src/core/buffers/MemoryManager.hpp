#ifndef __Core_Buffers_MemoryManager_hpp__
#define __Core_Buffers_MemoryManager_hpp__


#include "Array.hpp"
#include "core/synchronization/Mutex.hpp"
#include "util/debug.h"

namespace smsc{
namespace core{
namespace buffers{

#define MM_MAX_CHECK_POINTS 64
#define MM_MAX_TASK_NAME_SIZE 64
#define MM_DEFAULT_PREALLOCATED_HEAPS 2
#define MM_DEFAULT_RAWHEAP_SIZE 4096
#define MM_BLOCKS_HEAP_QUANTUM 32
#define MM_MAX_PREALLOC_BLOCK_SIZE 1024
#define MM_PER_POOL_BLOCKS 16

#define MM_BLOCK_HEAP_MAGIC 0x28611adbL
#define MM_RAW_HEAP_MAGIC 0xd82b3229L

class MemoryManager;

class MemoryHeap{
public:
  MemoryHeap(const char* taskname,MemoryManager *parentmanager);
  MemoryHeap(const char* taskname,MemoryManager *parentmanager,
             int rawheapsize,int blocksheapquantum);

  ~MemoryHeap();

  void initHeap(int rawheapsize,int blocksheapquantum,int maxblocksize,
               int blocksperpool);

  void cleanHeap();

  int selectHeap(const char* taskname,MemoryManager *parentmanager,
                 int rawheapsize,int blocksheapquantum);

  template <class T>
  void getRawMem(int size,T*& mem)
  {
    mem=(T*)getRawMem(size);
  }

  void* getRawMem(int size);
  int setCheckPoint();
  void doRollBack();
  void resetRawHeap();

  void* getMem(int size);

  template <class T>
  void getMem(int size,T*& mem)
  {
    mem=(T*)getMem(size);
  }

  template <class T>
  void freeMem(T*& block)
  {
    freeMem((void*&)block);
  }

  void freeMem(void*& block);

  void releaseHeap();


private:
  //general things
  MemoryManager *parentManager;
  char taskName[MM_MAX_TASK_NAME_SIZE];

  //Raw Heap staff
  int rawHeapPageSize;
  typedef struct RawHeapPage{
    int pageSize;
    int pageUsage;
    char* memPage;
    RawHeapPage *next;
  } *PRawHeapPage;
  PRawHeapPage firstRawPage;
  PRawHeapPage lastRawPage;

  struct RawHeapCheckPoint{
    PRawHeapPage page;
    int pageUsage;
  };

  RawHeapCheckPoint cpStack[MM_MAX_CHECK_POINTS];
  int checkPointsCount;

  //Blocks heap staff
  int blocksHeapQuantum;
  int blocksHeapQuantumShift;
  int blocksHeapQuantumMask;
  int blocksMaxSize;
  int blocksPerPool;

  typedef Array<char*> MemArray;
  struct BlocksHeapPooledPage{
    MemArray freeBlocks;
    MemArray usedBlocks;
  };
  struct BlocksHeapVarPage{
    BlocksHeapVarPage(int size):blocksize(size){}
    int blocksize;
    MemArray freeBlocks;
    MemArray usedBlocks;
  };
  typedef Array<BlocksHeapPooledPage*> BlocksArray;
  typedef Array<BlocksHeapVarPage*> VarBlocksArray;
  BlocksArray blocksPool;
  VarBlocksArray varBlocks;
}; // MemoryHeap

class MemoryManager{
public:
  MemoryManager();
  ~MemoryManager();

  void preallocateHeaps(int count);

  MemoryHeap* acquireHeap(const char* taskname,int rawheapsize,int blocksheapquantum);
  void releaseHeap(MemoryHeap* heap);
private:
  typedef Array<MemoryHeap*> HeapArray;
  HeapArray usedHeaps;
  HeapArray freeHeaps;

  smsc::core::synchronization::Mutex mutex;

  void Lock()
  {
    mutex.Lock();
  }
  void Unlock()
  {
    mutex.Unlock();
  }
};//MemoryManager

};//buffers
};//core
};//smsc

#endif
