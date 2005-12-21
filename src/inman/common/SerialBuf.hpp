#ident "$Id$"
#ifndef __SMSC_CORE_SERIAL_BUFFER_HPP
#define __SMSC_CORE_SERIAL_BUFFER_HPP

#include <algorithm>

namespace smsc {
namespace core {
namespace buffers {

//Very similar to vector<>, except it's a pure serial buffer (no insertion possible)
//In opposite to Stupniks's TmpBuf, this one may be allocated both on stack and heap.
//Additionally, reading from buffer always performed with check for ABR and effective
//buffer data size.
template <typename T, int SZ> class SerialBuffer {
protected:
    T   stackBuf[SZ];
    T*  heapBuf;
    int heapBufSz;  //actual size of allocated buffer
    T*  dataBuf;
    int heapSize;   //effective size of allocated buffer
    int pos;        //current position within buffer for reading/appending data
    int dataSz;     //current size of data in buffer, starting from pos = 0

public:
    explicit SerialBuffer(int size)
    {
        if (size > SZ) {
            heapBuf = new T[size];
            dataBuf = heapBuf;
            heapBufSz = heapSize = size;
        } else {
            heapBuf = 0;
            heapBufSz = heapSize = 0;
            dataBuf = stackBuf;
        }
        pos = dataSz = 0;
    }

    ~SerialBuffer()
    { 
        if (heapBuf)
            delete [] heapBuf; 
    }

    void setPos(int newpos) { pos = newpos; }
    int  getPos(void)      const { return pos; }
    T*   get()             const { return dataBuf; }
    T*   getCurPtr()       const { return dataBuf + pos; }
    int  getDataSize(void) const { return dataSz; }
    int  getMaxSize(void)  const { return heapSize ? heapSize : SZ; }

    //Sets effective buffer data size, returns new data size (may differ from requested)
    int  setDataSize(int new_dsz)
    { 
        dataSz = (new_dsz > getMaxSize()) ? getMaxSize() : new_dsz;
        return dataSz; 
    }

    //reallocates buffer if needed, destroys all previously set data
    T* reset(int size)
    {
        if (size > SZ) {
            if (size > heapBufSz) {
                delete [] heapBuf;
                heapBuf = new T[size];
                heapBufSz = heapSize = size;
                dataBuf = heapBuf;
            }
        }
        pos = dataSz = 0;
        return dataBuf;
    }

    //resizes buffer preserving current data, truncates data in buffer if needed
    T* resize(int size)
    {
        if (!size) {
            pos = dataSz = 0;
            return dataBuf;
        }
        if (size > SZ) {
            if (size > heapSize) { //reallocate heap buffer
                if (size > heapBufSz) {
                    T* dst = new T[size];
                    std::copy(dataBuf, dataBuf + dataSz, dst);
                    delete [] heapBuf;
                    heapBufSz = heapSize = size;
                    dataBuf = heapBuf = dst;
                } else
                    heapSize = size;
            } else { //truncate heap buffer
                heapSize = size;
                if (dataSz > size)
                    dataSz = size;
            }
        } else {
            if (dataSz > size)
                dataSz = size;
        }
        return dataBuf;
    }

    //Appends data to buffer.
    //NOTE: if appending started from position within existing data,
    //the new one is written over old data, accodingly adjusting buffer data size
    void Append(const T* data, int count)
    {
        if (!count) return;

        int newSz;
        if ((newSz = (pos + count)) > getMaxSize()) {
            int maxSz = getMaxSize() << 1;
            resize((newSz > maxSz) ? newSz : maxSz);
        }
        std::copy(data, data + count, dataBuf + pos);
        dataSz = (pos += count);
    }

    //Checks for ABR, returns number of objects have been red
    int Read(T* dst, int count)
    {
        if ((pos + count) >= dataSz)
            count = dataSz - pos;
        if (count) {
            std::copy(dataBuf + pos, dataBuf + pos + count, dst);
            pos += count;
        }
        return count;
    }

    inline operator T* () { return dataBuf; }

    SerialBuffer<T, SZ>& operator=(const SerialBuffer<T, SZ>& _Right)
    {
        if (this != &_Right) {
            this->reset(_Right.getDataSize());
            this->Append(_Right.get(), _Right.getDataSize());
            this->setPos(_Right.getPos());
        }
        return (*this);
    }
};

}//namespace buffers
}//namespace core
}//namespace smsc

#endif /* __SMSC_CORE_SERIAL_BUFFER_HPP */

