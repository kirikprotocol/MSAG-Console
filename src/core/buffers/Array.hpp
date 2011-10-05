/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@novosoft.ru>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

  Template class for quantified array.
*/

#ifndef __CORE_BUFFERS_ARRAY_HPP__
#define __CORE_BUFFERS_ARRAY_HPP__

#ifndef __cplusplus
#error This header is for use with C++ only
#endif

#include <new>
#include <stdexcept>

namespace smsc{
namespace core{
namespace buffers{


template <class T>
class Array{
protected:
  char* _data;
  int _count;
  int _size;
  int _empty;

  inline int SZ(){return sizeof(T);};

  inline void CallDestructor(T* obj)
  {
    if( obj != 0 ) obj->~T();
  }

  inline T* Ptr(int index=0)
  {
    return ((T*)_data)+index;
  }
  inline T& Obj(int index=0)
  {
    return *Ptr(index);
  }

  inline void DestroyRange(int from,int to)
  {
    for(int i=from;i<to;i++)CallDestructor(Ptr(i));
  }

  inline void InitRange(int from,int to)
  {
    for(int i=from;i<to;i++)new(Ptr(i))T();
  }

  void Resize(int newsize)
  {
    if(newsize<=_size)return;
    if(newsize<=_size+_empty)
    {
      _data-=_empty*SZ();
      Copy(_empty,0,_count);
      _size+=_empty;
      _empty=0;
      return;
    }
    newsize=Increment(newsize);
    char* newdata=new char[SZ()*newsize];
    int i;
    for(i=0;i<_count;i++)
    {
      new(((T*)newdata)+i)T(Obj(i));
    }
    _size=newsize;
    DestroyRange(0,_count);
    delete [] (_data-_empty*SZ());
    _empty=0;
    _data=newdata;
  }
  int Increment(int oldsize)
  {
    if(oldsize<16)return oldsize+4;
    if(oldsize<256)return oldsize+16;
    return oldsize+oldsize/4;
  }
  void Copy(int from,int to,int count)
  {
    if(from>to)
    {
      for(int i=0;i<count;i++)
      {
        new(Ptr(to+i))T(Obj(from+i));
        CallDestructor(Ptr(from+i));
      }
    }else
    {
      for(int i=count-1;i>=0;i--)
      {
        new(Ptr(to+i))T(Obj(from+i));
        CallDestructor(Ptr(from+i));
      }
    }
  }
public:
  Array():_data(0),_count(0),_size(0),_empty(0){}
  Array(int n):_count(0),_size(n),_empty(0)
  {
    _data=new char[SZ()*n];
  }
  Array(const Array<T>& src)
  {
    _data=new char[SZ()*src.Count()];
    int i;
    for(i=0;i<src.Count();i++)
    {
      new((T*)_data+i)T(src[i]);
    }
    _size=src.Count();
    _count=_size;
    _empty=0;
  }
  virtual ~Array()
  {
    Clean();
  }

  void operator=(const Array<T>& src)
  {
    if(this==&src)return;
    Clean();
    _data=new char[SZ()*src.Count()];
    int i;
    for(i=0;i<src.Count();i++)
    {
      new(Ptr(i))T(src[i]);
    }
    _size=src.Count();
    _count=_size;
  }

  T& operator[](int index)
  {
    if(index<0)index+=_count;
    if(index>=_size)Resize(index+1);
    if(index>=_count)
    {
      InitRange(_count,index+1);
      _count=index+1;
    }
    return Obj(index);
  }
  const T& operator[](int index)const
  {
    if(index<0)index+=_count;
    if(index>=_count)throw std::runtime_error("Array:Index out of bounds");
    return const_cast<const T&>((const_cast<Array<T>*>(this))->Obj(index));
  }

  int Push(const T& item)
  {
    Resize(_count+1);
    new(Ptr(_count))T(item);
    _count++;
    return _count;
  }

  int Pop(T& item)
  {
    if(_count==0)return 0;
    _count--;
    item=Obj(_count);
    CallDestructor(Ptr(_count));
    return _count;
  }

  int Unshift(const T& item)
  {
    if(_empty)
    {
      _data-=SZ();
      _empty--;
      new(Ptr())T(item);
      _size++;
      _count++;
    }else
    {
      Resize(_count+1);
      Copy(0,1,_count);
      new(Ptr())T(item);
      _count++;
    }
    return _count;
  }

  int Shift(T& item)
  {
    if(_count==0)return 0;
    item=Obj();
    CallDestructor(Ptr());
    _data+=SZ();
    _empty++;
    _size--;
    _count--;
    return _count;
  }

  int Insert(int index,const T& item)
  {
    if(index<0)index+=_count;
    if(index<0)return -1;
    if(index>_count)
    {
      Resize(index+1);
      InitRange(_count,index);
      new(Ptr(index))T(item);
      _count=index+1;
      return _count;
    }
    Resize(_count+1);
    Copy(index,index+1,_count-index);
    new(Ptr(index))T(item);
    _count++;
    return _count;
  }

  int Insert(int index,const Array<T>& src)
  {
    if(index<0)index+=_count;
    if(index<0)return -1;
    if(index>_count)
    {
      Resize(index+src.Count());
      int i;
      InitRange(_count,index);
      for(i=0;i<src.Count();i++)
      {
        new(Ptr(index+i))T(src[i]);
      }
      _count=index+src.Count();
    }else
    {
      Resize(_count+src.Count());
      Copy(index,index+src.Count(),src.Count());
      int i;
      for(i=0;i<src.Count();i++)
      {
        new(Ptr(index+i))T(src[i]);
      }
      _count+=src.Count();
    }
    return _count;
  }

  int Delete(int index,int count=1)
  {
    if(index<0)index+=_count;
    if(index<0)
    {
      if (count==-1)
      {
        count=_count;
        index=0;
      }else
      {
        count-=index;
        index=0;
      }
    }else
    {
      if(count==-1)count=_count-index;
    }
    if(index>=_count)return 0;
    if(index+count>=_count)
    {
      DestroyRange(index,_count);
      _count=index;
      return _count-index;
    }
    DestroyRange(index,index+count);
    Copy(index+count,index,_count-(index+count));
    _count-=count;
    return count;
  }

  void Reverse()
  {
    int i;
    char *newdata=new char[SZ()*_size];
    for(i=0;i<_count;i++)
    {
      new(((T*)newdata)+i)T(Obj(_count-1-i));
    }
    DestroyRange(0,_count);
    delete [] (_data-_empty*SZ());
    _empty=0;
    _data=newdata;
  }

  // int Count(){return _count;}
  int Count()const{return _count;}

  void Clean()
  {
    if(_data)
    {
      DestroyRange(0,_count);
      delete [] (_data-_empty*SZ());
    }
    _data=0;
    _size=0;
    _count=0;
    _empty=0;
  }

  void Empty()
  {
    if(_count)
    {
      DestroyRange(0,_count);
      _count=0;
      _data-=_empty*SZ();
      _size+=_empty;
      _empty=0;
    }
  }

  int SetSize(int count)
  {
    int oldsize=_size;
    Resize(count);
    return oldsize;
  }

  int Shrink()
  {
    char *newdata=new char[_count*SZ()];
    int i;
    for(i=0;i<_count;i++)
    {
      new(((T*)newdata)+i)T(Obj(i));
    }
    DestroyRange(0,_count);
    delete [] (_data-_empty*SZ());
    int oldsize=_size;
    _size=_count;
    _data=newdata;
    return oldsize;
  }

  void Init(int cnt)
  {
    Resize(cnt);
    if(cnt>_count)
    {
      InitRange(_count,cnt);
      _count=cnt;
    }

  }

};//Array

}//buffers
}//core
}//smsc

#endif
