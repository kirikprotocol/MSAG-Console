#ifndef __INTHASH_HPP__
#define __INTHASH_HPP__

#define INTHASH_MAX_CHAIN_LENGTH 16

#include <string.h>

namespace smsc{
namespace core{
namespace buffers{

template <class T>
class IntHash{
public:
  IntHash()
  {
    keys=0;
    values=0;
    refcounts=0;

    count=0;
    size=0;

    reflist=0;
    reflistsize=0;

    emptycount=0;
  }
  explicit IntHash(int n)
  {
    SetSize(n);
  }

  ~IntHash()
  {
    if(keys)delete [] keys;
    if(values)delete [] values;
    if(refcounts)delete [] refcounts;
    if(reflist)delete [] reflist;
  }

  void SetSize(int n)
  {
    if(n>size)Rehash(n);
  }

  int Insert(int key,const T& value)
  {
    //printf("ins:%d\n",key);fflush(stdout);
    if(!size || count>=size/4)Rehash(size*4);
    unsigned int idx=Index(key,0);
    int res=0;
    int attempt=0;
    for(;;)
    {
      if(keys[idx]<0)
      {
        //printf("ins:new\n");fflush(stdout);
        keys[idx]=key;
        values[idx]=value;
        refcounts[idx]++;
        res=1;
        break;
      }
      if(keys[idx]==key)
      {
        //printf("ins:exists\n");fflush(stdout);
        values[idx]=value;
        refcounts[idx]++;
        res=0;
        break;
      }
      //AddRef(idx,attempt);
      refcounts[idx]++;
      attempt++;
      if(attempt>=INTHASH_MAX_CHAIN_LENGTH)
      {
        //printf("rehash\n");fflush(stdout);
        Rehash(size*2);
        //printf("rehash done\n");fflush(stdout);
        attempt=0;
      }
      idx=Index(key,attempt);
    }
    if(res)count++;
    return res;
  }

  T& Get(int key)const
  {
    if(!size || !count)throw 0;
    unsigned idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0)throw 0;
      attempt++;
    }while(keys[idx]!=key);
    return values[idx];
  }

  T* GetPtr(int key)const
  {
    if(!size || !count)return 0;
    unsigned idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0)return 0;
      attempt++;
    }while(keys[idx]!=key);
    return &values[idx];
  }


  int Get(int key,T& value)const
  {
//    printf("get:%d\n",key);fflush(stdout);
    if(!size || !count)return 0;
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0)return 0;
      attempt++;
    }while(keys[idx]!=key);
    value=values[idx];
//    printf("get:ok\n");fflush(stdout);
    return 1;
  }

  int Exist(int key)
  {
    if(!count)return 0;
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0)return 0;
      attempt++;
    }while(keys[idx]!=key);
    return 1;
  }

  int Delete(int key)
  {
    //printf("del:%d\n",key);fflush(stdout);
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0)
      {
        //printf("del:not found\n");fflush(stdout);
        return 0;
      }
      AddRef(idx,attempt);
    }while(keys[idx]!=key);
    //printf("del:ok\n");fflush(stdout);
    keys[idx]=-1;
    for(int i=0;i<attempt;i++)
    {
      //printf("dele:%d\n",reflist[i]);fflush(stdout);
      refcounts[reflist[i]]--;
    }
    count--;
    return 1;
  }

  class Iterator{
  public:
    Iterator(const IntHash& owner):idx(0),h(owner){}
    Iterator(const Iterator& src):idx(src.idx),h(src.h){}
    Iterator& operator=(const Iterator& src)
    {
      idx=src.idx;
      h=src.h;
    }
    int Next(int& k,T& v)
    {
      if(idx>=h.size)return 0;
      while(idx<h.size && h.keys[idx]==-1)idx++;
      if(idx>=h.size)return 0;
      k=h.keys[idx];
      v=h.values[idx];
      idx++;
      return 1;
    }
    int Next(int& k,T*& v)
    {
      if(idx>=h.size)return 0;
      while(idx<h.size && h.keys[idx]==-1)idx++;
      if(idx>=h.size)return 0;
      k=h.keys[idx];
      v=&h.values[idx];
      idx++;
      return 1;
    }
    protected:
    int idx;
    const IntHash& h;
  };
  friend class Iterator;

  Iterator First()const
  {
    return Iterator(*this);
  }

  int Count(){return count;}

  void Empty()
  {
    if(keys)delete [] keys;
    if(values)delete [] values;
    if(refcounts)delete [] refcounts;
    if(reflist)delete [] reflist;
    keys=0;
    values=0;
    refcounts=0;

    count=0;
    size=0;

    reflist=0;
    reflistsize=0;

    emptycount=0;
  }


protected:
  int *keys;
  int *refcounts;
  T *values;
  int count;
  int size;
  int *reflist;
  int reflistsize;
  int emptycount;

  inline unsigned int HashKey(int key,int attempt)const
  {
    return key+attempt;
  }

  inline unsigned int Index(int key,int attempt)const
  {
    return HashKey(key,attempt)%size;
  }

  void AddRef(unsigned int refidx,int& idx)
  {
    if(reflistsize==idx)
    {
      int newsize=reflistsize*2;
      if(newsize==0)newsize=16;
      //printf("reflistresize:%d->%d\n",reflistsize,newsize);
      int *newlist=new int[newsize];
      for(int i=0;i<reflistsize;i++)newlist[i]=reflist[i];
      if(reflist)delete [] reflist;
      reflist=newlist;
      reflistsize=newsize;
    }
    reflist[idx]=refidx;
    idx++;
  }

  void Rehash(int newsize)
  {
    if(newsize==0)newsize=16;
    int *okeys=keys;
    int *orefs=refcounts;
    T *oval=values;
    keys=new int[newsize];
    refcounts=new int[newsize];
    values=new T[newsize];

    int i;
    memset(keys,-1,sizeof(int)*newsize);
    memset(refcounts,0,sizeof(int)*newsize);

    int oldsize=size;
    size=newsize;
    //count=0;
    int idx,att;
    for(i=0;i<oldsize;i++)
    {
      if(orefs[i]>0 && okeys[i]>=0)
      {
        //Insert(okeys[i],oval[i]);
        att=0;
        do{
          idx=Index(okeys[i],att++);
        }while(refcounts[idx]++!=0);
        keys[idx]=okeys[i];
        values[idx]=oval[i];
      }
    }
    if(okeys)delete [] okeys;
    if(okeys)delete [] oval;
    if(okeys)delete [] orefs;
  }

}; //IntHash

};//buffers
};//core
};//smsc

#endif
