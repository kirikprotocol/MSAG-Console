#include "informer/data/CommonSettings.h"
#include "InputJournal.h"
#include "informer/io/IOConverter.h"
#include "logger/Logger.h"
#include "core/buffers/TmpBuf.hpp"

namespace {
const unsigned LENSIZE = 2;
const unsigned VERSIZE = 4;
const unsigned defaultVersion = 1;
}

namespace eyeline {
namespace informer {

InputJournal::InputJournal( const CommonSettings& cs ) :
cs_(cs),
log_(smsc::logger::Logger::getInstance("injnl")),
version_(defaultVersion)
{
}


size_t InputJournal::journalRecord( dlvid_type dlvId,
                                    const InputRegionRecord& rec,
                                    uint64_t maxMsgId )
{
    char buf[100];
    ToBuf tb(buf,100);
    tb.skip(LENSIZE); // len
    tb.set32(dlvId);
    tb.set32(rec.regionId);
    tb.set32(rec.rfn);
    tb.set32(rec.roff);
    tb.set32(rec.wfn);
    tb.set32(rec.woff);
    tb.set32(rec.count);
    tb.set64(maxMsgId);
    const size_t pos = tb.getPos();
    tb.setPos(0);
    tb.set16(pos-LENSIZE);
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        jnl_.write(buf,pos);
    }
    smsc_log_debug(log_,"written record D=%u/R=%u size=%u",
                   dlvId,rec.regionId,unsigned(pos));
    jnl_.fsync();
    return pos;
}


void InputJournal::init( Reader& reader )
{
    std::string jpath = cs_.getStorePath() + "input/.journal";
    readRecordsFrom(jpath+".old",reader);
    readRecordsFrom(jpath,reader);
    // reopen journal
    if (reader.isStopping()) return;
    jnl_.create(jpath.c_str(),true,true);
    if ( 0 == jnl_.seek(0,SEEK_END) ) {
        // new file
        version_ = defaultVersion;
        smsc_log_info(log_,"creating a new file '%s', version=%u",jpath.c_str(),version_);
        char verbuf[VERSIZE];
        ToBuf tb(verbuf,VERSIZE);
        tb.set32(version_);
        jnl_.write(verbuf,VERSIZE);
    }
    reader.postInit();
    smsc_log_info(log_,"init done");
}
    

void InputJournal::rollOver()
{
    std::string jpath = cs_.getStorePath() + "input/.journal";
    smsc_log_info(log_,"rolling over '%s'",jpath.c_str());
    if ( -1 == rename( jpath.c_str(), (jpath + ".old").c_str() ) ) {
        char ebuf[100];
        throw InfosmeException("cannot rename '%s': %d, %s",
                               jpath.c_str(), errno, STRERROR(errno,ebuf,sizeof(ebuf)));
    }
    FileGuard fg;
    fg.create(jpath.c_str());
    char verbuf[VERSIZE];
    ToBuf tb(verbuf,VERSIZE);
    tb.set32(defaultVersion);
    fg.write(verbuf,VERSIZE);
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        jnl_.swap(fg);
        version_ = defaultVersion;
    }
    smsc_log_debug(log_,"file '%s' rolled",jpath.c_str());
}


void InputJournal::readRecordsFrom( const std::string& jpath, Reader& reader )
{
    if (reader.isStopping()) return;
    FileGuard fg;
    try {
        fg.ropen(jpath.c_str());
        char buf[VERSIZE];
        fg.read(buf,VERSIZE);
        FromBuf fb(buf,VERSIZE);
        uint32_t v = fb.get32();
        if ( v != 1 ) {
            throw InfosmeException("file '%s' version %u is not supported",jpath.c_str(),v);
        }
        version_ = v;
        smsc_log_debug(log_,"file '%s' header is ok, version=%u",jpath.c_str(),version_);
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
                throw InfosmeException("journal '%s' is not terminated at %llu",
                                       jpath.c_str(),ulonglong(pos));
            }
            break;
        }
            
        buf.SetPos(buf.GetPos()+wasread);
        while ( ptr < buf.GetCurPtr() ) {
            if (reader.isStopping()) return;
            if ( ptr+LENSIZE > buf.GetCurPtr()) {
                // too few items
                break;
            }
            FromBuf fb(ptr,LENSIZE);
            uint16_t reclen = fb.get16();
            if (reclen>100) {
                throw InfosmeException("journal '%s' record at %llu has invalid len: %u",
                                       jpath.c_str(), ulonglong(fg.getPos()-(buf.GetCurPtr()-ptr)), reclen );
            }
            if (ptr+LENSIZE+reclen > buf.GetCurPtr()) {
                // read more
                break;
            }
            fb.setLen(LENSIZE+reclen);
            const dlvid_type dlvId = fb.get32();
            InputRegionRecord rec;
            rec.regionId = fb.get32();
            rec.rfn = fb.get32();
            rec.roff = fb.get32();
            rec.wfn = fb.get32();
            rec.woff = fb.get32();
            rec.count = fb.get32();
            const uint64_t maxMsgId = fb.get64();
            if (fb.getPos() != unsigned(reclen+LENSIZE)) {
                throw InfosmeException("journal '%s' record at %llu has extra data",
                                       jpath.c_str(), ulonglong(fg.getPos()-(buf.GetCurPtr()-ptr)));
            }
            reader.setRecordAtInit(dlvId,rec,maxMsgId);
            ++total;
            ptr += reclen+LENSIZE;
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
