#include "PduBuffer.h"
#include "PduInputBuf.h"
#include "core/synchronization/AtomicCounter.hpp"

namespace eyeline {
namespace smpp {

#ifdef SMPPPDUUSEMEMORYPOOL

namespace {
const size_t poolsizes[] = {
    16, 32, 64, 128,
    0x100, 0x200, 0x400, 0x800,
    0x1000, 0x2000, 0x4000, 0x8000, 0x10000,
    0
};
}

eyeline::informer::MemoryPool PduBuffer::pool(poolsizes);
#endif

// smsc::logger::Logger* PduBuffer::log_ = 0;
namespace {
// smsc::core::synchronization::Mutex lock_;
smsc::core::synchronization::AtomicCounter<uint32_t> globalSeqNum_;
}

/*
void PduBuffer::initLog()
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    if (!log_) { log_ = smsc::logger::Logger::getInstance("pdu.buf"); }
}
 */

uint32_t PduBuffer::getNextSeqNum()
{
    do {
        uint32_t res = globalSeqNum_.inc();
        if ( !(res & 0x80000000)) {
            return res;
        }
        do {
            res = globalSeqNum_.get();
            if ( !(res & 0x80000000) ) { break; }
        } while ( res != globalSeqNum_.cas(res,0) );
    } while ( true );
}


PduBuffer::PduBuffer( const PduInputBuf& input ) :
bsz_(input.size_), seqNum_(input.getSeqNum()), isreq_(input.isRequest())
{
    // if (!log_) { initLog(); }
#ifdef SMPPPDUUSEMEMORYPOOL
            buf_ = reinterpret_cast<char*>(pool.allocate(bsz_));
#else
            buf_ = new char[bsz_];
#endif
    // smsc_log_debug(log_,"buf alloc %p",buf_);
}


void PduBuffer::setSeqNum( uint32_t seqNum )
{
    seqNum_ = seqNum;
    if ( buf_ ) {
        // we do not check for buffer size and alignment here
        reinterpret_cast<uint32_t*>(buf_)[3] = htonl(seqNum);
    }
}

}
}
