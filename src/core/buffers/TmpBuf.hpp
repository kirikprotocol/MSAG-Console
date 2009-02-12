#ifndef __SMSC_CORE_BUFFERS_HPP__
#define __SMSC_CORE_BUFFERS_HPP__
#ident "$Id$"

#include <algorithm>

namespace smsc{
namespace core{
namespace buffers{

template <typename T,size_t SZ>
class TmpBuf{
public:
    TmpBuf() : heapBuf(0), realBuf(stackBuf), heapSize(0), pos(0) {
    }

  explicit TmpBuf(size_t size)
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

  /*use to read from external buffer.
    never ever even try to write to TmpBuf created with this constructor.
  */
  TmpBuf(T* extbuf,int extbufsize)
  {
    realBuf=extbuf;
    heapBuf=0;
    heapSize=extbufsize;
    pos=0;
  }

    void setExtBuf( T* extbuf, int extbufsize ) {
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
        if ( sz > SZ ) {
            if ( sz > heapSize ) {
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

protected:
  TmpBuf(const TmpBuf&);
  void operator=(const TmpBuf&);
  template <class X> void operator=(const X&);
  void* operator new(size_t sz);
  T  stackBuf[SZ];
  T* heapBuf;
  T* realBuf;
  size_t heapSize;
  size_t pos;
};

}//namespace buffers
}//namespace core
}//namespace smsc

#endif
