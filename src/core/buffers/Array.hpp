/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@novosoft.ru>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

  Template class for quantified array.
*/

#ifndef __ARRAY_HPP__
#define __ARRAY_HPP__

#ifndef __cplusplus
#error This header is for use with C++ only
#endif

#include <new.h>

template <class T>
class Array{
protected:
  T* _data;
  int _count;
  int _size;
  int _empty;
  void Resize(int newsize)
  {
    if(newsize<=_size)return;
    newsize=Increment(newsize);
    T* newdata=(T*)new char[sizeof(T)*newsize];
    int i;
    for(i=0;i<_count;i++)
    {
      new(newdata+i)T(_data[i]);
    }
    _size=newsize;
    delete [] (_data-_empty);
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
        _data[to+i]=_data[from+i];
      }
    }else
    {
      for(int i=count-1;i>=0;i--)
      {
        _data[to+i]=_data[from+i];
      }
    }
  }
public:
  Array(){_data=0;_count=0;_size=0;_empty=0;}
  Array(int n){_data=new T[n];_count=0;_size=n;_empty=0;}
  Array(const Array<T>& src)
  {
    _data=(T*)new char[sizeof(T)*src.Count()];
    int i;
    for(i=0;i<src.Count();i++)
    {
      new(_data+i)T(src[i]);
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
    _data=(T*)new char[sizeof(T)*src.Count()];
    int i;
    for(i=0;i<src.Count();i++)
    {
      new(_data+i)T(src[i]);
    }
    _size=src.Count();
    _count=_size;
  }

  T& operator[](int index)
  {
    if(index<0)index+=_count;
    if(index>=_size)Resize(index+1);
    if(index>=_count)_count=index+1;
    return _data[index];
  }
  const T& operator[](int index)const
  {
    if(index<0)index+=_count;
    if(index>=_count)return *((T*)0);
    return _data[index];
  }

  int Push(const T& item)
  {
    Resize(_count+1);
    _data[_count]=item;
    _count++;
    return _count;
  }

  int Pop(T& item)
  {
    if(_count==0)return 0;
    _count--;
    item=_data[_count];
    return _count;
  }

  int Unshift(const T& item)
  {
    if(_empty)
    {
      _data--;
      _empty--;
      _data[0]=item;
      _size++;
      _count++;
    }else
    {
      Resize(_count+1);
      Copy(0,1,_count);
      _data[0]=item;
      _count++;
    }
    return _count;
  }

  int Shift(T& item)
  {
    if(_count==0)return 0;
    item=_data[0];
    _data++;
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
      _data[index]=item;
      _count=index+1;
      return _count;
    }
    Resize(_count+1);
    Copy(index,index+1,_count-index);
    _data[index]=item;
    _count++;
    return _count;
  }

  int Insert(int index,const Array<T>& src,int count=-1)
  {
    if(count==-1)count=src.Count();
    if(index<0)index+=_count;
    if(index<0)return -1;
    if(index>_count)
    {
      Resize(index+src.Count());
      int i;
      for(i=0;i<src.Count();i++)
      {
        _data[index+i]=src[i];
      }
      _count=index+src.Count();
    }else
    {
      Resize(_count+src.Count());
      Copy(index,index+src.Count(),src.Count());
      int i;
      for(i=0;i<src.Count();i++)
      {
        _data[index+i]=src[i];
      }
      _count+=src.Count();
    }
    return _count;
  }

  int Delete(int index,int count=-1)
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
      _count=index;
      return _count-index;
    }
    Copy(index+count,index,_count-(index+count));
    _count-=count;
    return count;
  }

  void Reverse()
  {
    int i;
    T* newdata=(T*)new char[sizeof(T)*_size];
    for(i=0;i<_count;i++)
    {
      new(newdata+i)T(_data[_count-1-i]);
    }
    delete [] (_data-_empty);
    _empty=0;
    _data=newdata;
  }

  int Count(){return _count;}
  int Count()const{return _count;}

  void Clean()
  {
    if(_data)delete [] (_data-_empty);
    _data=0;
    _size=0;
    _count=0;
    _empty=0;
  }

  int SetSize(int count)
  {
    int oldsize=_size;
    Resize(count);
    return oldsize;
  }

};

#endif
