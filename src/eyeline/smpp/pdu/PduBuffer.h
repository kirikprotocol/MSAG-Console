#ifndef _EYELINE_SMPP_PDUBUFFER_H
#define _EYELINE_SMPP_PDUBUFFER_H

#include "util/int.h"
#include "informer/io/MemoryPool.h"
// #include "logger/Logger.h"

#define SMPPPDUUSEMEMORYPOOL 1

namespace eyeline {
namespace smpp {

class PduInputBuf;

/// this class is a holder for actual pdu buffer
class PduBuffer
{
private:
    // static smsc::logger::Logger* log_;
    // static void initLog();
public:

#ifdef SMPPPDUUSEMEMORYPOOL
    static eyeline::informer::MemoryPool pool;
    inline static eyeline::informer::MemoryPool& getPool() { return pool; }

    static inline void* operator new ( size_t sz ) {
        return pool.allocate(sz);
    }
    static inline void operator delete ( void* ptr, size_t sz ) {
        pool.deallocate(ptr,sz);
    }
#endif


    /// NOTE: this method is moved here to be able to pass buffers
    /// b/w sockets w/o decoding.
    static uint32_t getNextSeqNum();


    explicit PduBuffer() : buf_(0), bsz_(0), seqNum_(uint32_t(-1)), isreq_(false) {
        // if (!log_) { initLog(); }
    }


    /// this method is useful for passing pdu buffer b/w sockets w/o decoding
    explicit PduBuffer( const PduInputBuf& input );


    explicit PduBuffer( size_t bsz, uint32_t seqNum, bool isreq ) :
    bsz_(bsz), seqNum_(seqNum), isreq_(isreq) {
        // if (!log_) { initLog(); }
        if (bsz_) {
#ifdef SMPPPDUUSEMEMORYPOOL
            buf_ = reinterpret_cast<char*>(pool.allocate(bsz_));
#else
            buf_ = new char[bsz_]; 
#endif
            // smsc_log_debug(log_,"buf alloc %p",buf_);
        } else {
            buf_ = 0;
        }
    }

    ~PduBuffer() {
        if (buf_) {
            // smsc_log_debug(log_,"buf dealloc %p",buf_);
#ifdef SMPPPDUUSEMEMORYPOOL
            pool.deallocate(buf_,bsz_);
#else
            delete [] buf_; 
#endif
        }
    }

    inline PduBuffer( const PduBuffer& b ) :
    buf_(b.buf_), bsz_(b.bsz_), seqNum_(b.seqNum_), isreq_(b.isreq_) {
        b.buf_ = 0;
    }

    PduBuffer& operator = ( const PduBuffer& b ) {
        if ( &b != this ) {
            if ( buf_ ) {
                // smsc_log_debug(log_,"buf dealloc %p",buf_);
#ifdef SMPPPDUUSEMEMORYPOOL
                pool.deallocate(buf_,bsz_);
#else
                delete [] buf_; 
#endif
            }
            buf_ = b.buf_;
            bsz_ = b.bsz_;
            seqNum_ = b.seqNum_;
            isreq_ = b.isreq_;
            b.buf_ = 0;
        }
        return *this;
    }

    size_t getSize() const { return bsz_; }
    uint32_t getSeqNum() const { return seqNum_; }
    char*  getBuf() const { return buf_; }
    bool isRequest() const { return isreq_; }

    /// this method is useful for passing buf b/w sockets w/o decoding
    void setSeqNum( uint32_t seqNum );

private:
    mutable char* buf_;
    size_t   bsz_;
    uint32_t seqNum_;
    bool     isreq_;
};

}
}

#endif
