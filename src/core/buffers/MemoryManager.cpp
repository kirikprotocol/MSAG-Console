#include "MemoryManager.hpp"

#include <string.h>
#include <stdio.h>

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
    freeHeaps[i]=new MemoryHeap(NULL,this,MM_DEFAULT_RAWHEAP_SIZE,MM_MAX_PREALLOC_BLOCK_SIZE);
  }
}

/****************************************************************************
 * Выделяет свободный хип под таск с именем taskname,
 * и размером страницы линейного хипа максимально близким к rawheapsize.
 * Возвращает указатель на готовый к использованию объект.
 */

MemoryHeap*
MemoryManager::acquireHeap(const char* taskname,int rawheapsize,int blocksheapquantum)
{
  int i;
  MemoryHeap *retval;
  Lock();
  for(i=0;i<freeHeaps.Count();i++)
  {
    if(freeHeaps[i]->selectHeap(taskname,this,rawheapsize,blocksheapquantum))
    {
      retval=freeHeaps[i];
      usedHeaps.Push(freeHeaps[i]);
      freeHeaps.Delete(i,1);
      Unlock();
      return retval;
    }
  }
  retval=new MemoryHeap(taskname,this,rawheapsize,blocksheapquantum);
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
      usedHeaps.Delete(i,1);
    }
  }
  Unlock();
}

MemoryManager::~MemoryManager()
{
  int i;
  for(i=0;i<usedHeaps.Count();i++)
  {
    delete usedHeaps[i];
  }
  for(i=0;i<freeHeaps.Count();i++)
  {
    delete freeHeaps[i];
  }
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

MemoryHeap::MemoryHeap(const char* taskname,
                       MemoryManager *parentmanager,
                       int rawheapsize,
                       int blocksheapquantum)
{
  memset(this,0,sizeof(*this));
  if(taskname)
  {
    strncpy(taskName,taskname,sizeof(taskName));
    taskName[sizeof(taskName)-1]=0;
  }else
  {
    taskName[0]=0;
  }
  parentManager=parentmanager;
  initHeap(rawheapsize,
           blocksheapquantum,
           MM_MAX_PREALLOC_BLOCK_SIZE,
           MM_PER_POOL_BLOCKS);
}

MemoryHeap::MemoryHeap(const char* taskname,MemoryManager *parentmanager)
{
  memset(this,0,sizeof(*this));
  if(taskname)
  {
    strncpy(taskName,taskname,sizeof(taskName));
    taskName[sizeof(taskName)-1]=0;
  }else
  {
    taskName[0]=0;
  }
  parentManager=parentmanager;
}

MemoryHeap::~MemoryHeap()
{
  cleanHeap();
}

void MemoryHeap::initHeap(int rawheapsize,
                         int blocksheapquantum,
                         int maxblocksize,
                         int blocksperpool)
{
  /* Init raw heap */
  rawHeapPageSize=rawheapsize;
  if(rawheapsize)
  {
    char *rawheap=new char[rawHeapPageSize+sizeof(RawHeapPage)];
    firstRawPage=lastRawPage=(PRawHeapPage)rawheap;
    firstRawPage->pageSize=rawHeapPageSize;
    firstRawPage->pageUsage=0;
    firstRawPage->memPage=rawheap+sizeof(RawHeapPage);
    firstRawPage->next=NULL;
    blocksHeapQuantum=blocksheapquantum;
    blocksMaxSize=maxblocksize;
    blocksPerPool=blocksperpool;
  }

  /* init blocks heap */
  int blocksize=blocksHeapQuantum;
  int n=blocksMaxSize/blocksHeapQuantum;
  int i,j;

  blocksHeapQuantumShift=0;
  blocksHeapQuantumMask=0;
  j=blocksHeapQuantum;
  while(!((j&1)==1))
  {
    j>>=1;
    blocksHeapQuantumShift++;
    blocksHeapQuantumMask<<=1;
    blocksHeapQuantumMask|=1;
  }

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

void MemoryHeap::cleanHeap()
{
  PRawHeapPage p=firstRawPage,q;
  char *mem;
  while(p)
  {
    q=p->next;
    mem=p->memPage;
    __require__(mem);
    mem-=sizeof(RawHeapPage);
    delete [] mem;
    p=q;
  }
  lastRawPage=firstRawPage=0;
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

int MemoryHeap::selectHeap(const char* taskname,MemoryManager *parentmanager,
               int rawheapsize,int blocksheapquantum)
{
  if( rawheapsize>=rawHeapPageSize &&
      rawheapsize<rawHeapPageSize+rawHeapPageSize/2 &&
      blocksheapquantum==blocksHeapQuantum)
  {
    if(taskname)
    {
      strncpy(taskName,taskname,sizeof(taskName));
      taskName[sizeof(taskName)-1]=0;
    }else
    {
      taskName[0]=0;
    }
    parentManager=parentmanager;
    return 1;
  }
  return 0;
}


void* MemoryHeap::getRawMem(int size)
{
  void *retval;
  //int *iptr;
  int havespace;
  __require__(firstRawPage!=NULL);
  __require__(lastRawPage!=NULL);
  __require__(lastRawPage->memPage!=NULL);
  for(;;)
  {
    havespace=lastRawPage->pageSize-lastRawPage->pageUsage>(int)(size+sizeof(int));
    if(!havespace && lastRawPage->next)
    {
      lastRawPage=lastRawPage->next;
      continue;
    }
    break;
  }
  if(havespace)
  {
    retval=lastRawPage->memPage+lastRawPage->pageUsage+sizeof(int);
    ((int*)retval)[-1]=MM_RAW_HEAP_MAGIC;
    lastRawPage->pageUsage+=size+sizeof(int);
  }else
  {
    PRawHeapPage p=lastRawPage;
    char *rawheap=new char[size+sizeof(int)+rawHeapPageSize+sizeof(RawHeapPage)];
    __require__(rawheap!=NULL);
    lastRawPage=(PRawHeapPage)rawheap;
    p->next=lastRawPage;
    p=lastRawPage;
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
  if(checkPointsCount>MM_MAX_CHECK_POINTS)
  {
    return 0;
  }
  cpStack[checkPointsCount].page=lastRawPage;
  cpStack[checkPointsCount].pageUsage=lastRawPage->pageUsage;
  checkPointsCount++;
  return 1;
}
void MemoryHeap::doRollBack()
{
  if(checkPointsCount==0)return;
  checkPointsCount--;
  PRawHeapPage p;
  lastRawPage=cpStack[checkPointsCount].page;
  lastRawPage->pageUsage=cpStack[checkPointsCount].pageUsage;
  p=lastRawPage->next;
  while(p)
  {
    p->pageUsage=0;
    p=p->next;
  }
}
void MemoryHeap::resetRawHeap()
{
  checkPointsCount=0;
  PRawHeapPage p=firstRawPage;
  while(p)
  {
    p->pageUsage=0;
    p=p->next;
  }
}

void* MemoryHeap::getMem(int size)
{
  char* retval;
  int n=size>>blocksHeapQuantumShift;
  if(size&blocksHeapQuantumMask)
  {
    n++;
  }
  int alignedSize=blocksHeapQuantum*n;
  n--;

  if(n<blocksPool.Count())
  {
    if(blocksPool[n]->freeBlocks.Count()>0)
    {
      blocksPool[n]->freeBlocks.Pop(retval);
      blocksPool[n]->usedBlocks.Push(retval);
    }else
    {
      retval=new char[alignedSize+sizeof(int)*3];
      __require__(retval!=NULL);
      retval+=sizeof(int)*3;
      blocksPool[n]->usedBlocks.Push(retval);
    }
    ((int*)retval)[-1]=MM_BLOCK_HEAP_MAGIC;
    ((int*)retval)[-2]=blocksPool[n]->usedBlocks.Count()-1;
    ((int*)retval)[-3]=size;
  }else
  {
    int i;
    for(i=0;i<varBlocks.Count();i++)
    {
      if(varBlocks[i]->blocksize==alignedSize)
      {
        if(varBlocks[i]->freeBlocks.Count()>0)
        {
          varBlocks[i]->freeBlocks.Pop(retval);
          varBlocks[i]->usedBlocks.Push(retval);
        }else
        {
          retval=new char[alignedSize+sizeof(int)*3];
          __require__(retval!=NULL);
          retval+=sizeof(int)*3;
          varBlocks[i]->usedBlocks.Push(retval);
        }
        ((int*)retval)[-1]=MM_BLOCK_HEAP_MAGIC;
        ((int*)retval)[-2]=varBlocks[i]->usedBlocks.Count()-1;
        ((int*)retval)[-3]=size;
        return retval;
      }
    }
    varBlocks.Push(new BlocksHeapVarPage(alignedSize));
    retval=new char[alignedSize+sizeof(int)*3];
    __require__(retval!=NULL);
    retval+=sizeof(int)*3;
    varBlocks[-1]->usedBlocks.Push(retval);
    ((int*)retval)[-1]=MM_BLOCK_HEAP_MAGIC;
    ((int*)retval)[-2]=varBlocks[-1]->usedBlocks.Count()-1;
    ((int*)retval)[-3]=size;
  }
  return retval;
}

void MemoryHeap::freeMem(void*& block)
{
  __require__((((const int*)block)[-1])==MM_BLOCK_HEAP_MAGIC);
  int pos=((int*)block)[-2];
  int size=((int*)block)[-3];
  int n=size>>blocksHeapQuantumShift;
  int i,j;
  if(size&blocksHeapQuantumMask)
  {
    n++;
  }
  int alignedSize=blocksHeapQuantum*n;
  n--;
  if(n<blocksPool.Count())
  {
    if(pos>=blocksPool[n]->usedBlocks.Count())pos=blocksPool[n]->usedBlocks.Count()-1;
    for(i=pos;i>=0;i--)
    {
      if(blocksPool[n]->usedBlocks[i]==block)
      {
        blocksPool[n]->usedBlocks.Delete(i,1);
        blocksPool[n]->freeBlocks.Push((char*)block);
        block=0;
        return;
      }
    }
    fprintf(stderr,"ATTEMPT TO FREE UNALLOCATED BLOCK\n");
    throw 0;
  }else
  {
    for(i=0;i<varBlocks.Count();i++)
    {
      if(varBlocks[i]->blocksize==alignedSize)
      {
        for(j=pos>=varBlocks[i]->usedBlocks.Count()?varBlocks[i]->usedBlocks.Count()-1:pos;j>=0;j--)
        {
          if(varBlocks[i]->usedBlocks[j]==block)
          {
            varBlocks[i]->usedBlocks.Delete(j,1);
            varBlocks[i]->freeBlocks.Push((char*)block);
            block=0;
            return;
          }
        }
        fprintf(stderr,"ATTEMPT TO FREE UNALLOCATED BLOCK\n");
        throw 0;
      }
    }
    fprintf(stderr,"ATTEMPT TO FREE UNALLOCATED BLOCK\n");
    throw 0;
  }
  block=0;
}

void MemoryHeap::releaseHeap()
{
  resetRawHeap();
  parentManager->releaseHeap(this);
}


};//buffers
};//core
};//smsc
