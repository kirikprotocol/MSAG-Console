/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@novosoft.ru>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

  Template class for hash table with asciiz string (char*) as a key.
  Special class (_strcon) used to store key value.

*/

#ifndef __CORE_BUFFERS_HASH_HPP__
#define __CORE_BUFFERS_HASH_HPP__

#ifndef __cplusplus
#error This header is for use with C++ only
#endif

#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdexcept>

namespace smsc{
namespace core{
namespace buffers{

typedef char hashchar;
typedef hashchar *phashstr;
typedef const hashchar* pchashstr;

class HashInvalidKeyException: public std::runtime_error
{
public:
  HashInvalidKeyException():runtime_error("HashInvalidKeyException")
  {
    
  }
  HashInvalidKeyException(const char* key):runtime_error(std::string("HashInvalidKeyException:")+key)
  {
    
  }
};

namespace _hashinternall{

class _strcon{
public:
  phashstr str;

  _strcon(pchashstr ptr)
  {
    str=strdup(ptr);
  }
  _strcon(const _strcon& ptr)
  {
    str=strdup(ptr.str);
  }
  _strcon()
  {
    str=NULL;
  }
  ~_strcon()
  {
    if(str)
    {
      free(str);
    }
    str=NULL;
  }

  void operator=(phashstr ptr)
  {
    if(str)free(str);
    str=strdup(ptr);
  }
  void operator=(const _strcon& ptr)
  {
    if(str)free(str);
    str=strdup(ptr.str);
  }
  int operator==(const _strcon& ptr){return str?!strcmp(str,ptr.str):0;}
  int operator==(const _strcon& ptr)const {return str?!strcmp(str,ptr.str):0;}
  int operator==(pchashstr ptr){return str?!strcmp(str,ptr):0;}
  int operator==(pchashstr ptr)const{return str?!strcmp(str,ptr):0;}
  operator phashstr(){return str;}
  operator phashstr() const {return str;}
};
}

static inline unsigned HashFunc(pchashstr key)
{
  phashstr curr = (phashstr)key;
  unsigned count = *curr;
  while(*curr)
  {
    count += 37 * count + *curr;
    curr++;
  }
  count=(unsigned)(( ( count * (unsigned)19L ) + (unsigned)12451L ) % (unsigned)8882693L);
  return count;
}


template <class T>
class HashKeyVal{
public:
  _hashinternall::_strcon _key;
  T _value;
  unsigned int _hashsum;
  HashKeyVal<T>(const HashKeyVal& keyval):_key(keyval._key),_value(keyval._value)
  {
    _hashsum=keyval._hashsum;
  };
  HashKeyVal<T>(pchashstr key,const T& value):_key(key),_value(value)
  {
    _hashsum=HashFunc(key);
  };
  HashKeyVal<T>(pchashstr key):_key(key)
  {
    _hashsum=HashFunc(key);
  };
};

template <class T>
class HashListLink{
public:
  HashKeyVal<T> _keyval;
  HashListLink<T> *_next;
  HashListLink<T>(pchashstr key,const T& value):_keyval(key,value){_next=NULL;};
  HashListLink<T>(pchashstr key):_keyval(key){_next=NULL;}
};

template <class T>
class HashList{
protected:
  typedef HashListLink<T> Link;
  typedef HashKeyVal<T> KeyVal;
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
  HashList<T>(){_head=NULL;_tail=NULL;}
  ~HashList<T>()
  {
    Empty();
  }

  Link* Add(pchashstr key,const T& value)
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

  Link* Add(pchashstr key)
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

  Link* Find(pchashstr key)
  {
    Link *tmp=_head;
    while(tmp)
    {
      if(tmp->_keyval._key==key)return tmp;
      tmp=tmp->_next;
    }
    return NULL;
  }

  int Remove(pchashstr key)
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

template <class T>
class Hash{
protected:
  typedef HashList<T> List;
  typedef HashListLink<T> Link;
private:
  List *_buckets;
  int _bucketsnum;

  int _count;

  int _iterindex;
  Link *_iterlink;

  Link* FindLink(pchashstr key)const
  {
    if(!_bucketsnum || !_count)return NULL;
    return _buckets[HashFunc(key) % _bucketsnum].Find(key);
  }
  Link* FindLinkEx(pchashstr key,unsigned &index)
  {
    if(!_bucketsnum)return NULL;
    index=HashFunc(key) % _bucketsnum;
    if(!_count)return NULL;
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
        newbuckets[(unsigned)p->_keyval._hashsum % (unsigned)newbucketsnum].Append(p);
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
    typedef T value_type;

  Hash<T>(int initbucketsnum=0)
  {
    if(initbucketsnum>=0)_bucketsnum=initbucketsnum;
    if(_bucketsnum>0)
    {
      _buckets=new List[_bucketsnum];
    }else
    {
      _buckets=NULL;
    }
    _count=0;
  }
  Hash<T>(const Hash<T>& src)
  {
    _buckets=NULL;
    Assign(src);
  }
  virtual ~Hash<T>()
  {
    if(_buckets)delete [] _buckets;
  };


  int Exists(pchashstr key)const
  {
    if(_count==0)return 0;
    return FindLink(key)!=NULL;
  }

  void Delete(pchashstr key)
  {
    if(_count==0) return;
    unsigned index=HashFunc(key) % _bucketsnum;
    if(_buckets[index].Remove(key))_count--;
  }

  bool Pop(pchashstr key, T& val)
  {
    if(!_bucketsnum || !_count) return false;
    List& list = _buckets[HashFunc(key) % _bucketsnum];
    Link* link = list.Find(key);
    if (!link) return false;
    val = link->_keyval._value;
    list.Remove(key);
    return true;
  }

  void operator=(const Hash<T>& src)
  {
    Assign(src);
  }

  void Assign(const Hash<T>& src)
  {
    int i;
    Link *lnk;
    if(_buckets)delete [] _buckets;
    _bucketsnum=src._bucketsnum;
    _buckets=new List[_bucketsnum];
    for(i=0;i<_bucketsnum;i++)
    {
      if( (lnk=src._buckets[i].First()) )
      {
        do{
          _buckets[i].Add(*lnk);
        }while( (lnk=_buckets[i].Next(lnk)) );
      }
    }
    _count=src._count;
  }

  const T& Get(pchashstr key)const
  {
    Link* link=FindLink(key);
    if(!link)throw HashInvalidKeyException(key);
    return link->_keyval._value;
  }

  T& Get(pchashstr key)
  {
    Link* link=FindLink(key);
    if(!link)throw HashInvalidKeyException(key);
    return link->_keyval._value;
  }

  const T* GetPtr(pchashstr key)const
  {
    Link* link=FindLink(key);
    if(!link)return 0;
    return &link->_keyval._value;
  }
  T* GetPtr(pchashstr key)
  {
    Link* link=FindLink(key);
    if(!link)return 0;
    return &link->_keyval._value;
  }

  T& operator[](pchashstr key)
  {
    unsigned index;
    Link *link=FindLinkEx(key,index);
    if(link)
    {
      return link->_keyval._value;
    }else
    {
      _count++;
      if(ResizeHash())index=HashFunc(key) % _bucketsnum;
      return _buckets[index].Add(key)->_keyval._value;
    }
  }
  T const & operator[](pchashstr key)const
  {
    Link *link=FindLink(key);
    if(link)
    {
      return link->_keyval._value;
    }else
    {
      throw HashInvalidKeyException(key);
    }
  }
  int Insert(pchashstr key,const T& value)
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
      if(ResizeHash())index=HashFunc(key) % _bucketsnum;
      _buckets[index].Add(key)->_keyval._value=value;
      return 1;
    }
  }

  inline T* SetItem(pchashstr key,const T& value)
  {
    unsigned index;
    Link *link=FindLinkEx(key,index);
    if(link)
    {
      link->_keyval._value=value;
      return &link->_keyval._value;
    }else
    {
      _count++;
      if(ResizeHash())index=HashFunc(key) % _bucketsnum;
      T* v=&_buckets[index].Add(key)->_keyval._value;
      *v=value;
      return v;
    }
  }

  void First(){_iterindex=0;_iterlink=NULL;};

  class Iterator{
    int _index;
    Link *_link;
    const Hash *_owner;
  public:
    Iterator(const Hash* owner):_owner(owner)
    {
      _index=0;
      _link=NULL;
    }
    void First(){_index=0;_link=NULL;}
    int Next(phashstr& key,T& value)
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
    int Next(phashstr& key,T*& value)
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
    return this;
  }

  int Next(phashstr& key,T& value)
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
  int Next(phashstr& key,T*& value)
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

  int GetCount() const {return _count;}
  int GetBucketsCount() const {return _bucketsnum;}
  int GetUsage()
  {
    int cnt=0;
    for(int i=0;i<_bucketsnum;i++)
    {
      if(_buckets[i].First())cnt++;
    }
    return cnt;
  }
  void Empty()
  {
    if(_buckets)delete [] _buckets;
    _buckets=NULL;
    _bucketsnum=0;
    _count=0;
  }
};


}//buffers
}//core
}//smsc


#endif
