#include "StoreJournal.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/Message.h"
#include "core/buffers/TmpBuf.hpp"
#include "informer/io/HexDump.h"
#include "informer/io/IOConverter.h"

namespace {
const unsigned LENSIZE = 2;
const unsigned VERSIZE = 4;
}


namespace eyeline {
namespace informer {

StoreJournal::StoreJournal( const CommonSettings& cs ) :
log_(smsc::logger::Logger::getInstance("storejnl")),
cs_(cs),
version_(1),
serial_(1)
{
}


void StoreJournal::journalMessage( dlvid_type     dlvId,
                                   regionid_type  regionId,
                                   const Message& msg,
                                   regionid_type& serial )
{
    smsc::core::buffers::TmpBuf<unsigned char,200> buf;
    const bool equalSerials = (serial == serial_);
    if (!equalSerials && msg.isTextUnique()) {
        // need to write text
        buf.setSize(90+strlen(msg.text->getText()));
    }
    ToBuf tb(buf.get(),buf.getSize());
    do {
        tb.skip(LENSIZE);
        tb.set32(dlvId);
        tb.set32(regionId);
        tb.set64(msg.msgId);
        if (msg.isTextUnique()) {
            tb.set8(msg.state | 0x80); // text will be embedded
        } else {
            tb.set8(msg.state & 0x7f);
        }
        if (msg.state >= uint8_t(MSGSTATE_FINAL)) {
            // final
            break;
        }
        // non-final
        tb.set32(msg.lastTime);
        tb.set32(msg.timeLeft);
        if (equalSerials) {
            // we have already written the message to this file
            break;
        }
        tb.set64(msg.subscriber);
        tb.setCString(msg.userData.c_str());
        if (msg.isTextUnique()) {
            tb.setCString(msg.text->getText());
        } else {
            tb.set32(msg.text->getTextId());
        }
    } while (false);

    // now compare
    size_t buflen = tb.getPos();
    tb.setPos(0);
    tb.set16(buflen-LENSIZE);
    smsc::core::synchronization::MutexGuard mg(lock_);
    if (msg.state < uint8_t(MSGSTATE_FINAL) &&
        equalSerials && serial != serial_ ) {
        // oops, the serial has changed while we were preparing the buffer
        if (msg.isTextUnique()) {
            buf.reserve(90+strlen(msg.text->getText()));
            tb.setBuf(buf.get(),buf.getSize());
        }
        tb.setPos(buflen);
        tb.set64(msg.subscriber);
        tb.setCString(msg.userData.c_str());
        if (msg.isTextUnique()) {
            tb.setCString(msg.text->getText());
        } else {
            tb.set32(msg.text->getTextId());
        }
        buflen = tb.getPos();
        tb.setPos(0);
        tb.set16(buflen-LENSIZE);
    }
    if (log_->isDebugEnabled()) {
        HexDump hd;
        HexDump::string_type dump;
        dump.reserve(buflen*5);
        hd.hexdump(dump,buf.get(),buflen);
        hd.strdump(dump,buf.get(),buflen);
        smsc_log_debug(log_,"buffer to save(%u): %s",buflen,hd.c_str(dump));
    }
    fg_.write(buf.get(),buflen);
    serial = serial_;
}


void StoreJournal::init( Reader& jr )
{
    std::string jpath = cs_.getStorePath() + "operative/.journal";
    readRecordsFrom(jpath+".old",jr);
    readRecordsFrom(jpath,jr);
    if (jr.isStopping()) return;
    fg_.create(jpath.c_str(),true,true);
    if ( 0 == fg_.seek(0,SEEK_END) ) {
        // new file
        version_ = 1;
        do {
            ++serial_;
        } while (serial_==0 || serial_==MessageLocker::lockedSerial );
        char buf[VERSIZE+4];
        ToBuf tb(buf,VERSIZE+4);
        tb.set32(version_);
        tb.set32(serial_);
        fg_.write(buf,VERSIZE+4);
    }
    jr.postInit();
}


void StoreJournal::rollOver()
{
    smsc_log_debug(log_,"FIXME: rolling over store journal");
}


void StoreJournal::readRecordsFrom( const std::string& jpath, Reader& reader )
{
    if (reader.isStopping()) return;
    FileGuard fg;
    try {
        fg.ropen(jpath.c_str());
        char buf[VERSIZE+4];
        fg.read(buf,VERSIZE+4);
        FromBuf fb(buf,VERSIZE+4);
        uint32_t v = fb.get32();
        if (v != 1) {
            throw InfosmeException("file '%s': version %u is not supported",jpath.c_str(),v);
        }
        regionid_type s = fb.get32();
        if (s == 0 || serial_ == MessageLocker::lockedSerial) {
            throw InfosmeException("file '%s': invalid serial number %u", s);
        }
        version_ = v;
        serial_ = s;
        smsc_log_debug(log_,"file '%s': header is ok, version=%u serial=%u",jpath.c_str(),version_,serial_);
    } catch (std::exception& e) {
        smsc_log_warn(log_,"cannot read '%s': %s",jpath.c_str(),e.what());
        return;
    }

    smsc::core::buffers::TmpBuf<char,8192> buf;
    unsigned total = 0;
    do {
        char* ptr = buf.get();
        size_t wasread = fg.read(buf.GetCurPtr(),buf.getSize()-buf.GetPos());
        if (wasread == 0) {
            // EOF
            if (ptr < buf.GetCurPtr()) {
                const size_t pos = fg.getPos() - (buf.GetCurPtr()-ptr);
                throw InfosmeException("journal '%s' is not terminated at %llu",
                                       jpath.c_str(), ulonglong(pos));
            }
            break;
        }
        
        buf.SetPos(buf.GetPos()+wasread);
        while (ptr < buf.GetCurPtr()) {
            if (reader.isStopping()) return;
            if (ptr+LENSIZE > buf.GetCurPtr()) {
                // too few items
                break;
            }
            FromBuf fb(ptr,LENSIZE);
            uint16_t reclen = fb.get16();
            if (reclen>10000) {
                throw InfosmeException("journal '%s' record at %llu has invalid len: %u",
                                       jpath.c_str(), ulonglong(fg.getPos()-(buf.GetCurPtr()-ptr)),reclen);
            }
            if (ptr+LENSIZE+reclen > buf.GetCurPtr()) {
                // read more
                break;
            }
            fb.setLen(LENSIZE+reclen);
            const dlvid_type dlvId = fb.get32();
            const regionid_type regId = fb.get32();
            Message msg;
            msg.msgId = fb.get64();
            uint8_t readstate = fb.get8();
            msg.state = readstate & 0x7f;
            do {
                if (fb.getPos() == unsigned(reclen+LENSIZE)) break;
                msg.lastTime = fb.get32();
                msg.timeLeft = fb.get32();
                if (fb.getPos() == unsigned(reclen+LENSIZE)) break;
                msg.subscriber = fb.get64();
                msg.userData = fb.getCString();
                if (readstate & 0x80) {
                    msg.text.reset( new MessageText(fb.getCString(),0) );
                } else {
                    msg.text.reset( new MessageText(0,fb.get32()));
                }
            } while (false);
            
            if (fb.getPos() != unsigned(reclen+LENSIZE)) {
                throw InfosmeException("journal '%s' record at %llu has extra data",
                                       jpath.c_str(), ulonglong(fg.getPos()-(buf.GetCurPtr()-ptr)));
            }
            reader.setRecordAtInit(dlvId,regId,msg,serial_);

            ++total;
            ptr += reclen+LENSIZE;
        } // while more record in read chunk
        if (ptr>buf.get()) {
            // shifting buffer back
            char* o = buf.get();
            const char* i = ptr;
            const char* e = buf.GetCurPtr();
            for ( ; i < e ; ) {
                *o++ = *i++;
            }
            buf.SetPos(o-buf.get());
        } else if ( buf.GetPos() >= buf.getSize() ) {
            // resize needed
            buf.reserve(buf.getSize()+buf.getSize()/2+100);
        }
    } while (true);
    smsc_log_info(log_,"journal '%s' has been read, %u records",jpath.c_str(),total);
}


}
}
