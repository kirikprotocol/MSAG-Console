#include "StoreJournal.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/Message.h"
#include "core/buffers/TmpBuf.hpp"
#include "informer/io/HexDump.h"
#include "informer/io/IOConverter.h"
#include "informer/io/FileReader.h"

namespace {
const unsigned LENSIZE = 2;
const unsigned VERSIZE = 4;
const uint8_t  NEXTRESENDID = 16;
const unsigned defaultVersion = 1;

using namespace eyeline::informer;

inline std::string makePath( const std::string& storePath )
{
    return storePath + "journals/operative.journal";
}

class SJReader : public FileReader::RecordReader
{
public:
    SJReader( StoreJournal::Reader& reader, smsc::logger::Logger* thelog ) :
    reader_(reader), log_(thelog), version_(0), serial_(0) {}

    void readHeader( FileGuard& fg, const std::string& jpath )
    {
        fg.ropen(jpath.c_str());
        char buf[VERSIZE+4];
        fg.read(buf,VERSIZE+4);
        FromBuf fb(buf,VERSIZE+4);
        uint32_t v = fb.get32();
        if (v != 1) {
            throw InfosmeException(EXC_BADFILE,"file '%s': version %u is not supported",jpath.c_str(),v);
        }
        regionid_type s = fb.get32();
        if (s == 0 || serial_ == MessageLocker::lockedSerial) {
            throw InfosmeException(EXC_BADFILE,"file '%s': invalid serial number %u", s);
        }
        version_ = v;
        serial_ = s;
        smsc_log_debug(log_,"file '%s': header is ok, version=%u serial=%u",jpath.c_str(),version_,serial_);
    }

    /// check if reader is stopping
    virtual bool isStopping() {
        return reader_.isStopping();
    }
    /// return the size of record length in octets.
    virtual size_t recordLengthSize() const {
        return LENSIZE;
    }
    /// read record length from fb and checks its validity.
    virtual size_t readRecordLength( size_t filePos, FromBuf& fb ) {
        size_t rl(fb.get16());
        if ( rl > 10000 ) {
            throw InfosmeException(EXC_BADFILE,"record at %llu has invalid len: %u",
                                   ulonglong(filePos), unsigned(rl));
        }
        return rl;
    }
    /// read the record data (w/o length)
    virtual void readRecordData( size_t filePos, FromBuf& fb ) {
        const dlvid_type dlvId = fb.get32();
        const regionid_type regId = fb.get32();
        Message msg;
        uint8_t readstate = fb.get8();
        if (readstate == NEXTRESENDID) {
            const msgtime_type nextResend = fb.get32();
            if (fb.getPos() != fb.getLen()) {
                throw InfosmeException(EXC_BADFILE,"next resend record at %llu has extra data", ulonglong(filePos));
            }
            reader_.setNextResendAtInit(dlvId,regId,nextResend);
            return;
        }
        msg.state = readstate & 0x7f;
        msg.msgId = fb.get64();
        do {
            if (fb.getPos() == fb.getLen()) break;
            msg.lastTime = fb.get32();
            msg.timeLeft = fb.get32();
            msg.retryCount = fb.get16();
            if (fb.getPos() == fb.getLen()) break;
            msg.subscriber = fb.get64();
            msg.userData = fb.getCString();
            if (readstate & 0x80) {
                msg.text.reset( new MessageText(fb.getCString(),0) );
            } else {
                msg.text.reset( new MessageText(0,fb.get32()));
            }
        } while (false);
        
        if (fb.getPos() != fb.getLen()) {
            throw InfosmeException(EXC_BADFILE,"record at %llu has extra data", ulonglong(filePos));
        }
        reader_.setRecordAtInit(dlvId,regId,msg,serial_);
    }
public:
    StoreJournal::Reader& reader_;
    smsc::logger::Logger* log_;
    uint32_t              version_;
    uint32_t              serial_;
};

}


namespace eyeline {
namespace informer {

StoreJournal::StoreJournal( const CommonSettings& cs ) :
log_(smsc::logger::Logger::getInstance("storejnl")),
cs_(cs),
version_(defaultVersion),
serial_(1)
{
}


size_t StoreJournal::journalMessage( dlvid_type     dlvId,
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
        if (msg.isTextUnique()) {
            tb.set8(msg.state | 0x80); // text will be embedded
        } else {
            tb.set8(msg.state & 0x7f);
        }
        tb.set64(msg.msgId);
        if (msg.state >= uint8_t(MSGSTATE_FINAL)) {
            // final
            break;
        }
        // non-final
        tb.set32(msg.lastTime);
        tb.set32(msg.timeLeft);
        tb.set16(msg.retryCount);
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
    tb.set16(uint16_t(buflen-LENSIZE));
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
        tb.set16(uint16_t(buflen-LENSIZE));
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
    return buflen;
}


size_t StoreJournal::journalNextResend( dlvid_type dlvId,
                                        regionid_type regionId,
                                        msgtime_type nextResend )
{
    char buf[100];
    ToBuf tb(buf,sizeof(buf));
    tb.skip(LENSIZE);
    tb.set32(dlvId);
    tb.set32(regionId);
    tb.set8(NEXTRESENDID);
    tb.set32(nextResend);
    const size_t buflen = tb.getPos();
    tb.setPos(0);
    tb.set16(uint16_t(buflen-LENSIZE));
    smsc::core::synchronization::MutexGuard mg(lock_);
    if (log_->isDebugEnabled()) {
        HexDump hd;
        HexDump::string_type dump;
        dump.reserve(buflen*5);
        hd.hexdump(dump,buf,buflen);
        hd.strdump(dump,buf,buflen);
        smsc_log_debug(log_,"buffer to save(%u): %s",buflen,hd.c_str(dump));
    }
    fg_.write(buf,buflen);
    return buflen;
}


void StoreJournal::init( Reader& jr )
{
    std::string jpath = makePath(cs_.getStorePath());
    readRecordsFrom(jpath+".old",jr);
    readRecordsFrom(jpath,jr);
    if (jr.isStopping()) return;
    fg_.create(jpath.c_str(),0666,true,true);
    if ( 0 == fg_.seek(0,SEEK_END) ) {
        // new file
        version_ = defaultVersion;
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
    std::string jpath = makePath(cs_.getStorePath());
    smsc_log_info(log_,"rolling over '%s'",jpath.c_str());
    if ( -1 == rename( jpath.c_str(), (jpath + ".old").c_str() ) ) {
        throw ErrnoException(errno,"rename('%s')",jpath.c_str());
    }
    FileGuard fg;
    fg.create(jpath.c_str(),0666);
    unsigned serial = serial_;
    do {
        ++serial;
    } while (serial==0 || serial==MessageLocker::lockedSerial );
    char buf[VERSIZE+4];
    ToBuf tb(buf,VERSIZE+4);
    tb.set32(defaultVersion);
    tb.set32(serial);
    fg.write(buf,VERSIZE+4);
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        fg_.swap(fg);
        serial_ = serial;
        version_ = defaultVersion;
    }
    smsc_log_debug(log_,"file '%s' rolled",jpath.c_str());
}


void StoreJournal::readRecordsFrom( const std::string& jpath, Reader& reader )
{
    if (reader.isStopping()) return;
    FileGuard fg;
    SJReader sjreader(reader,log_);
    try {
        sjreader.readHeader(fg, jpath);
    } catch (std::exception& e) {
        smsc_log_warn(log_,"cannot read '%s': %s",jpath.c_str(),e.what());
        return;
    }
    smsc::core::buffers::TmpBuf<char,8192> buf;
    FileReader fileReader(fg);
    try {
        const size_t total = fileReader.readRecords(buf,sjreader);
        smsc_log_info(log_,"journal '%s' has been read, %u records",jpath.c_str(),unsigned(total));
        version_ = sjreader.version_;
        serial_ = sjreader.serial_;
    } catch ( FileDataException& e ) {
        smsc_log_warn(log_,"file '%s': %s", jpath.c_str(), e.what());
        // FIXME: the journal is corrupted, should we trunk the file?
        throw;
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"file '%s': %s", jpath.c_str(), e.what());
        throw;
    }
}


}
}
