#include <assert.h>
#include "Socket.h"
#include "SessionBase.h"
#include "eyeline/smpp/SmppException.h"
#include "SmppWriter.h"
#include "eyeline/smpp/pdu/Pdu.h"
#include "eyeline/smpp/pdu/GenericNackPdu.h"
#include "eyeline/smpp/pdu/EnquireLinkPdu.h"
#include "eyeline/smpp/pdu/PduInputBuf.h"
#include "informer/io/IOConverter.h"
#include "informer/io/HexDump.h"
#include "system/status.h"

using namespace smsc::core::synchronization;
using namespace eyeline::informer;

namespace eyeline {
namespace smpp {

unsigned Socket::globalSocketId_ = 0;
smsc::core::synchronization::AtomicCounter<unsigned> Socket::total_(0);

smsc::logger::Logger* Socket::log_ = 0;

namespace {
Mutex socketMutex;
Mutex logMutex;

/*
void checkHeap() {}
{
    struct PB {
        char*    buf;
        size_t   bsz;
        uint32_t seq;
        bool     isreq;
    };
    struct BM {
        void* subs;
        PB    buf;
    };
    BM *newdata = new BM[16];
    assert(newdata);
    fprintf(stderr,"heap checked newdata=%p sz=%u\n",newdata,unsigned(sizeof(BM)*16));
    delete [] newdata;
}
 */


}

unsigned Socket::getGlobalSocketId()
{
    MutexGuard mg(socketMutex);
    if (!++globalSocketId_) {
        ++globalSocketId_;
    }
    return globalSocketId_;
}


void Socket::initLog()
{
    MutexGuard mg(logMutex);
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("smpp.sock");
    }
}


Socket::Socket( smsc::core::network::Socket* s,
                size_t maxrsize, size_t maxwqueue ) :
socket_(s),
ref_(0),
socketId_(getGlobalSocketId()),
maxrsize_(maxrsize),
maxwqueue_(maxwqueue),
lastread_(currentTimeMillis()),
bindmode_(BindMode(0)),
shutdown_(false),
wpos_(0),
writeIdx_(0),
rpos_(0),
rlen_(0)
{
    if (!socket_) {
        socket_ = new smsc::core::network::Socket();
    }
    wpos_ = 0;
    if (!log_) { initLog(); }
    socket_->setData(0,this);
    const unsigned total = total_.inc();
    smsc_log_debug(log_,"ctor Sk%u @%p total=%u",socketId_,this,total);
}


Socket::~Socket()
{
    const unsigned total = total_.dec();
    smsc_log_debug(log_,"dtor Sk%u @%p total=%u",socketId_,this,total);
    delete socket_;
}


SessionBasePtr Socket::getSession()
{
    // mutex?
    MutexGuard mg(writeMutex_);
    return session_;
}


eyeline::informer::EmbedRefPtr< SmppWriter > Socket::getWriter()
{
    MutexGuard mg(writeMutex_);
    return writer_;
}


void Socket::setSession( SessionBase* session )
{
    // mutex?
    smsc_log_debug(log_,"Sk%u setSession(S'%s' @%p)",
                   socketId_,session ? session->getSessionId() : "",session);
    MutexGuard mg(writeMutex_);
    session_.reset(session);
}


void Socket::setWriter( SmppWriter* writer )
{
    smsc_log_debug(log_,"Sk%u setWriter(%p)",socketId_,writer);
    {
        MutexGuard mg(writeMutex_);
        writer_.reset(writer);
    }
    if ( writer ) { writer->wakeUp(); }
}


void Socket::send( PduBuffer pdu, SendSubscriber* subs )
{
    {
        MutexGuard mg(writeMutex_);
        if (shutdown_) {
            throw SmppException("Sk%u is shutting down",socketId_);
        } else if ( !pdu.getSize() || !pdu.getBuf() ) {
            throw SmppException("Sk%u buffer is empty",socketId_);
        }

        // checkHeap();
        if ( pdu.isRequest() ) {
            const size_t wqsz = pducount_.inc();
            if ( wqsz > maxwqueue_ ) {
                pducount_.dec();
                throw SmppException("Sk%u too long write queue %u",
                                    socketId_, unsigned(wqsz));
            }
        }

        // checkHeap();
        if (log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type dump;
            hd.hexdump(dump,pdu.getBuf(),pdu.getSize());
            smsc_log_debug(log_,"Sk%u send seq=%u: %s",
                           socketId_, unsigned(pdu.getSeqNum()), hd.c_str(dump));
        }

        // checkHeap();
        BufferMark& mark = wbuf_[writeIdx_].Push( BufferMark() );
        mark.subs = subs;
        mark.buf = pdu;
        if (writer_.get()) { writer_->wakeUp(); }
    }
}


// void Socket::setReadWriteMode( int rw )
// {
//     smsc_log_debug(log_,"sock=%u is setting rwmode=%d",socketId_,rw);
//     rwmode_ = rw;
// }


void Socket::setBindMode( BindMode bm )
{
    smsc_log_debug(log_,"Sk%u set bind=%s",socketId_,bindModeToString(bm));
    bindmode_ = bm;
}


bool Socket::setShutdown()
{
    bool res = true;
    do {
        MutexGuard mg(writeMutex_);
        shutdown_ = true;
        if ( wbuf_[0].Count() || wbuf_[1].Count() ) {
            res = false;
            break;
        }
        // socket has no data, close it
        socket_->Close();
        bindmode_ = BindMode(0);
    } while (false);
    return res;
}


void Socket::unsetShutdown()
{
    MutexGuard mg(writeMutex_);
    shutdown_ = false;
    lastread_ = currentTimeMillis();
    socket_->setNonBlocking(0);
}


void Socket::processInput()
{
    // could be invoked safely as it belongs to only one reader
    bool hadread = false;
    do {

        smsc_log_debug(log_,"procIn: rlen=%u bpos=%u rpos=%u",
                       unsigned(rlen_),
                       unsigned(rbuf_.getPos()),
                       unsigned(rpos_));

        if ( rlen_ <= 4 ) {

            if ( rbuf_.getPos() - rpos_ >= 4 ) {
                // get the length
                eyeline::informer::FromBuf fb(rbuf_.get()+rpos_,4);
                rlen_ = size_t(fb.get32());
                if ( rlen_ > maxrsize_ ) {
                    throw SmppException("too large pkgsize=%u > %u",
                                        unsigned(rlen_), unsigned(maxrsize_));
                } else if ( rlen_ < sizeof(uint32_t)*4 ) {
                    throw SmppException("too small pkgsize=%u < %u",
                                        unsigned(rlen_), unsigned(sizeof(uint32_t)*4));
                }
                continue;
            }

        } else if ( rpos_ + rlen_ <= rbuf_.getPos() ) {

            // the full pkg could be parsed
            try {

                const PduInputBuf inputBuf(rlen_,rbuf_.get()+rpos_);
                lastread_ = currentTimeMillis();

                // FIXME: pre-process well known types
                switch (PduType(inputBuf.getPduTypeId())) {
                case ENQUIRE_LINK: {
                    std::auto_ptr<Pdu> resp(inputBuf.createResponse());
                    send(resp->encode(),0);
                    break;
                }
                case ENQUIRE_LINK_RESP: break;
                case GENERIC_NACK : {
                    smsc_log_warn(log_,"Sk%u generic nack obtained, shutting down",socketId_);
                    // FIXME: think about what to do with gnack
                    setShutdown();
                    break;
                }
                default: {
                    SessionBasePtr sb( getSession() );
                    if ( !sb ) {
                        smsc_log_debug(log_,"Sk%u pdu listener is not set",socketId_);
                    } else {
                        sb->receivePdu(*this,inputBuf);
                    }
                }
                } // switch

            } catch ( std::exception& e ) {
                // FIXME: discriminate on the exception type
                addGNack(Status::SYSERR);
                smsc_log_warn(log_,"Sk%u exc at decode: %s",socketId_,e.what());
            }
            rpos_ += rlen_;
            rlen_ = 0;
            continue;

        }

        // need to read more data into buffer
        if ( rpos_ > 0 ) {
            if ( rpos_ < rbuf_.getPos() ) {
                // need to shift the buffer
                for ( char* o = rbuf_.get(), *i = rbuf_.get()+rpos_, *e = rbuf_.getCurPtr();
                      i != e; ) {
                    *o++ = *i++;
                }
            }
            rbuf_.setPos( rbuf_.getPos() - rpos_ );
            rpos_ = 0;
        } else if ( rbuf_.getSize() - rbuf_.getPos() < 20 ) {
            // need to resize the buffer
            rbuf_.reserve(rbuf_.getSize()*2 + 32);
        }

        if (hadread) break; // cannot read twice (need poll)

        ssize_t wasread = socket_->Read(rbuf_.getCurPtr(),unsigned(rbuf_.getSize()-rbuf_.getPos()));
        if ( wasread == 0 ) {
            smsc_log_debug(log_,"Sk%u : EOF",socketId_);
            // addGNack(Status::INVCMDLEN);
            throw SmppException("EOF");
        } else if ( wasread < 0 ) {
            smsc_log_debug(log_,"Sk%u : read failed %u",socketId_,errno);
            socket_->Close();
            bindmode_ = BindMode(0);
            // addGNack(Status::INVCMDLEN);
            char ebuf[100];
            throw SmppException("read: %s",STRERROR(errno,ebuf,sizeof(ebuf)));
        }
        hadread = true;
        rbuf_.setPos(rbuf_.getPos() + wasread);
        if (log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type dump;
            hd.hexdump(dump,rbuf_.get()+rpos_,rbuf_.getPos()-rpos_);
            smsc_log_debug(log_,"Sk%u : read %u bytes, buffer: %s",
                           socketId_,unsigned(wasread),hd.c_str(dump));
        }

    } while ( true );
}


bool Socket::hasWriteData()
{
    bool res = true;
    do {
        unsigned widx = 1-writeIdx_;
        if ( wbuf_[widx].Count() ) {
            break;
        }
        MutexGuard mg(writeMutex_);
        // switch buffers
        widx = writeIdx_;
        writeIdx_ = 1 - writeIdx_;
        wpos_ = 0;
        res = wbuf_[widx].Count();
    } while (false);
    smsc_log_debug(log_,"Sk%u hasWriteData=%u",socketId_,res);
    return res;
}


bool Socket::sendData()
{
    try {

        if (!hasWriteData()) return false;

        // widx is safe to work with
        const unsigned widx = 1-writeIdx_;
        PduBuffer& pdu = wbuf_[widx].Front().buf;
        const size_t towrite = pdu.getSize() - wpos_;
        if (log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type dump;
            hd.hexdump(dump,pdu.getBuf()+wpos_,towrite);
            smsc_log_debug(log_,"Sk%u writing %u bytes: %s",
                           socketId_,unsigned(towrite),hd.c_str(dump));
        }
        int res = socket_->Write(pdu.getBuf()+wpos_,unsigned(towrite));
        if ( res == -1 ) {
            char ebuf[100];
            throw SmppException("failed to write: %s",STRERROR(errno,ebuf,sizeof(ebuf)));
        }

        const size_t wp = (wpos_ += res);
        if ( wp >= pdu.getSize() ) {
            if (pdu.isRequest()) {
                pducount_.dec();
            }
            SendSubscriber* subs = wbuf_[widx].Front().subs;
            if ( subs ) {
                subs->wasSent(*this,pdu.getSeqNum(),pdu.isRequest());
            }
            // move to the next buffer
            wpos_ = 0;
            wbuf_[widx].Pop();
        }
        return hasWriteData();

    } catch ( const std::exception& e ) {
        setShutdown();
        for ( int pass = 0; pass < 2; ++pass ) {
            const unsigned widx = 1-writeIdx_;
            while ( wbuf_[widx].Count() ) {
                BufferMark& mark = wbuf_[widx].Front();
                if ( mark.subs ) {
                    mark.subs->failedToSend(*this,
                                            mark.buf.getSeqNum(),
                                            mark.buf.isRequest(),
                                            &e);
                }
                wbuf_[widx].Pop();
            }
            wpos_ = 0;
            hasWriteData(); // it will swap buffers
        }
        pducount_.set(0);
        throw;
    }
}


void Socket::addGNack( int status )
{
    try {
        uint32_t seqNum = 0;
        if ( rbuf_.getPos() - rpos_ >= 16 ) {
            eyeline::informer::FromBuf fb(rbuf_.get()+rpos_+12,4);
            seqNum = fb.get32();
        }
        GenericNackPdu pdu(status,seqNum);
        send(pdu.encode(),0);
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"Sk%u: exc cannot send gnack: %s",socketId_,e.what());
    }
}

}
}
