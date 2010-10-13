#ifndef __SMSC_CORE_BUFFERS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_CORE_BUFFERS_HPP__

#include <algorithm>
#include "util/int.h" // for uint64_t

namespace smsc{
namespace core{
namespace buffers{

template <typename T,size_t SZ>
class TmpBuf {
protected:
  TmpBuf(const TmpBuf&);
  void operator=(const TmpBuf&);
  template <class X> void operator=(const X&);
  void* operator new(size_t sz);

  union {
    T         buf[SZ];
    uint64_t  aligner;
  } _stack;
  T*      heapBuf;
  T*      realBuf;
  size_t  heapSize;
  size_t  pos;

public:
  explicit TmpBuf(size_t size = 0)
    : heapBuf(0), realBuf(_stack.buf), heapSize(0), pos(0)
  {
    _stack.aligner = 0;
    if (size > SZ)
    {
      realBuf = heapBuf = new T[size];
      heapSize = size;
    }
  }

  /*use to read from external buffer.
    never ever even try to write to TmpBuf created with this constructor.
  */
  TmpBuf(T* extbuf,size_t extbufsize)
  {
    realBuf=extbuf;
    heapBuf=0;
    heapSize=extbufsize;
    pos=0;
  }

  void setExtBuf( T* extbuf, size_t extbufsize )
  {
    if ( heapBuf ) delete [] heapBuf;
    realBuf = extbuf;
    heapBuf = 0;
    heapSize = extbufsize;
    pos = 0;
  }


    /// NOTE: buffer content is not preserved!
  T* setSize(size_t size)
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

    /// NOTE: buffer content is preserved
  void reserve( size_t sz )
  {
    if ( sz > SZ )
    {
      if ( sz > heapSize )
      {
        T* newbuf = new T[sz];
        memcpy( newbuf, realBuf, pos );
        if (heapBuf) delete [] heapBuf;
        heapBuf = newbuf;
        heapSize = sz;
        realBuf = heapBuf;
      }
    }
  }

  size_t  getSize(void) const
  {
    return heapSize ? heapSize : SZ;
  }

  void Append(const T* data,size_t count)
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

  void SetPos(size_t newpos)
  {
    pos=newpos;
  }

  size_t GetPos() const
  {
    return pos;
  }

  void Read(T* dst,size_t count)
  {
    std::copy(realBuf+pos,realBuf+pos+count,dst);
    pos+=count;
  }

  //Checks for ABR, returns number of objects have been red
  int ReadSafe(T* dst,size_t count)
  {
    size_t maxPos = heapSize ? heapSize : SZ;

    if ((pos + count) >= maxPos)
      count = maxPos - pos;
    if (count) {
      std::copy(realBuf + pos, realBuf + pos + count, dst);
      pos += count;
    }
    return count;
  }

  T* GetCurPtr()
  {
    return realBuf+pos;
  }

  const T* GetCurPtr() const
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

  inline operator const T* () const
  {
    return realBuf;
  }

  inline T* get()
  {
    return realBuf;
  }

  inline const T* get() const
  {
    return realBuf;
  }
};

}//namespace buffers
}//namespace core
}//namespace smsc

#endif /* __SMSC_CORE_BUFFERS_HPP__ */

