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
    }else
    {
      heapBuf=0;
      realBuf=stackBuf;
    }
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
};

}//namespace buffers
}//namespace core
}//namespace smsc

#endif
