#ifndef __SMSC_CORE_EXTENDING_BUFFER_HPP
#ident "@(#)$Id$"
#define __SMSC_CORE_EXTENDING_BUFFER_HPP

#include <algorithm>

namespace smsc {
namespace core {
namespace buffers {

//Similar to smsc::core::buffers::TmpBuf (that is a pure serial buffer).
//Difference is the possibility to not just append but also write data at any
//position within buffer. Reading from buffer always performed with check for
//ABR and effective data size.
//May be allocated both on stack and heap. 
//
//    0                               dataSz
//    |                                  |
//    |-------- pos --------------------->
//               |
//             Write()/Append()
// 
template <typename T, unsigned SZ /*stackBufSZ*/> class ExtendingBuffer {
protected:
    union {
      T         buf[SZ];
      uint64_t  aligner;
    } _stack;

    T *       heapBuf;
    unsigned  heapBufSz;  //actual size of allocated buffer
    T *       dataBuf;
    unsigned  pos;        //current position within buffer for reading/writing data
    unsigned  dataSz;     //effective size of data in buffer (stack or heap), starting from pos = 0

public:
    explicit ExtendingBuffer(unsigned size = SZ)
      : heapBuf(0), heapBufSz(0), dataBuf(_stack.buf), pos(0), dataSz(0)
    {
        _stack.aligner = 0;
        if (size > SZ) {
            dataBuf = heapBuf = new T[size];
            heapBufSz = size;
        }
    }

    ~ExtendingBuffer()
    { 
        if (heapBuf)
            delete [] heapBuf; 
    }

    T*   get()                  const { return dataBuf; }
    T*   getCurPtr()            const { return dataBuf + pos; }
    unsigned  getPos(void)      const { return pos; }
    unsigned  getDataSize(void) const { return dataSz; }
    unsigned  getMaxSize(void)  const { return heapBufSz ? heapBufSz : SZ; }

    //resets buffer, destroys all previously set data
    T * reset(void)
    {
        pos = dataSz = 0;
        return dataBuf;
    }
    //resets buffer, resizing it if needed, destroys all previously set data
    T* reset(unsigned size)
    {
        if (heapBuf) {
            if (size > heapBufSz) { //reallocate heap buffer
                delete [] heapBuf;
                dataBuf = heapBuf = new T[heapBufSz = size];
            }
        } else if (size > SZ) { //switch to heap buffer
            dataBuf = heapBuf = new T[heapBufSz = size];
        }
        pos = dataSz = 0;
        return dataBuf;
    }

    //Extends buffer size to hold at least the given number of elements
    //Returns true on buffer resizing
    bool extend(unsigned size)
    {
        if (heapBuf) {
            if (size > heapBufSz) { //reallocate heap buffer
                heapBuf = new T[heapBufSz = size];
                std::copy(dataBuf, dataBuf + dataSz, heapBuf);
                delete [] dataBuf;
                dataBuf = heapBuf;
                return true;
            }
        } else if (size > SZ) { //switch to heap buffer
            heapBuf = new T[heapBufSz = size];
            std::copy(dataBuf, dataBuf + dataSz, heapBuf);
            dataBuf = heapBuf;
            return true;
        }
        return false;
    }

    //resizes buffer preserving current data, truncates data in buffer if needed
    T* resize(unsigned size)
    {
        if (dataSz > size) { //truncate effective data size and position
            dataSz = size;
            if (pos > size)
                pos = size;
        } else 
            extend(size);
        return dataBuf;
    }

    //Sets current position for Read/Append operations, extending buffer
    //and effective data size if necessary
    void setPos(unsigned newpos)
    { 
        if (newpos > getMaxSize())
            extend(newpos + 1);
        if (dataSz < newpos)
            dataSz = newpos;
        pos = newpos;
    }
    //Sets effective buffer data size, extending buffer if necessary
    void setDataSize(unsigned new_dsz)
    { 
        if (new_dsz > getMaxSize())
            extend(new_dsz);
        dataSz = new_dsz;
    }

    //Reads requested number of elements at current position
    //and adjusts it if requested.
    //Checks for ABR, returns number of objects have been read.
    unsigned Read(T* dst, unsigned count, bool adjust_pos = true)
    {
        if ((pos + count) >= dataSz)
            count = dataSz - pos;
        if (count) {
            std::copy(dataBuf + pos, dataBuf + pos + count, dst);
            if (adjust_pos)
                pos += count;
        }
        return count;
    }

    //Copies requested number of elements at specified position.
    //Checks for ABR, returns number of objects have been read.
    unsigned Copy(T* dst, unsigned pos_at, unsigned count) const
    {
      if (pos_at >= dataSz)
        return 0;

      unsigned pos_end = pos_at + count;
      if (pos_end >= dataSz)
          count = dataSz - pos_at;
      if (count)
          std::copy(dataBuf + pos_at, dataBuf + pos_at + count, dst);
      return count;
    }


    //Writes data to buffer at given position, extending buffer and 
    //effective data size if necessary. Doesn't change current position!!!
    void Write(unsigned use_pos, const T* data, unsigned count)
    {
        if (!count)
            return;
        unsigned newSz = use_pos + count;
        if (newSz > getMaxSize()) {
            extend(newSz);
            dataSz = newSz;
        } else if (newSz > dataSz)
            dataSz = newSz;
        std::copy(data, data + count, dataBuf + use_pos);
    }

    //Writes/Appends data to buffer starting from current position and adjusts it.
    //NOTE: if position is set within existing data, it's overwritten.
    //Returns adjusted position.
    unsigned Append(const T* data, unsigned count)
    {
        Write(pos, data, count);
        return (pos += count);
    }

    operator T* () { return dataBuf; }

    ExtendingBuffer<T, SZ>& operator=(const ExtendingBuffer<T, SZ>& _Right)
    {
        if (this != &_Right) {
            this->reset(_Right.getMaxSize()); //pos = 0
            this->Append(_Right.get(), _Right.getDataSize());
            this->setPos(_Right.getPos());
        }
        return (*this);
    }
    ExtendingBuffer<T, SZ>& operator+=(const ExtendingBuffer<T, SZ>& _Right)
    {
        this->Append(_Right.get(), _Right.getDataSize());
        return (*this);
    }
};

}//namespace buffers
}//namespace core
}//namespace smsc

#endif /* __SMSC_CORE_EXTENDING_BUFFER_HPP */

