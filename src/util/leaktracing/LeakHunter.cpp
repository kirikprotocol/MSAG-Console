#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/synchronization/Mutex.hpp"
#include "core/threads/Thread.hpp"
#include "util/debug.h"
#include <new>
#include <setjmp.h>
#include <signal.h>


namespace smsc{
namespace util{
namespace leaktracing{

const int TRACESIZE=20;

static void* threadstart=NULL;

static int inbacktrace=0;
static sigjmp_buf j;

static void sighandler(int);

static void BackTrace(void** trace)
{
  inbacktrace=1;
  sigsetjmp(j,0);
  if(!inbacktrace)
  {
    sigset(11,sighandler);
    return;
  }
#define TRACE_BACK(n) \
  trace[n]=0;\
  trace[n]=__builtin_return_address(n+2);\
  if(!trace[n]){inbacktrace=0;return;}\
  if(trace[n]==threadstart){inbacktrace=0;trace[n]=0;return;}
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
  TRACE_BACK(10)
  TRACE_BACK(11)
  TRACE_BACK(12)
  TRACE_BACK(13)
  TRACE_BACK(14)
  TRACE_BACK(15)
  TRACE_BACK(16)
  TRACE_BACK(17)
  TRACE_BACK(18)
  TRACE_BACK(19)
}

static void sighandler(int param)
{
  if(inbacktrace)
  {
    inbacktrace=0;
    siglongjmp(j,1);
  }
  abort();
}


struct BlockInfo{
  void* addr;
  int size;
  void *trace[TRACESIZE];
};


class __LH__DummyThread:public smsc::core::threads::Thread{
public:
  virtual int Execute()
  {
    threadstart=__builtin_return_address(0);
    return 0;
  }
};

const int LH_HASHSIZE=1024;
const int LH_DEFAULTBUCKETSIZE=16;

class LeakHunter{
  BlockInfo *memblocks[LH_HASHSIZE];
  int memcounts[LH_HASHSIZE];
  int memsizes[LH_HASHSIZE];

  int maxmem;
  int alloc;

  FILE *f;
  int init;

  smsc::core::synchronization::Mutex m;

public:
  LeakHunter()
  {
    smsc::core::synchronization::MutexGuard guard(m);
    init=0;
    Init();
  }
  ~LeakHunter();

  void Init();

  void RegisterAlloc(void* ptr,int size);
  int RegisterDealloc(void* ptr);

  void DumpTrace(void**);

};


static LeakHunter* lh=NULL;
static mutex_t mtx=DEFAULTMUTEX;

void LeakHunter::Init()
{
  sigset(11,sighandler);

  __LH__DummyThread t;
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
      fprintf(f,"Mem:0x%08X size %d, allocated at\n",(int)bi->addr,bi->size);
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
    fprintf(f,"{0x%08X}\n",(int)trace[i]);
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
    fprintf(stderr,"l *0x%08X\n",(int)trace[i]);
  }
}

int LeakHunter::RegisterDealloc(void* ptr)
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
      return 1;
    }
  }
  fprintf(stderr,"Error: Block with address 0x%08X deallocated twice or wasn't allocated!\n",(int)ptr);
  PrintTrace();
  return 0;
  //throw "DELETE UNALLOCATED BLOCK";
}

static void deletelh()
{
  lh->~LeakHunter();
  free(lh);
}

static void initlh()
{
  using namespace std;
  if(!lh)
  {
    mutex_lock(&mtx);
    if(!lh)
    {
      void *mem=malloc(sizeof(LeakHunter));
      lh=new(mem)LeakHunter();
      atexit(deletelh);
    }
    mutex_unlock(&mtx);
  }
}

}//leaktracing
}//util
}//smsc;

#define PRE_ALLOC 32
#define POST_ALLOC 32
#define CHECK_SIZE (PRE_ALLOC+POST_ALLOC+sizeof(size_t))
#define PRE_FILL_PATTERN 0xaa
#define FILL_PATTERN 0xcc
#define POST_FILL_PATTERN 0xbb

static void* xmalloc(size_t size)
{
  void *rv=malloc(size+CHECK_SIZE);
  unsigned char *mem=(unsigned char*)rv;
  int i;
  for(i=0;i<PRE_ALLOC;i++)mem[i]=PRE_FILL_PATTERN;
  mem+=PRE_ALLOC;
  *((size_t*)mem)=size;
  mem+=sizeof(size_t);
  for(i=0;i<size;i++)mem[i]=FILL_PATTERN;
  mem+=size;
  for(i=0;i<POST_ALLOC;i++)mem[i]=POST_FILL_PATTERN;
  return (unsigned char*)rv+PRE_ALLOC+sizeof(size_t);
}

static void xfree(void* ptr)
{
  unsigned char* mem=(unsigned char*)ptr;
  size_t size=((size_t*)mem)[-1];
  mem-=sizeof(size_t);
  mem-=PRE_ALLOC;
  int i;
  for(i=0;i<PRE_ALLOC;i++)
  {
    if(mem[i]!=PRE_FILL_PATTERN)
    {
      fprintf(stderr,"ERROR: Memory underrun for block %p size %d at\n",ptr,size);
      smsc::util::leaktracing::PrintTrace();
      abort();
    }
  }
  mem=(unsigned char*)ptr;
  mem+=size;
  for(i=0;i<POST_ALLOC;i++)
  {
    if(mem[i]!=POST_FILL_PATTERN)
    {
      fprintf(stderr,"ERROR: Memory overrun for block %p at\n",ptr);
      smsc::util::leaktracing::PrintTrace();
      abort();
    }
  }
}

void* operator new(unsigned int size)
{
  smsc::util::leaktracing::initlh();
  void* mem=xmalloc(size);
  //printf("ALLOC:%x(%d)\n",mem,size);
  if(!mem)
  {
    fprintf(stderr,"OUT OF MEMORY!\n");
    throw "OUT OF MEMORY!\n";
  }
  if(getenv("LHFULLREPORT"))
  {
    fprintf(stderr,"new:0x%08x(%d)\n",(int)mem,size);
    smsc::util::leaktracing::PrintTrace();
  }
  smsc::util::leaktracing::lh->RegisterAlloc(mem,size);
  return mem;
}

void* operator new[](unsigned int size)
{
  smsc::util::leaktracing::initlh();
  void* mem=xmalloc(size);
  if(!mem)
  {
    fprintf(stderr,"OUT OF MEMORY!\n");
    throw "OUT OF MEMORY!\n";
  }
  if(getenv("LHFULLREPORT"))
  {
    fprintf(stderr,"new[]:0x%08x(%d)\n",(int)mem,size);
    smsc::util::leaktracing::PrintTrace();
  }
  smsc::util::leaktracing::lh->RegisterAlloc(mem,size);
  return mem;
}

void operator delete(void* mem)
{
  smsc::util::leaktracing::initlh();
  if(mem)
  {
    if(getenv("LHFULLREPORT"))
    {
      fprintf(stderr,"delete:0x%08x\n",(int)mem);
      smsc::util::leaktracing::PrintTrace();
    }
    if(smsc::util::leaktracing::lh->RegisterDealloc(mem))
    {
      xfree(mem);
    }
  }
}

void operator delete[](void* mem)
{
  //printf("FREE:%x\n",mem);
  smsc::util::leaktracing::initlh();
  if(mem)
  {
    if(getenv("LHFULLREPORT"))
    {
      fprintf(stderr,"delete[]:0x%08x\n",(int)mem);
      smsc::util::leaktracing::PrintTrace();
    }
    if(smsc::util::leaktracing::lh->RegisterDealloc(mem))
    {
      xfree(mem);
    }
  }else
  {
    fprintf(stderr,"FATAL ERROR: delete [] NULL\n");
    smsc::util::leaktracing::PrintTrace();
  }
}
