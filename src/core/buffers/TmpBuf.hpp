#ifndef __SMSC_CORE_BUFFERS_HPP__
#define __SMSC_CORE_BUFFERS_HPP__

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
  }

  T* getSize(int size)
  {
    if(size>SZ)
    {
      if(size>heapSize)
      {
        delete [] heapBuf;
        heapBuf=new T[size];
        heapSize=size;
        realBuf=heapBuf;
      }
    }else
    {
      realBuf=stackBuf;
    }
    return realBuf;
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
};

}//namespace buffers
}//namespace core
}//namespace smsc

#endif
