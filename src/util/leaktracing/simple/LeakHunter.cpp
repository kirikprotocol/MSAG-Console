#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/synchronization/Mutex.hpp"
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

#define MMAP_SIZE 512*1024*1024


namespace smsc{
namespace util{
namespace leaktracing{

static int PAGESIZE=-1;


struct BlockInfo{
  void* addr;
  int size;
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
  int GetBlockSize(void* ptr);
  void AddFreeBlock(BlockInfo*);
  void* FindFreeBlock(int sz);

  void* Alloc(int size);
  void Free(void* ptr);
};


static LeakHunter* lh=NULL;
static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;

void* innermalloc(size_t sz)
{
  void* ptr=mem+allocated;
  allocated+=sz;
  FixAlloc();
  mprotect((caddr_t)ptr,sz,PROT_READ|PROT_WRITE);
  return ptr;
}


void LeakHunter::Init()
{
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
      fprintf(f,"Mem:0x%08X size %d\n",(int)bi->addr,bi->size);
      fprintf(f,"\n");
    }
  }
  fprintf(f,"---\n");
  fclose(f);
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
    memblocks[idx]=tmp;
    memsizes[idx]*=2;
  }
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

void LeakHunter::AddFreeBlock(BlockInfo* _bi)
{
  int rest=(PAGESIZE-(_bi->size&(PAGESIZE-1)))&(PAGESIZE-1);
  int rsize=_bi->size+rest+PAGESIZE;
  int idx=(rsize>>12)&(LH_HASHSIZE-1);
  if(szmemcounts[idx]==szmemsizes[idx])
  {
    BlockInfo *tmp=(BlockInfo *)innermalloc(sizeof(BlockInfo)*szmemsizes[idx]*2);
    memcpy(tmp,szmemblocks[idx],sizeof(BlockInfo)*szmemcounts[idx]);
    szmemblocks[idx]=tmp;
    szmemsizes[idx]*=2;
  }
  BlockInfo *bi=&szmemblocks[idx][szmemcounts[idx]];

  bi->addr=_bi->addr;
  bi->size=_bi->size;
  void* ptr=((char*)bi->addr)-(_bi->size&(PAGESIZE-1));
  mprotect((caddr_t)ptr,rsize,PROT_NONE);
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
      mprotect((caddr_t)ptr,rsize,PROT_READ|PROT_WRITE);
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

      return 1;
    }
  }
  fprintf(stderr,"Error: Block with address 0x%08X deallocated twice or wasn't allocated!\n",(int)ptr);
  abort();
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
    pthread_mutex_lock(&mtx);
    if(!lh)
    {
      PAGESIZE=sysconf(_SC_PAGESIZE);
      mmapfd=open("mmap",O_CREAT|O_RDWR,0666);
      if(mmapfd==-1)
      {
        fprintf(stderr,"lh-open:%d\n",errno);
        exit(-1);
      }
      lseek(mmapfd,MMAP_SIZE,SEEK_SET);
      char c=0;
      write(mmapfd,&c,1);

      void* addr=mmap(0,MMAP_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE,mmapfd,0);
      if(addr==MAP_FAILED)
      {
        fprintf(stderr,"lh-mmap:%d\n",errno);
        exit(-1);
      }
      mem=(char*)addr;
      allocated=0;
      lh=new(innermalloc(sizeof(LeakHunter)))LeakHunter();
      atexit(deletelh);
    }
    pthread_mutex_unlock(&mtx);
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
  mprotect((caddr_t)mem+rsize,PAGESIZE,PROT_NONE);
  rest&=~7;
  return mem+rest;
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
    smsc::util::leaktracing::lh->RegisterDealloc(mem);
  }
}

void operator delete[](void* mem)
{
  smsc::util::leaktracing::initlh();
  if(mem)
  {
    smsc::util::leaktracing::lh->RegisterDealloc(mem);
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
  smsc::util::leaktracing::lh->RegisterAlloc(mem,size);
  return mem;
}

void free(void* mem)
{
  smsc::util::leaktracing::initlh();
  if(mem)
  {
    smsc::util::leaktracing::lh->RegisterDealloc(mem);
  }
}

void* realloc(void* oldmem,size_t newsize)
{
  smsc::util::leaktracing::initlh();
  void* mem=xmalloc(newsize);
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
