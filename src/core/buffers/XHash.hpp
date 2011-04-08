/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@novosoft.ru>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

  Template class for hash table.

*/

#ifndef __XHASH_HPP__
#define __XHASH_HPP__

#ifndef __cplusplus
#error This header is for use with C++ only
#endif

#include <string.h>
#include <stdlib.h>
#include <exception>
//#include "util/debug.h"
namespace smsc{
namespace core{
namespace buffers{

class XHashInvalidKeyException: public std::exception
{
public:
  const char* what()const throw()
  {
    return "XHashInvalidKeyException";
  }
};

class DefXHashFunc{
public:
  template <class T>
  static inline unsigned int CalcHash(T key)
  {
    return (unsigned int)key;
  }
};


template <class KT,class VT,class HF=DefXHashFunc>
class XHash{
protected:

class XHashKeyVal{
public:
  KT _key;
  VT _value;
  unsigned int _hash;
  XHashKeyVal(const XHashKeyVal& keyval):
    _key(keyval._key),
    _value(keyval._value),
    _hash(keyval._hash)
  {
  };
  XHashKeyVal(const KT& key,const VT& value):_key(key),_value(value)
  {
    _hash=HF::CalcHash(_key);
  };
  XHashKeyVal(const KT& key):_key(key)
  {
    _hash=HF::CalcHash(_key);
  };
};

class XHashListLink{
public:
  XHashKeyVal _keyval;
  XHashListLink *_next;
  XHashListLink(const KT& key,const VT& value):_keyval(key,value){_next=NULL;};
  XHashListLink(const KT& key):_keyval(key){_next=NULL;}
};

class XHashList{
protected:
  typedef XHashListLink Link;
  typedef XHashKeyVal KeyVal;
private:
  Link *_head,*_tail;
  void Empty()
  {
    while(_head)
    {
      _tail=_head->_next;
      delete _head;
      _head=_tail;
    }
  }
public:
  XHashList(){_head=NULL;_tail=NULL;}
  ~XHashList()
  {
    Empty();
  }

  Link* Add(const KT& key,const VT& value)
  {
    if(_head)
    {
      _tail->_next=new Link(key,value);
      _tail=_tail->_next;
    }else
    {
      _head=new Link(key,value);
      _tail=_head;
    }
    return _tail;
  }

  Link* Add(const Link& link)
  {
    if(_head)
    {
      _tail->_next=new Link(link);
      _tail=_tail->_next;
    }else
    {
      _head=new Link(link);
      _tail=_head;
    }
    return _tail;
  }

  Link* Add(const KT& key)
  {
    if(_head)
    {
      _tail->_next=new Link(key);
      _tail=_tail->_next;
    }else
    {
      _head=new Link(key);
      _tail=_head;
    }
    return _tail;
  }

  void Append(Link* link)
  {
    if(_head)
    {
      _tail->_next=link;
      _tail=link;
      _tail->_next=NULL;
    }else
    {
      _head=link;
      _tail=_head;
      _tail->_next=NULL;
    }
  }

  Link* Find(const KT& key)
  {
    Link *tmp=_head;
    while(tmp)
    {
      if(tmp->_keyval._key==key)return tmp;
      tmp=tmp->_next;
    }
    return NULL;
  }

  int Remove(KT key)
  {
    Link *tmp=_head,*last=NULL;
    while(tmp)
    {
      if(tmp->_keyval._key==key)
      {
        if(last)
        {
          last->_next=tmp->_next;
          if(_tail==tmp)_tail=last;
          delete tmp;
        }else
        {
          _head=_head->_next;
          if(_tail==tmp)_tail=_head;
          delete tmp;
        }
        return 1;
      }
      last=tmp;
      tmp=tmp->_next;
    }
    return 0;
  }
  Link *First(){return _head;};
  Link *Next(Link* link){if(link)return link->_next; else return NULL;};


  void Reset(int dofree)
  {
    if(dofree)
    {
      Empty();
    }else
    {
      _head=NULL;
    }
  }
};


  typedef XHashList List;
  typedef XHashListLink Link;
private:
  List *_buckets;
  int _bucketsnum;

  int _count;

  int _iterindex;
  Link *_iterlink;

  Link* FindLink(const KT& key)const
  {
    if(_count==0)return NULL;
    return _buckets[((unsigned int)HF::CalcHash(key)) % _bucketsnum].Find(key);
  }
  Link* FindLinkEx(const KT& key,unsigned &index)
  {
    if(_bucketsnum==0)return NULL;
    index=((unsigned int)HF::CalcHash(key)) % _bucketsnum;
    if(_count==0)return NULL;
    return _buckets[index].Find(key);
  }

  int ResizeHash()
  {
    if(_count<_bucketsnum)return 0;
    int newbucketsnum=_bucketsnum==0?8:_bucketsnum*2;
    List *newbuckets=new List[newbucketsnum];
    Link *p,*q;
    for(int i=0;i<_bucketsnum;i++)
    {
      p=_buckets[i].First();
      while(p)
      {
        q=_buckets[i].Next(p);
        newbuckets[(unsigned)p->_keyval._hash % (unsigned)newbucketsnum].Append(p);
        p=q;
      }
      _buckets[i].Reset(0);
    }
    if(_buckets)delete [] _buckets;
    _buckets=newbuckets;
    _bucketsnum=newbucketsnum;
    return 1;
  }

public:
  XHash(int initbucketsnum=0)
  {
    if(initbucketsnum>=0)
      _bucketsnum=initbucketsnum;
    else
      _bucketsnum=0;

    if(_bucketsnum>0)
    {
      _buckets=new List[_bucketsnum];
    }else
    {
      _buckets=NULL;
    }
    _count=0;
  }
  XHash(const XHash& src)
  {
    _buckets=NULL;
    Assign(src);
  }
  virtual ~XHash(){if(_buckets)delete [] _buckets;};


  int Exists(const KT& key){ return FindLink(key)!=NULL;}
  int Delete(const KT& key)
  {
    if(_count==0)return 0;
    unsigned index=((unsigned int)HF::CalcHash(key)) % _bucketsnum;
    if(_buckets[index].Remove(key))
    {
      _count--;
      return 1;
    }else
    {
      return 0;
    }
  }

  XHash& operator=(const XHash& src)
  {
    Assign(src);
    return *this;
  }

  void Assign(const XHash& src)
  {
    int i;
    Link *lnk;
    if(_buckets)delete [] _buckets;
    _bucketsnum=src._bucketsnum;
    _buckets=new List[_bucketsnum];
    for(i=0;i<_bucketsnum;i++)
    {
      if((lnk=src._buckets[i].First()))
      {
        do{
          _buckets[i].Add(*lnk);
        }while((lnk=_buckets[i].Next(lnk)));
      }
    }
    _count=src._count;
  }

  const VT& Get(const KT& key)const
  {
    Link* link=FindLink(key);
    if(!link)throw XHashInvalidKeyException();
    return link->_keyval._value;
  }

  VT& Get(const KT& key)
  {
    Link* link=FindLink(key);
    if(!link)throw XHashInvalidKeyException();
    return link->_keyval._value;
  }
  const VT* GetPtr(const KT& key)const
  {
    Link* link=FindLink(key);
    if(!link)return 0;
    return &link->_keyval._value;
  }
  VT* GetPtr(const KT& key)
  {
    Link* link=FindLink(key);
    if(!link)return 0;
    return &link->_keyval._value;
  }
  int Get(const KT& key,VT& value)const
  {
    //__trace2__("MAP:: XHash value addr 0x%x",&value);
    Link* link=FindLink(key);
    //__trace2__("MAP:: XHash link 0x%x",link);
    if(!link)return 0;
    value=link->_keyval._value;
    /*char b[256] = {0,};
    int k = 0;
    for ( int i=0; i<sizeof(link->_keyval); ++i){
      k += sprintf(b+k,"%x ",((const unsigned char*)(&link->_keyval))[i]);
    }*/
    //__trace2__("MAP:: XHash _value %s",b);
    //__trace2__("MAP:: XHash _value 0x%x",link->_keyval._value);
    //__trace2__("MAP:: XHash value 0x%x",value);
    return 1;
  }

  VT& operator[](const KT& key)
  {
    unsigned index;
    Link *link=FindLinkEx(key,index);
    if(link)
    {
      return link->_keyval._value;
    }else
    {
      _count++;
      if(ResizeHash())index=((unsigned int)HF::CalcHash(key)) % _bucketsnum;
      return _buckets[index].Add(key)->_keyval._value;
    }
  }
  VT const & operator[](const KT& key)const
  {
    Link *link=FindLink(key);
    if(link)
    {
      return link->_keyval._value;
    }else
    {
      throw XHashInvalidKeyException();
    }
  }
  int Insert(const KT& key,const VT& value)
  {
    unsigned index;
    Link *link=FindLinkEx(key,index);
    if(link)
    {
      link->_keyval._value=value;
      return 0;
    }else
    {
      _count++;
      if(ResizeHash())index=((unsigned int)HF::CalcHash(key)) % _bucketsnum;
      _buckets[index].Add(key)->_keyval._value=value;
      return 1;
    }
  }

  VT& InsertEx(const KT& key,const VT& value)
  {
    unsigned index;
    Link *link=FindLinkEx(key,index);
    if(link)
    {
      link->_keyval._value=value;
    }else
    {
      _count++;
      if(ResizeHash())index=((unsigned int)HF::CalcHash(key)) % _bucketsnum;
      link=_buckets[index].Add(key);
      link->_keyval._value=value;
    }
    return link->_keyval._value;
  }

  void First(){_iterindex=0;_iterlink=NULL;};

  class Iterator{
    int _index;
    Link *_link;
    const XHash *_owner;
  public:
    Iterator(const XHash* owner):_owner(owner)
    {
      _index=0;
      _link=NULL;
    }
    void First(){_index=0;_link=NULL;}
    int Next(KT& key,VT& value)
    {
      if(_index>=_owner->_bucketsnum)return 0;
      if(!_link)
      {
        while((_link=_owner->_buckets[_index].First())==NULL)
        {
          _index++;
          if(_index>=_owner->_bucketsnum)return 0;
        }
      }
      key=_link->_keyval._key;
      value=_link->_keyval._value;
      _link=_owner->_buckets[_index].Next(_link);
      if(!_link)_index++;
      return 1;
    }
    int Next(KT& key,VT*& value)
    {
      if(_index>=_owner->_bucketsnum)return 0;
      if(!_link)
      {
        while((_link=_owner->_buckets[_index].First())==NULL)
        {
          _index++;
          if(_index>=_owner->_bucketsnum)return 0;
        }
      }
      key=_link->_keyval._key;
      value=&_link->_keyval._value;
      _link=_owner->_buckets[_index].Next(_link);
      if(!_link)_index++;
      return 1;
    }
  };
  friend class Iterator;

  Iterator getIterator()const
  {
    return Iterator(this);
  }

  int Next(KT& key,VT& value)
  {
    if(_iterindex>=_bucketsnum)return 0;
    if(!_iterlink)
    {
      while((_iterlink=_buckets[_iterindex].First())==NULL)
      {
        _iterindex++;
        if(_iterindex>=_bucketsnum)return 0;
      }
    }
    key=_iterlink->_keyval._key;
    value=_iterlink->_keyval._value;
    _iterlink=_buckets[_iterindex].Next(_iterlink);
    if(!_iterlink)_iterindex++;
    return 1;
  }
  int Next(KT& key,VT*& value)
  {
    if(_iterindex>=_bucketsnum)return 0;
    if(!_iterlink)
    {
      while((_iterlink=_buckets[_iterindex].First())==NULL)
      {
        _iterindex++;
        if(_iterindex>=_bucketsnum)return 0;
      }
    }
    key=_iterlink->_keyval._key;
    value=&_iterlink->_keyval._value;
    _iterlink=_buckets[_iterindex].Next(_iterlink);
    if(!_iterlink)_iterindex++;
    return 1;
  }

  int Count() const {return _count;}
  int Usage()
  {
    int cnt=0;
    for(int i=0;i<_bucketsnum;i++)
    {
      if(_buckets[i].First())cnt++;
    }
    return 100*cnt/_bucketsnum;
  }
  void GetStats(int& maxchain,int& usedbuckets,int& totalbuckets)
  {
    maxchain=0;
    usedbuckets=0;
    totalbuckets=_bucketsnum;
    for(int i=0;i<_bucketsnum;i++)
    {
      Link* l=_buckets[i].First();
      if(l)
      {
        usedbuckets++;
        int cnt=1;
        while((l=_buckets[i].Next(l)))cnt++;
        if(cnt>maxchain)maxchain=cnt;
      }
    }

  }
  void Clean()
  {
    if(_buckets)delete [] _buckets;
    _buckets=NULL;
    _bucketsnum=0;
    _count=0;
  }
  void Empty()
  {
    Clean();
  }
};

}
}
}

#endif
