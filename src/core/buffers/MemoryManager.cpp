#include "MemoryManager.hpp"

#include <string.h>

namespace smsc{
namespace core{
namespace buffers{

/****************************************************************************
 *
 * MemoryManager
 */

/****************************************************************************
 * Default Constructor
 * Содаёт дефолтное кол-во свободных хипов
*/

MemoryManager::MemoryManager()
{
  preallocateHeaps(MM_DEFAULT_PREALLOCATED_HEAPS);
}

/****************************************************************************
 * Гарантирует наличие count свободных хипов
 *
 */

void MemoryManager::preallocateHeaps(int count)
{
  int i;
  freeHeaps.SetSize(count);
  for(i=freeHeaps.Count();i<count;i++)
  {
    freeHeaps[i]=new MemoryHeap(NULL,MM_DEFAULT_RAWHEAP_SIZE,MM_MAX_PREALLOC_BLOCK_SIZE,this);
  }
}

/****************************************************************************
 * Выделяет свободный хип под таск с именем taskname,
 * и размером страницы линейного хипа максимально близким к rawheapsize.
 * Возвращает указатель на готовый к использованию объект.
 */

MemoryHeap*
MemoryManager::acquireHeap(char* taskname,int rawheapsize,int blocksheapquantum)
{
  int i;
  MemoryHeap *retval;
  Lock();
  for(i=0;i<freeHeaps.Count();i++)
  {
    if(freeHeaps[i]->selectHeap(taskname,rawheapsize,blocksheapquantum,this))
    {
      retval=freeHeaps[i];
      usedHeaps.Push(freeHeaps[i]);
      freeHeaps.Delete(i);
      Unlock();
      return retval;
    }
  }
  retval=new MemoryHeap(taskname,rawheapsize,blocksheapquantum,this);
  usedHeaps.Push(retval);
  Unlock();
  return retval;
}

/****************************************************************************
 * Освобождает хип.
 *
 */

void MemoryManager::releaseHeap(MemoryHeap *heap)
{
  int i;
  Lock();
  for(i=0;i<usedHeaps.Count();i++)
  {
    if(usedHeaps[i]==heap)
    {
      freeHeaps.Push(usedHeaps[i]);
      usedHeaps.Delete(i);
    }
  }
  Unlock();
}

/****************************************************************************
 *
 * MemoryHeap
 */

/****************************************************************************
 * Constructor
 * Создаёт heap для задачи с именем taskname, и задаёт размер
 * страницы линейного хипа.
 */

MemoryHeap::MemoryHeap(char* taskname,
                       int rawheapsize,
                       int blocksheapquantum,
                       MemoryManager *parentmanager)
{
  memset(this,0,sizeof(*this));
  taskName[sizeof(taskName)-1]=0;
  strncpy(taskName,taskname,sizeof(taskName));
  parentManager=parentmanager;
  initHeap(rawheapsize,
           blocksheapquantum,
           MM_MAX_PREALLOC_BLOCK_SIZE,
           MM_PER_POOL_BLOCKS);
}

int MemoryHeap::initHeap(int rawheapsize,
                         int blocksheapquantum,
                         int maxblocksize,
                         int blocksperpool)
{
  /* Init raw heap */
  rawHeapPageSize=rawheapsize;
  char *rawheap=new char[rawHeapPageSize+sizeof(RawHeapPage)];
  firstRawPage=lastRawPage=(PRawHeapPage)rawheap;
  firstRawPage->pageSize=rawHeapPageSize;
  firstRawPage->pageUsage=0;
  firstRawPage->memPage=rawheap+sizeof(RawHeapPage);
  firstRawPage->next=NULL;
  blocksHeapQuantum=blocksheapquantum;
  blocksMaxSize=maxblocksize;
  blocksPerPool=blocksperpool;

  /* init blocks heap */
  int blocksize=blocksHeapQuantum;
  int n=blocksMaxSize/blocksHeapQuantum;
  int i,j;
  char *ptr;
  int *iptr;
  blocksPool.SetSize(n);
  for(i=0;i<n;blocksize+=blocksHeapQuantum,i++)
  {
    blocksPool[i]=new BlocksHeapPooledPage;
    blocksPool[i]->freeBlocks.SetSize(blocksPerPool);
    for(j=0;j<blocksPerPool;j++)
    {
      ptr=new char[blocksize+sizeof(int)*3];
      blocksPool[i]->freeBlocks[j]=ptr+sizeof(int)*3;
      iptr=(int*)ptr;
      iptr[0]=blocksize;
      iptr[1]=-1;
      iptr[2]=MM_BLOCK_HEAP_MAGIC;
    }
  }
}

int MemoryHeap::cleanHeap()
{
  PRawHeapPage p=firstRawPage,q;
  while(p)
  {
    q=p->next;
    delete [] p->memPage;
    p=q;
  }
  checkPointsCount=0;
  int i,j;
  for(i=0;i<blocksPool.Count();i++)
  {
    for(j=0;j<blocksPool[i]->freeBlocks.Count();j++)
    {
      // first 3 ints of block filled with
      // [0] - blocksize
      // [1] - index of block
      // [2] - block magic
      delete [] (blocksPool[i]->freeBlocks[j]-sizeof(int)*3);
    }
    blocksPool[i]->freeBlocks.Clean();
    for(j=0;j<blocksPool[i]->usedBlocks.Count();j++)
    {
      delete [] (blocksPool[i]->usedBlocks[j]-sizeof(int)*3);
    }
    blocksPool[i]->usedBlocks.Clean();
    delete blocksPool[i];
  }
  blocksPool.Clean();
  for(i=0;i<varBlocks.Count();i++)
  {
    for(j=0;j<varBlocks[i]->freeBlocks.Count();j++)
    {
      delete [] (varBlocks[i]->freeBlocks[j]-sizeof(int)*3);
    }
    varBlocks[i]->freeBlocks.Clean();
    for(j=0;j<varBlocks[i]->usedBlocks.Count();j++)
    {
      delete [] (varBlocks[i]->usedBlocks[j]-sizeof(int)*3);
    }
    varBlocks[i]->usedBlocks.Clean();
    delete varBlocks[i];
  }
  varBlocks.Clean();
}


void* MemoryHeap::getRawMem(int size)
{
  void *retval;
  int *iptr;
  int havespace;
  for(;;)
  {
    havespace=lastRawPage->pageSize-lastRawPage->pageUsage>size+sizeof(int);
    if(!havespace && lastRawPage->next)
    {
      lastRawPage=lastRawPage->next;
      continue;
    }
    break;
  }
  if(havespace)
  {
    retval=lastRawPage->memPage+sizeof(int);
    ((int*)lastRawPage->memPage)[0]=MM_RAW_HEAP_MAGIC;
    lastRawPage->memPage+=size+sizeof(int);
    lastRawPage->pageUsage+=size+sizeof(int);
  }else
  {
    PRawHeapPage p=new RawHeapPage;
    lastRawPage->next=p;
    lastRawPage=p;
    char *rawheap=new char[size+sizeof(int)+rawHeapPageSize+sizeof(RawHeapPage)];
    p=lastRawPage=(PRawHeapPage)rawheap;
    p->pageSize=size+sizeof(int)+rawHeapPageSize;
    p->pageUsage=size+sizeof(int);
    p->memPage=rawheap+sizeof(RawHeapPage);
    p->next=NULL;
    retval=p->memPage+sizeof(int);
    ((int*)p->memPage)[0]=MM_RAW_HEAP_MAGIC;
  }
  return retval;
}

int MemoryHeap::setCheckPoint()
{
  return 0;
}
void MemoryHeap::doRollBack()
{
}
void MemoryHeap::resetRawHeap()
{
}

void* MemoryHeap::getMem(int size)
{
  return NULL;
}
void MemoryHeap::freeMem(void* block)
{
}


};//buffers
};//core
};//smsc
