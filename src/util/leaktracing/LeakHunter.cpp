#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/synchronization/Mutex.hpp"
#include "core/threads/Thread.hpp"
#include "util/debug.h"
#include <new>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MMAP_SIZE 1024*1024*1024


namespace smsc{
namespace util{
namespace leaktracing{

static const int TRACESIZE=20;
static int PAGESIZE=-1;

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

static void PrintTrace();

static void sighandler(int param)
{
  if(inbacktrace)
  {
    inbacktrace=0;
    siglongjmp(j,1);
  }
  PrintTrace();
  abort();
}


struct BlockInfo{
  void* addr;
  int size;
  int id;
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

const int LH_HASHSIZE=16*1024;
const int LH_DEFAULTBUCKETSIZE=16;

static int mmapfd;
static char* mem;
static int allocated;

void FixAlloc()
{
  if(allocated&(PAGESIZE-1))
  {
    allocated+=PAGESIZE-(allocated&(PAGESIZE-1));
  }
}


class LeakHunter{
  BlockInfo *memblocks[LH_HASHSIZE];
  int memcounts[LH_HASHSIZE];
  int memsizes[LH_HASHSIZE];

  BlockInfo *szmemblocks[LH_HASHSIZE];
  int szmemcounts[LH_HASHSIZE];
  int szmemsizes[LH_HASHSIZE];

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
  int GetBlockSize(void* ptr);
  void AddFreeBlock(BlockInfo*);
  void* FindFreeBlock(int sz);

  void DumpTrace(void**);

  void CheckPoint();

  void* Alloc(int size);
  void Free(void* ptr);
};


static LeakHunter* lh=NULL;
static mutex_t mtx=DEFAULTMUTEX;

static void sigcheckpoint(int param)
{
  lh->CheckPoint();
}

void* innermalloc(size_t sz)
{
  void* ptr=mem+allocated;
  allocated+=sz;
  FixAlloc();
  mprotect(ptr,sz,PROT_READ|PROT_WRITE);
  return ptr;
}


void LeakHunter::Init()
{
  sigset(11,sighandler);
  sigset(17,sigcheckpoint);

  __LH__DummyThread t;
  t.Start();
  t.WaitFor();
  int i;
  for(i=0;i<LH_HASHSIZE;i++)
  {
    memblocks[i]=(BlockInfo*)innermalloc(sizeof(BlockInfo)*LH_DEFAULTBUCKETSIZE);

    memcounts[i]=0;
    memsizes[i]=LH_DEFAULTBUCKETSIZE;
    szmemblocks[i]=(BlockInfo*)innermalloc(sizeof(BlockInfo)*LH_DEFAULTBUCKETSIZE);
    szmemcounts[i]=0;
    szmemsizes[i]=LH_DEFAULTBUCKETSIZE;
  }

  maxmem=0;
  alloc=0;
  cp=0;
  idcnt=0;
  makecp=0;
  init=1;
}

void* LeakHunter::Alloc(int size)
{
  smsc::core::synchronization::MutexGuard guard(m);
  void *res=FindFreeBlock(size);
  if(res)return res;
  if(allocated+size>=MMAP_SIZE)
  {
    fprintf(stderr,"OUT OF MEMORY\n");
    abort();
  }
  res=mem+allocated;
  allocated+=size;
  return res;
}

void LeakHunter::Free(void* ptr)
{

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

#define PRE_ALLOC 256
#define POST_ALLOC 256
#define CHECK_SIZE (PRE_ALLOC+POST_ALLOC+sizeof(size_t))
#define PRE_FILL_PATTERN 0xaa
#define FILL_PATTERN 0xcc
#define POST_FILL_PATTERN 0xbb


void LeakHunter::CheckPoint()
{
  /*
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
  */
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
  int idx=(((int)ptr)>>8)&(LH_HASHSIZE-1);
  if(memcounts[idx]==memsizes[idx])
  {
    BlockInfo *tmp=(BlockInfo*)innermalloc(sizeof(BlockInfo)*memsizes[idx]*2);
    memcpy(tmp,memblocks[idx],sizeof(BlockInfo)*memcounts[idx]);
    //free(memblocks[idx]);
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

  /*
  for(int i=0;i<LH_HASHSIZE;i++)
  {
    for(int j=0;j<memcounts[i];j++)
    {
      BlockInfo& bi=memblocks[i][j];
      unsigned char *ptr=(unsigned char *)bi.addr;
      ptr=ptr-sizeof(size_t)-PRE_ALLOC;
      for(int k=0;k<PRE_ALLOC;k++)
      {
        if(ptr[k]!=PRE_FILL_PATTERN)
        {
          abort();
        }
      }
      ptr=(unsigned char *)bi.addr;
      ptr+=bi.size;
      for(int k=0;k<POST_ALLOC;k++)
      {
        if(ptr[k]!=POST_FILL_PATTERN)
        {
          abort();
        }
      }
    }
  }
  */
  if(makecp)CheckPoint();
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

void LeakHunter::AddFreeBlock(BlockInfo* _bi)
{
  int rest=(PAGESIZE-(_bi->size&(PAGESIZE-1)))&(PAGESIZE-1);
  int rsize=_bi->size+rest+PAGESIZE;
  int idx=(rsize>>12)&(LH_HASHSIZE-1);
  if(szmemcounts[idx]==szmemsizes[idx])
  {
    BlockInfo *tmp=(BlockInfo *)innermalloc(sizeof(BlockInfo)*szmemsizes[idx]*2);
    memcpy(tmp,szmemblocks[idx],sizeof(BlockInfo)*szmemcounts[idx]);
    //free(szmemblocks[idx]);
    szmemblocks[idx]=tmp;
    szmemsizes[idx]*=2;
  }
  BackTrace(szmemblocks[idx][szmemcounts[idx]].trace);
  BlockInfo *bi=&szmemblocks[idx][szmemcounts[idx]];

  bi->addr=_bi->addr;
  bi->size=_bi->size;
  bi->id=idcnt++;
  void* ptr=((char*)bi->addr)-(_bi->size&(PAGESIZE-1));
  mprotect(ptr,rsize,PROT_NONE);
  szmemcounts[idx]++;
}

void* LeakHunter::FindFreeBlock(int sz)
{
  int idx=(sz>>12)&(LH_HASHSIZE-1);
  int i;
  for(i=szmemcounts[idx]-1;i>=0;i--)
  {
    int rest=(PAGESIZE-(szmemblocks[idx][i].size&(PAGESIZE-1)))&(PAGESIZE-1);
    int rsize=szmemblocks[idx][i].size+rest+PAGESIZE;
    if(rsize==sz)
    {
      void* ptr=((char*)szmemblocks[idx][i].addr)-(((int)szmemblocks[idx][i].addr)&(PAGESIZE-1));
      //fprintf(stderr,"FFB: ptr=%p\n",ptr);
      mprotect(ptr,rsize,PROT_READ|PROT_WRITE);
      if(szmemcounts[idx]-1-i>0)
      {
        memcpy(szmemblocks[idx]+i,szmemblocks[idx]+i+1,sizeof(BlockInfo)*(szmemcounts[idx]-1-i));
      }
      szmemcounts[idx]--;
      return ptr;
    }
  }
  return NULL;
}


int LeakHunter::RegisterDealloc(void* ptr)
{
  smsc::core::synchronization::MutexGuard guard(m);
  if(!init)Init();
  int idx=(((int)ptr)>>8)&(LH_HASHSIZE-1);
  int i;
  for(i=memcounts[idx]-1;i>=0;i--)
  {
    if(memblocks[idx][i].addr==ptr)
    {
      alloc-=memblocks[idx][i].size;

      AddFreeBlock(&memblocks[idx][i]);

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

int LeakHunter::GetBlockSize(void* ptr)
{
  smsc::core::synchronization::MutexGuard guard(m);
  if(!init)Init();
  int idx=(((int)ptr)>>8)&(LH_HASHSIZE-1);
  int i;
  for(i=memcounts[idx]-1;i>=0;i--)
  {
    if(memblocks[idx][i].addr==ptr)
    {
      return memblocks[idx][i].size;
    }
  }
  return -1;
}



static void deletelh()
{
  close(mmapfd);
  unlink("mmap");
  lh->~LeakHunter();
}

static void initlh()
{
  using namespace std;
  if(!lh)
  {
    mutex_lock(&mtx);
    if(!lh)
    {
      PAGESIZE=sysconf(_SC_PAGESIZE);
      mmapfd=open("mmap",O_CREAT|O_RDWR,0666);
      if(mmapfd==-1)
      {
        fprintf(stderr,"lh-open:%s\n",strerror(errno));
        exit(-1);
      }
      lseek(mmapfd,MMAP_SIZE,SEEK_SET);
      char c=0;
      write(mmapfd,&c,1);

      void* addr=mmap(0,MMAP_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE,mmapfd,0);
      if(addr==MAP_FAILED)
      {
        fprintf(stderr,"lh-mmap:%s\n",strerror(errno));
        exit(-1);
      }
      mem=(char*)addr;
      allocated=0;
      lh=new(innermalloc(sizeof(LeakHunter)))LeakHunter();
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
  using smsc::util::leaktracing::PAGESIZE;
  int rest=(PAGESIZE-(size&(PAGESIZE-1)))&(PAGESIZE-1);
  int rsize=size+rest;
  void *rv=smsc::util::leaktracing::lh->Alloc(rsize+PAGESIZE);
  char *mem=(char*)rv;
  mprotect(mem+rsize,PAGESIZE,PROT_NONE);
  rest&=~7;
  //fprintf(stderr,"rest=%d, allocated=%d, p=%p\n",rest,smsc::util::leaktracing::allocated,mem+rest);
  return mem+rest;
}

/*static void xfree(void* ptr)
{
  //lh->
}*/

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
      //xfree(mem);
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
      //xfree(mem);
    }
  }else
  {
    fprintf(stderr,"FATAL ERROR: delete [] NULL\n");
    smsc::util::leaktracing::PrintTrace();
  }
}

void* malloc(size_t size)
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
    fprintf(stderr,"malloc:0x%08x(%d)\n",(int)mem,size);
    smsc::util::leaktracing::PrintTrace();
  }
  smsc::util::leaktracing::lh->RegisterAlloc(mem,size);
  return mem;
}

void free(void* mem)
{
  smsc::util::leaktracing::initlh();
  if(mem)
  {
    if(getenv("LHFULLREPORT"))
    {
      fprintf(stderr,"free:0x%08x\n",(int)mem);
      smsc::util::leaktracing::PrintTrace();
    }
    if(smsc::util::leaktracing::lh->RegisterDealloc(mem))
    {
      //xfree(mem);
    }
  }
}

void* realloc(void* oldmem,size_t newsize)
{
  smsc::util::leaktracing::initlh();
  void* mem=xmalloc(newsize);
  if(getenv("LHFULLREPORT"))
  {
    fprintf(stderr,"realloc:0x%08x(%d)\n",(int)mem,newsize);
    smsc::util::leaktracing::PrintTrace();
  }
  smsc::util::leaktracing::lh->RegisterAlloc(mem,newsize);
  if(oldmem==0)return mem;
  int sz=smsc::util::leaktracing::lh->GetBlockSize(oldmem);
  if(sz==-1)
  {
    fprintf(stderr,"ERROR: REALLOC ON INVALID/UNKNOWN BLOCK:%p\n",oldmem);
    abort();
  }
  memcpy(mem,oldmem,sz);
  smsc::util::leaktracing::lh->RegisterDealloc(oldmem);
  return mem;
}
