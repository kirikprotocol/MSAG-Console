#ifndef __Core_Buffers_MemoryManager_hpp__
#define __Core_Buffers_MemoryManager_hpp__


#include "Array.hpp"
#include "core/synchronization/Mutex.hpp"

namespace smsc{
namespace core{
namespace buffers{

//#include "Hash.hpp"
//#include "List.hpp"

class MemoryManager;

#define MM_MAX_CHECK_POINTS 64
#define MM_MAX_TASK_NAME_SIZE 64
#define MM_DEFAULT_PREALLOCATED_HEAPS 32
#define MM_DEFAULT_RAWHEAP_SIZE 4096
#define MM_BLOCKS_HEAP_QUANTUM 32
#define MM_MAX_PREALLOC_BLOCK_SIZE 1024
#define MM_PER_POOL_BLOCKS 16

#define MM_BLOCK_HEAP_MAGIC 0x28611adbL
#define MM_RAW_HEAP_MAGIC 0xd82b3229L

class MemoryHeap{
public:
  MemoryHeap(char* taskname,int rawheapsize,int blocksheapquantum,
             MemoryManager *parentmanager);

  ~MemoryHeap();

  int initHeap(int rawheapsize,int blocksheapquantum,int maxblocksize,
               int blocksperpool);

  int cleanHeap();

  int selectHeap(char* taskname,int rawheapsize,int blocksheapquantum,
                 MemoryManager *parentmanager);

  void* getRawMem(int size);
  int setCheckPoint();
  void doRollBack();
  void resetRawHeap();

  void* getMem(int size);
  void freeMem(void* block);


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
  int blocksMaxSize;
  int blocksPerPool;

  typedef Array<char*> MemArray;
  struct BlocksHeapPooledPage{
    //char *firstpage;
    MemArray freeBlocks;
    MemArray usedBlocks;
  };
  struct BlocksHeapVarPage{
    int blocksize;
    MemArray freeBlocks;
    MemArray usedBlocks;
  };
  typedef Array<BlocksHeapPooledPage*> BlocksArray;
  typedef Array<BlocksHeapVarPage*> VarBlocksArray;
  BlocksArray blocksPool;
  VarBlocksArray varBlocks;
};

class MemoryManager{
public:
  MemoryManager();

  void preallocateHeaps(int count);

  MemoryHeap* acquireHeap(char* taskname,int rawheapsize,int blocksheapquantum);
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
};

};//buffers
};//core
};//smsc

#endif
