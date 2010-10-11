#include "informer/data/CommonSettings.h"
#include "InputJournal.h"
#include "informer/io/IOConverter.h"
#include "logger/Logger.h"
#include "core/buffers/TmpBuf.hpp"

namespace eyeline {
namespace informer {

InputJournal::InputJournal( const CommonSettings& cs ) :
cs_(cs)
{
}


void InputJournal::journalRecord( dlvid_type dlvId,
                                  regionid_type regId,
                                  const InputRegionRecord& rec,
                                  uint64_t maxMsgId )
{
    char buf[100];
    // if (version_!=1) {
    // throw InfosmeException("invalid journal version=%u",version_);
    // }
    ToBuf tb(buf,100);
    tb.skip(2); // len
    tb.set8(1); // version
    tb.set32(dlvId);
    tb.set32(regId);
    tb.set32(rec.rfn);
    tb.set32(rec.roff);
    tb.set32(rec.wfn);
    tb.set32(rec.woff);
    tb.set32(rec.count);
    tb.set64(maxMsgId);
    size_t pos = tb.getPos();
    tb.setPos(0);
    tb.set16(pos-2);
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        if (!jnl_.isOpened()) {
            jnl_.create((cs_.getStorePath() + "input/.journal").c_str(),true,true);
            jnl_.seek(0,SEEK_END);
        }
        jnl_.write(buf,pos);
    }
    jnl_.fsync();
}


void InputJournal::init( Reader& reader )
{
    std::string jpath = cs_.getStorePath() + "input/.journal";
    readRecordsFrom(jpath+".old",reader);
    readRecordsFrom(jpath,reader);
}
    

void InputJournal::readRecordsFrom( const std::string& jpath, Reader& reader )
{
    smsc::logger::Logger* log_ = smsc::logger::Logger::getInstance("injnl");
    FileGuard fg;
    try {
        fg.ropen(jpath.c_str());
    } catch (std::exception& e) {
        smsc_log_debug(log_,"journal '%s' is not found, ok",jpath.c_str());
        return;
    }

    // reading old journal
    smsc::core::buffers::TmpBuf<char,8192> buf;
    unsigned total = 0;
    do {
        char* ptr = buf.get();
        size_t wasread = fg.read(buf.GetCurPtr(), buf.getSize()-buf.GetPos());
        if (wasread == 0) {
            // EOF
            if (ptr<buf.GetCurPtr()) {
                const size_t pos = fg.getPos() - (buf.GetCurPtr()-ptr);
                throw InfosmeException("journal '%s' is not terminated at %llu",jpath.c_str(),ulonglong(pos));
            }
            break;
        }
            
        buf.SetPos(buf.GetPos()+wasread);
        while ( ptr < buf.GetCurPtr() ) {
            if ( buf.GetCurPtr()-ptr < 2 ) {
                // too few items
                break;
            }
            FromBuf fb(ptr,2);
            uint16_t reclen = fb.get16();
            if (reclen>100) {
                throw InfosmeException("journal '%s' record at %llu has invalid len: %u",
                                       jpath.c_str(), ulonglong(fg.getPos()-(buf.GetCurPtr()-ptr)), reclen );
            }
            if (ptr+2+reclen > buf.GetCurPtr()) {
                // read more
                break;
            }
            fb.setLen(2+reclen);
            const uint8_t version = fb.get8();
            if (version!=1) {
                throw InfosmeException("journal '%s' record at %llu has wrong version: %u",
                                       jpath.c_str(), ulonglong(fg.getPos()-(buf.GetCurPtr()-ptr)), version);
            }
            const dlvid_type dlvId = fb.get32();
            const regionid_type regId = fb.get32();
            InputRegionRecord rec;
            rec.rfn = fb.get32();
            rec.roff = fb.get32();
            rec.wfn = fb.get32();
            rec.woff = fb.get32();
            rec.count = fb.get32();
            const uint64_t maxMsgId = fb.get32();
            if (fb.getPos() != unsigned(reclen+2)) {
                throw InfosmeException("journal '%s' record at %llu has extra data",
                                       jpath.c_str(), ulonglong(fg.getPos()-(buf.GetCurPtr()-ptr)));
            }
            reader.setRecordAtInit(dlvId,regId,rec,maxMsgId);
            ++total;
            ptr += reclen+2;
        }
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
