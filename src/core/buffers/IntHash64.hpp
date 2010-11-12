#ifndef __SMSC_CORE_BUFFERS_INTHASH64_HPP__
#define __SMSC_CORE_BUFFERS_INTHASH64_HPP__

#define INTHASH64_MAX_CHAIN_LENGTH 32

#include <string.h>
#include <string>
#include <stdexcept>
#include "util/int.h"
#include "util/crc32.h"

namespace smsc{
namespace core{
namespace buffers{

template <class T>
class IntHash64{
public:
  typedef int64_t KeyType;
  IntHash64()
  {
    keys=0;
    values=0;
    refcounts=0;

    count=0;
    size=0;

    reflist=0;
    reflistsize=0;

  }
  IntHash64(const IntHash64& src)
  {
    keys=0;
    values=0;
    refcounts=0;

    count=0;
    size=0;

    reflist=0;
    reflistsize=0;

    *this=src;
  }

  IntHash64& operator=(const IntHash64& src)
  {
    Empty();
    size=src.size;
    count=src.count;
    keys=new KeyType[size];
    values=new T[size];
    refcounts=new int[size];
    memcpy(keys,src.keys,sizeof(KeyType)*size);
    memcpy(refcounts,src.refcounts,sizeof(int)*size);
    for(int i=0;i<size;i++)
    {
      if(refcounts[i]!=0)values[i]=src.values[i];
    }
    reflist=0;
    reflistsize=0;
    return *this;
  }

  explicit IntHash64(int n)
  {
    keys=0;
    values=0;
    refcounts=0;

    count=0;
    size=0;

    reflist=0;
    reflistsize=0;
    SetSize(n);
  }

  ~IntHash64()
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

  T& Insert(KeyType key,const T& value)
  {
    //printf("ins:%d\n",key);fflush(stdout);
    if(!size || count>=size/2)Rehash();
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
        // the next line commented (a bug supposed), bukind 2010-11-12
        // refcounts[idx]++;
        res=0;
        break;
      }
      //AddRef(idx,attempt);
      refcounts[idx]++;
      attempt++;
      if(attempt>=INTHASH64_MAX_CHAIN_LENGTH)
      {
        //printf("rehash\n");fflush(stdout);
        Rehash();
        //printf("rehash done\n");fflush(stdout);
        attempt=0;
      }
      idx=Index(key,attempt);
    }
    if(res)count++;
    return values[idx];
  }

  const T& Get(KeyType key)const
  {
    if(!size || !count)throw std::runtime_error("get on empty inthash");
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0)throw std::runtime_error("get on empty inthash");
      attempt++;
    }while(keys[idx]!=key);
    return values[idx];
  }

  T& Get(KeyType key)
  {
    if(!size || !count)throw std::runtime_error("get on empty inthash");
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0)throw std::runtime_error("get on empty inthash");
      attempt++;
    }while(keys[idx]!=key);
    return values[idx];
  }

  T* GetPtr(KeyType key)const
  {
    if(!size || !count)return 0;
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0)return 0;
      attempt++;
    }while(keys[idx]!=key);
    return &values[idx];
  }


  int Get(KeyType key,T& value)const
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

  int Exist(KeyType key)
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

  int Delete(KeyType key)
  {
    //printf("del:%d\n",key);fflush(stdout);
    if(size==0)return 0;
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

  bool Pop( KeyType key, T& value )
  {
    if(size==0)return false;
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0)
      {
        //printf("del:not found\n");fflush(stdout);
        return false;
      }
      AddRef(idx,attempt);
    }while(keys[idx]!=key);
    //printf("del:ok\n");fflush(stdout);
    value = values[idx];
    keys[idx]=-1;
    for(int i=0;i<attempt;i++)
    {
      //printf("dele:%d\n",reflist[i]);fflush(stdout);
      refcounts[reflist[i]]--;
    }
    count--;
    return true;
  }

  class Iterator{
  public:
    Iterator(const IntHash64& owner):idx(0),h(owner){}
    Iterator(const Iterator& src):idx(src.idx),h(src.h){}
    Iterator& operator=(const Iterator& src)
    {
      idx=src.idx;
      h=src.h;
    }
    int Next(KeyType& k,T& v)
    {
      if(idx>=h.size || h.count==0)return 0;
      while(idx<h.size && h.refcounts[idx]==0)idx++;
      if(idx>=h.size)return 0;
      k=h.keys[idx];
      v=h.values[idx];
      idx++;
      return 1;
    }
    int Next(KeyType& k,T*& v)
    {
      if(idx>=h.size || h.count==0)return 0;
      while(idx<h.size && h.refcounts[idx]==0)idx++;
      if(idx>=h.size)return 0;
      k=h.keys[idx];
      v=&h.values[idx];
      idx++;
      return 1;
    }
    protected:
    int idx;
    const IntHash64& h;
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

  }

protected:
  KeyType *keys;
  int *refcounts;
  T *values;
  int count;
  int size;
  int *reflist;
  int reflistsize;

  inline unsigned int HashKey(KeyType key,int attempt)const
  {
//    uint32_t rv=0;
//    for(int i=0;i<=attempt;i++)rv=smsc::util::crc32(rv,&key,sizeof(key));
//    return rv;
    return key+attempt;
  }

  inline unsigned int Index(KeyType key,int attempt)const
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

  void Rehash()
  {
    int newsize;
    if(size==0)newsize=15;else
    {
      newsize=(size+1)*2-1;
    }
    KeyType *okeys=keys;
    int *orefs=refcounts;
    T *oval=values;
    keys=new KeyType[newsize];
    refcounts=new int[newsize];
    values=new T[newsize];

    int i;
    memset(keys,-1,sizeof(KeyType)*newsize);
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
    if(oval)delete [] oval;
    if(orefs)delete [] orefs;
  }

}; //IntHash

}//buffers
}//core
}//smsc

#endif
