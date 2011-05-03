#include "informer/data/CommonSettings.h"
#include "InputJournal.h"
#include "informer/io/IOConverter.h"
#include "logger/Logger.h"
#include "informer/io/TmpBuf.h"
#include "informer/io/FileReader.h"
#include "informer/data/InputRegionRecord.h"

namespace {

using namespace eyeline::informer;

const unsigned LENSIZE = 2;
const unsigned VERSIZE = 4;
const unsigned defaultVersion = 1;

inline std::string makePath( const std::string& storePath )
{
    return storePath + "journals/input.journal";
}

class IJReader : public FileReader::RecordReader
{
public:
    IJReader( InputJournal::Reader& reader, uint32_t version ) : reader_(reader), version_(version) {}

    virtual bool isStopping() { return getCS()->isStopping(); }
    /// return the size of record length in octets.
    virtual size_t recordLengthSize() const { return LENSIZE; }
    /// read record length from fb and checks its validity.
    virtual size_t readRecordLength( size_t filePos, FromBuf& fb ) {
        const size_t rl(fb.get16());
        if ( rl > 100 ) {
            throw InfosmeException(EXC_BADFILE,"record at %llu has invalid len: %u",
                                   ulonglong(filePos), unsigned(rl));
        }
        return rl;
    }
    /// read the record data (w/o length)
    virtual bool readRecordData( size_t filePos, FromBuf& fb ) {
        const dlvid_type dlvId = fb.get32();
        InputRegionRecord rec;
        rec.regionId = fb.get32();
        rec.rfn = fb.get32();
        rec.roff = fb.get32();
        rec.rlast = fb.get64();
        rec.wfn = fb.get32();
        rec.woff = fb.get32();
        rec.count = fb.get32();
        const uint64_t maxMsgId = fb.get64();
        reader_.setRecordAtInit(dlvId,rec,maxMsgId);
        return true;
    }
private:
    InputJournal::Reader& reader_;
    uint32_t              version_;
};

}

namespace eyeline {
namespace informer {

InputJournal::InputJournal() :
log_(smsc::logger::Logger::getInstance("injnl")),
lock_( MTXWHEREAMI ),
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
    tb.set64(rec.rlast);
    tb.set32(rec.wfn);
    tb.set32(rec.woff);
    tb.set32(rec.count);
    tb.set64(maxMsgId);
    const size_t pos = tb.getPos();
    tb.setPos(0);
    tb.set16(uint16_t(pos-LENSIZE));
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        jnl_.write(buf,pos);
    }
    smsc_log_debug(log_,"written record R=%u/D=%u size=%u",
                   rec.regionId,
                   dlvId,
                   unsigned(pos));
    jnl_.fsync();
    return pos;
}


void InputJournal::init( Reader& reader )
{
    std::string jpath = makePath(getCS()->getStorePath());
    readRecordsFrom(jpath+".old",reader);
    readRecordsFrom(jpath,reader);
    // reopen journal
    if (getCS()->isStopping()) return;
    jnl_.create(jpath.c_str(),0666,true,true);
    if ( 0 == jnl_.seek(0,SEEK_END) ) {
        // new file
        version_ = defaultVersion;
        smsc_log_debug(log_,"creating a new file '%s', version=%u",jpath.c_str(),version_);
        char verbuf[VERSIZE];
        ToBuf tb(verbuf,VERSIZE);
        tb.set32(version_);
        jnl_.write(verbuf,VERSIZE);
    }
    reader.postInit();
    smsc_log_debug(log_,"init done");
}
    

void InputJournal::rollOver()
{
    std::string jpath = makePath(getCS()->getStorePath());
    smsc_log_debug(log_,"rolling over '%s'",jpath.c_str());
    if ( -1 == rename( jpath.c_str(), (jpath + ".old").c_str() ) ) {
        const int err = errno;
        // check if file original file exists
        struct stat st;
        if ( -1 == ::stat( jpath.c_str(), &st ) ) { return; }
        throw ErrnoException(err,"rename('%s')",jpath.c_str());
    }
    FileGuard fg;
    fg.create(jpath.c_str(),0666);
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
    if (getCS()->isStopping()) return;
    FileGuard fg;
    try {
        fg.ropen(jpath.c_str());
        char buf[VERSIZE];
        fg.read(buf,VERSIZE);
        FromBuf fb(buf,VERSIZE);
        uint32_t v = fb.get32();
        if ( v != 1 ) {
            throw InfosmeException(EXC_BADFILE,
                                   "file '%s' version %u is not supported",
                                   jpath.c_str(),v);
        }
        version_ = v;
        smsc_log_debug(log_,"file '%s' header is ok, version=%u",jpath.c_str(),version_);
    } catch (std::exception& e) {
        smsc_log_debug(log_,"journal '%s' is not found, ok",jpath.c_str());
        return;
    }

    // reading old journal
    TmpBuf<char,8192> buf;
    FileReader fileReader(fg);
    IJReader ijreader(reader,version_);
    try {
        const size_t total = fileReader.readRecords(buf,ijreader);
        smsc_log_info(log_,"journal '%s' has been read, %u records",jpath.c_str(),unsigned(total));
    } catch ( FileDataException& e ) {
        smsc_log_warn(log_,"file '%s' exc: %s", jpath.c_str(), e.what());
        throw;
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"file '%s' exc: %s", jpath.c_str(), e.what());
        throw;
    }
}

}
}
