#ifndef __INTHASH_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
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

template <
  class T //required: T(); and operator=(const T&);
>
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
    unsigned int valIdx;
    if(!findVal(key, valIdx))
    {
      throw std::runtime_error((!size || !count) ? "IntHash::Get - empty inthash"
                                                 : "IntHash::Get - item not found");
    }
    return values[valIdx];
  }

  T& Get(int key)
  {
    unsigned int valIdx;
    if(!findVal(key, valIdx))
    {
      throw std::runtime_error((!size || !count) ? "IntHash::Get - empty inthash"
                                                 : "IntHash::Get - item not found");
    }
    return values[valIdx];
  }

  T* GetPtr(int key)const
  {
    unsigned int valIdx;
    return findVal(key, valIdx) ? &values[valIdx] : NULL;
  }

  bool Get(int key,T& value)const
  {
    unsigned int valIdx;
    if(!findVal(key, valIdx))
      return false;
    value = values[valIdx];
    return true;
  }

  bool Exist(int key)const
  {
    unsigned int valIdx;
    return findVal(key, valIdx);
  }

  bool Delete(int key)
  {
    int valIdx;
    if (!unmarkVal(key, valIdx))
      return false;
    values[valIdx] = T();
    return true;
  }

  bool Pop(int key,T& value)
  {
    int valIdx;
    if (!unmarkVal(key, valIdx))
      return false;
    value = values[valIdx];
    values[valIdx] = T();
    return true;
  }

  //Note: it's not recommended to modify cache while accessing its elements
  //via iterators. Iterator may become invalid due to insertion of new elements.
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

    bool Next(int& k,T & v)
    {
      T * pVal = NULL;
      if(!Next(k, pVal))
        return false;
      v = *pVal;
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

  //Searches for value associated with given key.
  //Returns true if value is found, false - otherwise.
  bool findVal(int use_key, unsigned int & val_idx) const
  {
    if(!size || !count)
      return false;

    int attempt=0;
    do{
      val_idx=Index(use_key,attempt);
      if(refcounts[val_idx]==0 || attempt>INTHASH_MAX_CHAIN_LENGTH)
        return false;
      ++attempt;
    }while((refcounts[val_idx]&0x80000000)==0 || keys[val_idx]!=use_key);
    return true;
  }

  //Searches for value associated with given key,
  //marks it as deleted. Additionally composes
  //intermediate array of traversed nodes refs and
  //adjusts it accordingly.
  //Returns true if value is found, false - otherwise.
  bool unmarkVal(int use_key, unsigned int & val_idx)
  {
    //printf("del:%d\n",key);fflush(stdout);
    if(!size)
      return false;

    int attempt=0;
    do{
      val_idx=Index(use_key,attempt);
      if(refcounts[val_idx]==0 || attempt>INTHASH_MAX_CHAIN_LENGTH)
        return false;

      AddRef(val_idx, attempt);
    }while((refcounts[val_idx]&0x80000000)==0 || keys[val_idx]!=use_key);

    //printf("del:ok\n");fflush(stdout);
    refcounts[val_idx] &= 0x7fffffff;
    for(int i=0;i<attempt;i++)
    {
      //printf("dele:%d\n",reflist[i]);fflush(stdout);
      refcounts[reflist[i]]--;
    }
    count--;
    return true;
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
