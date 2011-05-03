#ifndef _INFORMER_TMPBUF_H
#define _INFORMER_TMPBUF_H

#include <algorithm>
#include <string.h>
#include "util/int.h" // for uint64_t

namespace eyeline {
namespace informer {

template < typename T > class TmpBufBase
{
protected:
    inline TmpBufBase(T* stackBuf, size_t sz) :
        stackBuf_(stackBuf), heapBuf(0), realBuf(stackBuf),
        SZ(sz), heapSize(0), pos(0) {}

    inline ~TmpBufBase() {
        if (heapBuf) delete [] heapBuf;
    }

public:

    inline void setPos( size_t newpos ) { pos = newpos; }
    inline size_t getPos() const { return pos; }

    T* getCurPtr() { return realBuf+pos; }
    const T* getCurPtr() const { return realBuf+pos; }

    inline T* get() { return realBuf; }
    inline const T* get() const { return realBuf; }

    inline size_t getSize() const { return heapSize ? heapSize : SZ; }


    void setExtBuf( T* extBuf, size_t extBufSz )
    {
        if ( heapBuf ) delete [] heapBuf;
        realBuf = extBuf;
        heapBuf = 0;
        heapSize = extBufSz;
        pos = 0;
    }


    /// NOTE: buffer contents is not preserved
    T* setSize( size_t size )
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
                std::copy( realBuf, realBuf+pos, newbuf );
                if (heapBuf) delete [] heapBuf;
                heapBuf = newbuf;
                heapSize = sz;
                realBuf = heapBuf;
            }
        }
    }


    void append( const T* data, size_t count )
    {
        if (count==0) return;
        if (pos+count>SZ) {
            if (heapBuf) {
                if (pos+count>heapSize) {
                    heapSize = (pos+count)*2;
                    T* tmp = new T[heapSize];
                    std::copy(realBuf,realBuf+pos,tmp);
                    delete [] heapBuf;
                    heapBuf=tmp;
                    realBuf=tmp;
                }
            } else {
                heapSize=(pos+count)*2;
                heapBuf=new T[heapSize];
                std::copy(realBuf,realBuf+pos,heapBuf);
                realBuf=heapBuf;
            }
        }
        std::copy(data,data+count,realBuf+pos);
        pos+=count;
    }


    void read( T* dst, size_t count ) {
        std::copy(realBuf+pos,realBuf+pos+count,dst);
        pos += count;
    }


    // deprecated
    inline void Append( const T* data, size_t count ) {
        append(data,count);
    }
    inline void SetPos( size_t newpos ) {
        setPos(newpos);
    }
    inline size_t GetPos() const {
        return getPos();
    }
    inline void Read( T* dst, size_t count ) {
        read(dst,count);
    }
    inline T* GetCurPtr() { return getCurPtr(); }
    inline const T* GetCurPtr() const { return getCurPtr(); }

protected:
    // forbidden
    TmpBufBase(const TmpBufBase&);
    TmpBufBase& operator = (const TmpBufBase&);
    template <class X> void operator = (const X&);
    void* operator new(size_t sz);

protected:
    T*      stackBuf_;
    T*      heapBuf;
    T*      realBuf;
    const size_t SZ;
    size_t  heapSize;
    size_t  pos;

};


template < typename T, size_t SIZE > class TmpBuf : public TmpBufBase< T >
{
public:
    explicit TmpBuf( size_t size = 0 ) : TmpBufBase< T >(stack_.buf,SIZE)
    {
        stack_.aligner = 0;
        if (size > SIZE) {
            this->realBuf = this->heapBuf = new T[size];
            this->heapSize = size;
        }
    }

    /// used to read from external buffer
    /// NOTE: never try to write to TmpBuf created with this ctor.
    TmpBuf( T* extbuf, size_t extBufSize ) : TmpBufBase< T >(stack_.buf,SIZE) {
        setExtBuf(extbuf,extBufSize);
    }

protected:
    union {
        T buf[SIZE];
        uint64_t aligner;
    } stack_;
};
    
}
}

#endif /* __SMSC_CORE_BUFFERS_HPP__ */

