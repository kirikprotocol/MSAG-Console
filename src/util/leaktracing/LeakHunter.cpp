#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/synchronization/Mutex.hpp"
#include "core/threads/Thread.hpp"
#include "util/debug.h"

#define TRACESIZE 10

namespace smsc{
namespace util{
namespace leaktracing{

static void* threadstart=NULL;


void BackTrace(void** trace)
{
#define TRACE_BACK(n) \
  trace[n]=__builtin_return_address(n+2);\
  if(!trace[n])return;\
  if(trace[n]==threadstart){trace[n]=0;return;}
  TRACE_BACK(0)
  TRACE_BACK(1)
  TRACE_BACK(2)
  TRACE_BACK(3)
  TRACE_BACK(4)
  TRACE_BACK(5)
  TRACE_BACK(6)
  TRACE_BACK(7)
  TRACE_BACK(8)
  TRACE_BACK(9)
}


struct BlockInfo{
  void* addr;
  int size;
  void *trace[TRACESIZE];
};


class DummyThread:public smsc::core::threads::Thread{
public:
  virtual int Execute()
  {
    threadstart=__builtin_return_address(0);
  }
};


static class LeakHunter{
#define LH_HASHSIZE 1024
#define LH_DEFAULTBUCKETSIZE 16
  BlockInfo *memblocks[LH_HASHSIZE];
  int memcounts[LH_HASHSIZE];
  int memsizes[LH_HASHSIZE];

  int maxmem;
  int alloc;

  FILE *f;
  int init;

  smsc::core::synchronization::Mutex m;

public:
  LeakHunter(){init=0;Init();}
  ~LeakHunter();

  void Init();

  void RegisterAlloc(void* ptr,int size);
  void RegisterDealloc(void* ptr);

  void DumpTrace(void**);

} lh;

void LeakHunter::Init()
{
  smsc::core::synchronization::MutexGuard guard(m);
  DummyThread t;
  t.Start();
  t.WaitFor();
  int i;
  for(i=0;i<LH_HASHSIZE;i++)
  {
    memblocks[i]=(BlockInfo*)malloc(sizeof(BlockInfo)*LH_DEFAULTBUCKETSIZE);
    memcounts[i]=0;
    memsizes[i]=LH_DEFAULTBUCKETSIZE;
  }
  maxmem=0;
  alloc=0;
  init=1;
}


LeakHunter::~LeakHunter()
{
  char *fn="lh.log";
  if(getenv("LEAK_HUNTER_LOG"))
  {
    fn=getenv("LEAK_HUNTER_LOG");
  }
  f=fopen(fn,"wt+");
  fprintf(f,"Peak mem usage:%d bytes\n",maxmem);
  if(alloc==0)return;
  fprintf(f,"Unallocated: %d bytes\n",alloc);
  for(int i=0;i<LH_HASHSIZE;i++)
  {
    for(int j=0;j<memcounts[i];j++)
    {
      BlockInfo *bi=&memblocks[i][j];
      fprintf(f,"Mem:0x%08X size %d, allocated at\n",bi->addr,bi->size);
      DumpTrace(bi->trace);
      fprintf(f,"\n");
    }
  }
  fprintf(f,"---\n");
  fclose(f);
}

void LeakHunter::DumpTrace(void** trace)
{
  for(int i=0;i<TRACESIZE;i++)
  {
    if(!trace[i])break;
    fprintf(f,"{0x%08X}\n",trace[i]);
  }
}


void LeakHunter::RegisterAlloc(void* ptr,int size)
{
  smsc::core::synchronization::MutexGuard guard(m);
  if(!init)Init();
  int idx=(((int)ptr)>>5)&0x3ff;
  if(memcounts[idx]==memsizes[idx])
  {
    BlockInfo *tmp=(BlockInfo *)malloc(sizeof(BlockInfo)*memsizes[idx]*2);
    memcpy(tmp,memblocks[idx],sizeof(BlockInfo)*memcounts[idx]);
    free(memblocks[idx]);
    memblocks[idx]=tmp;
    memsizes[idx]*=2;
  }
  BackTrace(memblocks[idx][memcounts[idx]].trace);
  BlockInfo *bi=&memblocks[idx][memcounts[idx]];

  bi->addr=ptr;
  bi->size=size;
  memcounts[idx]++;
  alloc+=size;
  if(alloc>maxmem)
  {
    maxmem=alloc;
  }
}

static void PrintTrace()
{
  void* trace[TRACESIZE];
  smsc::util::leaktracing::BackTrace(trace);
  for(int i=0;i<TRACESIZE;i++)
  {
    if(!trace[i])break;
    fprintf(stderr,"{0x%08X}\n",trace[i]);
  }
}

void LeakHunter::RegisterDealloc(void* ptr)
{
  smsc::core::synchronization::MutexGuard guard(m);
  if(!init)Init();
  int idx=(((int)ptr)>>5)&0x3ff;
  int i;
  for(i=memcounts[idx]-1;i>=0;i--)
  {
    if(memblocks[idx][i].addr==ptr)
    {
      alloc-=memblocks[idx][i].size;
      //if(memblocks[idx][i].trace)free(memblocks[idx][i].trace);
      if(memcounts[idx]-1-i>0)
      {
        memcpy(memblocks[idx]+i,memblocks[idx]+i+1,sizeof(BlockInfo)*(memcounts[idx]-1-i));
      }
      memcounts[idx]--;
      return;
    }
  }
  fprintf(stderr,"Error: Block with address 0x%08X deallocated twice or wasn't allocated!\n",ptr);
  PrintTrace();
  throw "DELETE UNALLOCATED BLOCK";
}
}//leaktracing
}//util
}//smsc;

void* operator new(unsigned int size)
{
  void* mem=malloc(size);
  //printf("ALLOC:%x(%d)\n",mem,size);
  if(!mem)
  {
    fprintf(stderr,"OUT OF MEMORY!\n");
    throw "OUT OF MEMORY!\n";
  }
  smsc::util::leaktracing::lh.RegisterAlloc(mem,size);
  return mem;
}

void* operator new[](unsigned int size)
{
  void* mem=malloc(size);
  if(!mem)
  {
    fprintf(stderr,"OUT OF MEMORY!\n");
    throw "OUT OF MEMORY!\n";
  }
  smsc::util::leaktracing::lh.RegisterAlloc(mem,size);
  return mem;
}

void operator delete(void* mem)
{
  if(mem)
  {
    smsc::util::leaktracing::lh.RegisterDealloc(mem);
    free(mem);
  }
}

void operator delete[](void* mem)
{
  //printf("FREE:%x\n",mem);
  if(mem)
  {
    smsc::util::leaktracing::lh.RegisterDealloc(mem);
    free(mem);
  }else
  {
    fprintf(stderr,"FATAL ERROR: delete [] NULL\n");
    smsc::util::leaktracing::PrintTrace();
  }
}
