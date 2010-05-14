#ifndef __INTHASH_HPP__
#define __INTHASH_HPP__

#define INTHASH_MAX_CHAIN_LENGTH 16

#include <string.h>
#include <string>
#include <stdexcept>

#ifdef INTHASH_USAGE_CHECKING
#include <cstdio>
#include "util/fileline.h"
#include "core/synchronization/Mutex.hpp"
#endif

namespace smsc{
namespace core{
namespace buffers{

template <class T>
class IntHash{
public:
#ifdef INTHASH_USAGE_CHECKING
  IntHash( const char* id ) : id_(id)
#else
  IntHash()
#endif
  {
    keys=0;
    values=0;
    refcounts=0;

    count=0;
    size=0;

    reflist=0;
    reflistsize=0;
  }
  IntHash(const IntHash& src)
#ifdef INTHASH_USAGE_CHECKING
        : id_(src.id_)
#endif
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

  IntHash& operator=(const IntHash& src)
  {
    if (this == &src) return *this;
#ifdef INTHASH_USAGE_CHECKING
    id_ = src.id_;
#endif
    Empty();
    size=src.size;
    count=src.count;
    keys=new int[size];
    values=new T[size];
    refcounts=new unsigned int[size];
    memcpy(keys,src.keys,sizeof(int)*size);
    memcpy(refcounts,src.refcounts,sizeof(unsigned int)*size);
    for(int i=0;i<size;i++)
    {
      if(refcounts[i]&0x80000000)values[i]=src.values[i];
    }
    return *this;
  }

#ifdef INTHASH_USAGE_CHECKING
  explicit IntHash(int n, const char* id ) : id_(id)
#else
  explicit IntHash(int n)
#endif
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

  T& Insert(int key,const T& value)
  {
    //printf("ins:%d\n",key);fflush(stdout);
    if(!size || count>=size/2)Rehash(size*2);
    unsigned int idx=Index(key,0);
    int res=0;
    int attempt=0;
    for(;;)
    {
      if((refcounts[idx]&0x80000000)==0)
      {
        //printf("ins:new\n");fflush(stdout);
        keys[idx]=key;
        values[idx]=value;
        refcounts[idx]++;
        refcounts[idx]|=0x80000000;
        res=1;
        break;
      }
      if(keys[idx]==key)
      {
        //printf("ins:exists\n");fflush(stdout);
        values[idx]=value;
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
    return values[idx];
  }

  const T& Get(int key)const
  {
    if(!size || !count)throw std::runtime_error("get on empty inthash");
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0 || attempt>INTHASH_MAX_CHAIN_LENGTH)
      {
        throw std::runtime_error("IntHash::Get - item not found");
      }
      attempt++;
    }while((refcounts[idx]&0x80000000)==0 || keys[idx]!=key);
    return values[idx];
  }

  T& Get(int key)
  {
    if(!size || !count)throw std::runtime_error("get on empty inthash");
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0 || attempt>INTHASH_MAX_CHAIN_LENGTH)
      {
        throw std::runtime_error("IntHash::Get - item not found");
      }
      attempt++;
    }while((refcounts[idx]&0x80000000)==0 || keys[idx]!=key);
    return values[idx];
  }

  T* GetPtr(int key)const
  {
    if(!size || !count)
    {
      return 0;
    }
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0 || attempt>INTHASH_MAX_CHAIN_LENGTH)return 0;
      attempt++;
    }while((refcounts[idx]&0x80000000)==0 || keys[idx]!=key);
    return &values[idx];
  }

  bool Get(int key,T& value)const
  {
//    printf("get:%d\n",key);fflush(stdout);
    if(!size || !count)
    {
      return false;
    }
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0 || attempt>INTHASH_MAX_CHAIN_LENGTH)return false;
      attempt++;
    }while((refcounts[idx]&0x80000000)==0 || keys[idx]!=key);
    value=values[idx];
//    printf("get:ok\n");fflush(stdout);
    return true;
  }

  bool Exist(int key)const
  {
    if(!count)
    {
      return false;
    }
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0 || attempt>INTHASH_MAX_CHAIN_LENGTH)return 0;
      attempt++;
    }while((refcounts[idx]&0x80000000)==0 || keys[idx]!=key);
    return true;
  }

  bool Delete(int key)
  {
    //printf("del:%d\n",key);fflush(stdout);
    if(size==0)
    {
      return false;
    }
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0 || attempt>INTHASH_MAX_CHAIN_LENGTH)
      {
        //printf("del:not found\n");fflush(stdout);
        return false;
      }
      AddRef(idx,attempt);
    }while((refcounts[idx]&0x80000000)==0 || keys[idx]!=key);
    //printf("del:ok\n");fflush(stdout);
    refcounts[idx]&=0x7fffffff;
    for(int i=0;i<attempt;i++)
    {
      //printf("dele:%d\n",reflist[i]);fflush(stdout);
      refcounts[reflist[i]]--;
    }
    count--;
    return true;
  }

  bool Pop(int key,T& value)
  {
    //printf("del:%d\n",key);fflush(stdout);
    if(size==0)
    {
      return false;
    }
    unsigned int idx;
    int attempt=0;
    do{
      idx=Index(key,attempt);
      if(refcounts[idx]==0 || attempt>INTHASH_MAX_CHAIN_LENGTH)
      {
        //printf("del:not found\n");fflush(stdout);
        return false;
      }
      AddRef(idx,attempt);
    }while((refcounts[idx]&0x80000000)==0 || keys[idx]!=key);
    //printf("del:ok\n");fflush(stdout);
    value = values[idx];
    refcounts[idx]&=0x7fffffff;
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
    Iterator(const IntHash& owner):idx(0),h(&owner){}
    Iterator(const Iterator& src):idx(src.idx),h(src.h){}

    Iterator& operator=(const Iterator& src)
    {
      idx=src.idx;
      h=src.h;
      return *this;
    }

    bool Next(int& k,T& v)
    {
      if(idx>=h->size || h->count==0)return false;
      while(idx<h->size && (h->refcounts[idx]&0x80000000)==0)idx++;
      if(idx>=h->size)return false;
      k=h->keys[idx];
      v=h->values[idx];
      idx++;
      return true;
    }
    bool Next(int& k,T*& v)
    {
      if(idx>=h->size || h->count==0)return false;
      while(idx<h->size && (h->refcounts[idx]&0x80000000)==0)idx++;
      if(idx>=h->size)return false;
      k=h->keys[idx];
      v=&h->values[idx];
      idx++;
      return true;
    }
    void First()
    {
      idx=0;
    }
  protected:
    int idx;
    const IntHash* h;
  };
  friend class Iterator;

  Iterator First()const
  {
    return Iterator(*this);
  }

  int Count() const {return count;}
    int Size() const {return size;}

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
  int *keys;
  unsigned int *refcounts;
  T *values;
  int count;
  int size;
  int *reflist;
  int reflistsize;

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
    unsigned int *orefs=refcounts;
    T *oval=values;
    keys=new int[newsize];
    refcounts=new unsigned int[newsize];
    values=new T[newsize];

    int i;
    //memset(keys,-1,sizeof(int)*newsize);
    memset(refcounts,0,sizeof(unsigned int)*newsize);

    int oldsize=size;
    size=newsize;
    //count=0;
    int idx,att;
    for(i=0;i<oldsize;i++)
    {
      if(orefs[i]&0x80000000)
      {
        //Insert(okeys[i],oval[i]);
        att=0;
        do{
          idx=Index(okeys[i],att++);
        }while((refcounts[idx]++)&0x80000000);
        refcounts[idx]|=0x80000000;
        keys[idx]=okeys[i];
        values[idx]=oval[i];
      }
    }
    if(okeys)delete [] okeys;
    if(oval)delete [] oval;
    if(orefs)delete [] orefs;
#ifdef INTHASH_USAGE_CHECKING
    if (size > 1024 && (unsigned(count) * 128U < unsigned(size)) && id_) {
        smsc::core::synchronization::MutexGuard mg(idMutex_);
        if (id_) {
            fprintf(stderr,"inthash %s has bad usage: size=%u count=%u\n",
                    id_, unsigned(size), unsigned(count) );
            id_ = 0;
        }
    }
#endif
  }

#ifdef INTHASH_USAGE_CHECKING
  static smsc::core::synchronization::Mutex idMutex_;
  const char* id_;
#endif

}; //IntHash

#ifdef INTHASH_USAGE_CHECKING
template < class T > smsc::core::synchronization::Mutex IntHash< T >::idMutex_;
#endif

}//buffers
}//core
}//smsc

#endif
