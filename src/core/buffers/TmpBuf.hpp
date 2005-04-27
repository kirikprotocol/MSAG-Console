#ifndef __SMSC_CORE_BUFFERS_HPP__
#define __SMSC_CORE_BUFFERS_HPP__

#include <algorithm>

namespace smsc{
namespace core{
namespace buffers{

template <typename T,int SZ>
class TmpBuf{
public:
  explicit TmpBuf(int size)
  {
    if(size>SZ)
    {
      heapBuf=new T[size];
      realBuf=heapBuf;
      heapSize=size;
    }else
    {
      heapBuf=0;
      heapSize=0;
      realBuf=stackBuf;
    }
    pos=0;
  }

  T* setSize(int size)
  {
    if(size>SZ)
    {
      if(size>heapSize)
      {
        if(heapBuf)delete [] heapBuf;
        heapBuf=new T[size];
        heapSize=size;
        realBuf=heapBuf;
      }
    }
    return realBuf;
  }

  void Append(const T* data,int count)
  {
    if(count==0)return;
    if(pos+count>SZ)
    {
      if(heapBuf)
      {
        if(pos+count>heapSize)
        {
          heapSize=(pos+count)*2;
          T* tmp=new T[heapSize];
          std::copy(realBuf,realBuf+pos,tmp);
          delete [] heapBuf;
          heapBuf=tmp;
          realBuf=tmp;
        }
      }else
      {
        heapSize=(pos+count)*2;
        heapBuf=new T[heapSize];
        std::copy(realBuf,realBuf+pos,heapBuf);
        realBuf=heapBuf;
      }
    }
    std::copy(data,data+count,realBuf+pos);
    pos+=count;
  }

  void SetPos(int newpos)
  {
    pos=newpos;
  }

  int GetPos()
  {
    return pos;
  }

  void Read(T* dst,int count)
  {
    std::copy(realBuf+pos,realBuf+pos+count,dst);
    pos+=count;
  }

  T* GetCurPtr()
  {
    return realBuf+pos;
  }


  ~TmpBuf()
  {
    if(heapBuf)delete [] heapBuf;
  }

  inline operator T* ()
  {
    return realBuf;
  }

  inline T* get()
  {
    return realBuf;
  }

protected:
  TmpBuf(const TmpBuf&);
  void operator=(const TmpBuf&);
  template <class X> void operator=(const X&);
  void* operator new(size_t sz);
  T  stackBuf[SZ];
  T* heapBuf;
  T* realBuf;
  int heapSize;
  int pos;
};

}//namespace buffers
}//namespace core
}//namespace smsc

#endif
