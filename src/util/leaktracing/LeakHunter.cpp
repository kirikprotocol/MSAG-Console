#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/synchronization/Mutex.hpp"
#include "util/debug.h"
#include <new>
#include <unistd.h>
#include <ucontext.h>
#include <sys/frame.h>
#include <signal.h>

namespace smsc{
namespace util{
namespace leaktracing{

static const int MAXTRACESIZE=20;

static const int PRE_ALLOC=32;
static const int POST_ALLOC=32;
static const int USER_DATA=sizeof(size_t)*2+sizeof(void*);
static const int CHECK_SIZE=(PRE_ALLOC+POST_ALLOC+USER_DATA);

static const int PRE_FILL_PATTERN=0xaa;
static const int FILL_PATTERN=0xcc;
static const int POST_FILL_PATTERN=0xbb;
static const int DELETE_PATTERN=0xdd;


#if defined(sparc) || defined(__sparc)
#define FRAME_PTR_REGISTER REG_SP
#endif

#if defined(i386) || defined(__i386)
#define FRAME_PTR_REGISTER EBP
#endif

static void BackTrace(void** dump)
{
  int counter=0;

  ucontext_t u;
  getcontext(&u);
  frame* fp=(frame*)u.uc_mcontext.gregs[FRAME_PTR_REGISTER];

  void* savpc;
  frame* savfp;

  while(
    ((unsigned long)fp)>0x1000 &&
    (savpc = ((void*)fp->fr_savpc)) &&
    counter < MAXTRACESIZE
  )
  {
    dump[counter]=savpc;
    fp = (::frame*)fp->fr_savfp;
    ++counter;
  }
  if(counter!=MAXTRACESIZE)dump[counter]=0;
}



struct BlockInfo{
  void* addr;
  int size;
  int id;
  void *trace[MAXTRACESIZE];
};



const int LH_HASHSIZE=16*1024;
const int LH_DEFAULTBUCKETSIZE=16;

class LeakHunter{
  BlockInfo *memblocks[LH_HASHSIZE];
  int memcounts[LH_HASHSIZE];
  int memsizes[LH_HASHSIZE];

  int *cp;
  int cpcnt;
  int cpalloc;
  int makecp;

  int maxmem;
  int alloc;

  int idcnt;

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

  void CheckPoint();
};


static LeakHunter* lh=NULL;
static mutex_t mtx=DEFAULTMUTEX;

static void sigcheckpoint(int param)
{
  lh->CheckPoint();
}


void LeakHunter::Init()
{
  sigset(17,sigcheckpoint);

  int i;
  for(i=0;i<LH_HASHSIZE;i++)
  {
    memblocks[i]=(BlockInfo*)malloc(sizeof(BlockInfo)*LH_DEFAULTBUCKETSIZE);
    memcounts[i]=0;
    memsizes[i]=LH_DEFAULTBUCKETSIZE;
  }
  maxmem=0;
  alloc=0;
  cp=0;
  idcnt=0;
  makecp=0;
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

void LeakHunter::CheckPoint()
{
  f=fopen("lhcheckpoint.log","wt");
  if(!f)
  {
    fprintf(stderr,"\n\nFailed to open lhcheckpoint.log\n\n");
    return;
  }
  if(!makecp && !m.TryLock())
  {
    fclose(f);
    makecp=1;
    return;
  }
  makecp=0;
  if(cp)
  {
    if(alloc-cpalloc>0)fprintf(f,"Unallocated: %d bytes\n",alloc-cpalloc);
    for(int i=0;i<LH_HASHSIZE;i++)
    {
      for(int j=0;j<memcounts[i];j++)
      {
        bool fnd=false;
        for(int k=0;k<cpcnt;k++)
        {
          if(memblocks[i][j].id==cp[k])
          {
            fnd=true;
            break;
          }
        }
        if(!fnd)
        {
          BlockInfo *bi=&memblocks[i][j];
          fprintf(f,"Mem:0x%08X size %d, allocated at\n",(int)bi->addr,bi->size);
          DumpTrace(bi->trace);
          fprintf(f,"\n");
        }
      }
    }
  }
  int cnt=0;
  for(int i=0;i<LH_HASHSIZE;i++)
  {
    cnt+=memcounts[i];
  }
  if(cp)
  {
    free(cp);
    cp=0;
  }
  if(!cp)
  {
    cp=(int*)malloc(cnt*sizeof(int));
  }
  int k=0;
  //fprintf(f,"Checkpoint created, %d block stored\n",cnt);
  for(int i=0;i<LH_HASHSIZE;i++)
  {
    for(int j=0;j<memcounts[i];j++)
    {
      cp[k++]=memblocks[i][j].id;
      //fprintf(f,"%d\n",memblocks[i][j].id);
    }
  }
  cpcnt=cnt;
  cpalloc=alloc;
  fclose(f);
  m.Unlock();
}

void LeakHunter::DumpTrace(void** trace)
{
  for(int i=0;i<MAXTRACESIZE;i++)
  {
    if(!trace[i])break;
    fprintf(f,"{0x%08X}\n",(int)trace[i]);
  }
}


void LeakHunter::RegisterAlloc(void* ptr,int size)
{
  smsc::core::synchronization::MutexGuard guard(m);
  if(!init)Init();
  int idx=(((int)ptr)>>5)%LH_HASHSIZE;
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
  bi->id=idcnt++;
  memcounts[idx]++;
  alloc+=size;
  if(alloc>maxmem)
  {
    maxmem=alloc;
  }
  unsigned char* mem=(unsigned char*)ptr;
  mem-=PRE_ALLOC+USER_DATA;
  mem+=sizeof(size_t)*2;//size+magic
  *((void**)mem)=bi;
  if(makecp)CheckPoint();
}

static void PrintTrace()
{
  void* trace[MAXTRACESIZE];
  smsc::util::leaktracing::BackTrace(trace);
  for(int i=0;i<MAXTRACESIZE;i++)
  {
    if(!trace[i])break;
    fprintf(stderr,"l *0x%08X\n",(int)trace[i]);
  }
}

int LeakHunter::RegisterDealloc(void* ptr)
{
  smsc::core::synchronization::MutexGuard guard(m);
  if(!init)Init();
  int idx=(((int)ptr)>>5)%LH_HASHSIZE;
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
      if(makecp)CheckPoint();
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

static void* xmalloc(size_t size)
{
  using namespace smsc::util::leaktracing;
  int i;
  void *rv=malloc(size+CHECK_SIZE);
  unsigned char *mem=(unsigned char*)rv;
  size_t *ptr=(size_t*)mem;
  ptr[0]=size;
  ptr[1]=size^0xAAAABBBB^((size_t)mem);
  ptr[2]=0;
  mem+=USER_DATA;
  memset(mem,PRE_FILL_PATTERN,PRE_ALLOC);
  mem+=PRE_ALLOC;
  rv=mem;
  memset(mem,FILL_PATTERN,size);
  mem+=size;
  memset(mem,POST_FILL_PATTERN,POST_ALLOC);
  return rv;
}

static void xfree(void* ptr)
{
  using namespace smsc::util::leaktracing;
  unsigned char* mem=(unsigned char*)ptr;
  size_t *iptr=(size_t*)(mem-PRE_ALLOC-USER_DATA);
  size_t size=iptr[0];
  size_t magic=iptr[1];
  if((size^0xAAAABBBB^((size_t)iptr))!=magic)
  {
    fprintf(stderr,"ERROR: attempt to free invalid block %p size %d at\n",ptr,size);
    smsc::util::leaktracing::PrintTrace();
    return;
  }
  mem-=PRE_ALLOC;
  void *orgmem=mem-USER_DATA;
  int i;
  for(i=0;i<PRE_ALLOC;i++)
  {
    if(mem[i]!=PRE_FILL_PATTERN)
    {
      fprintf(stderr,"ERROR: Memory underrun for block %p size %d at\n",ptr,size);
      smsc::util::leaktracing::PrintTrace();
      for(int j=0;j<PRE_ALLOC/2;j++)
      {
        if(mem[j]!=PRE_FILL_PATTERN)abort();
      }
      break;
    }
  }
  mem+=PRE_ALLOC;
  mem+=size;
  for(i=0;i<POST_ALLOC;i++)
  {
    if(mem[i]!=POST_FILL_PATTERN)
    {
      fprintf(stderr,"ERROR: Memory overrun for block %p at\n",ptr);
      smsc::util::leaktracing::PrintTrace();
      for(int j=0;j<POST_ALLOC/2;j++)
      {
        if(mem[POST_ALLOC-1-j]!=POST_FILL_PATTERN)abort();
      }
      break;
    }
  }
  mem=(unsigned char*)ptr;
  memset(mem-PRE_ALLOC,DELETE_PATTERN,PRE_ALLOC+size+POST_ALLOC);
  free(orgmem);
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
  smsc::util::leaktracing::lh->RegisterAlloc(mem,size);
  return mem;
}

void operator delete(void* mem)
{
  smsc::util::leaktracing::initlh();
  if(mem)
  {
    if(smsc::util::leaktracing::lh->RegisterDealloc(mem))
    {
      xfree(mem);
    }
  }
}

void operator delete[](void* mem)
{
  smsc::util::leaktracing::initlh();
  if(mem)
  {
    if(smsc::util::leaktracing::lh->RegisterDealloc(mem))
    {
      xfree(mem);
    }
  }
}
