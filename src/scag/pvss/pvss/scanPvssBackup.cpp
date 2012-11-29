#include "logger/Logger.h"
#include "scag/pvss/profile/RollingFileStream.h"
#include "informer/io/InfosmeException.h"

using namespace scag2::pvss;
using namespace eyeline::informer;

class DummyParser : public ProfileLogStreamRecordParser
{
public:
    virtual void parseRecord( const char* record, size_t reclen ) {
        ::tm ltm;
        std::string catname;
        RollingFileStream::scanPrefix( record, reclen, ltm, catname );
    }
};


int main( int argc, char** argv )
{
    if ( argc <= 1 ) {
        fprintf(stderr,"please specify a pvss backup file\n");
        return 1;
    }

    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_INFO );

    int retval = 0;
    RollingFileStreamReader rfsr;
    try {
        DummyParser dp;
        rfsr.read( argv[1], 0, &dp );
        if ( rfsr.getLines() != rfsr.getLinesRead() ) {
            throw FileReadException("",0,size_t(-1),"not matched: linesInTail=%u lines=%u",
                                    rfsr.getLinesRead(), rfsr.getLines() );
        }
        if ( rfsr.getCrc32() != rfsr.getCrc32Read() ) {
            throw FileReadException("",0,size_t(-1),"not matched: crcInTail=%08x crc32=%08x",
                                    rfsr.getCrc32Read(), rfsr.getCrc32() );
        }
    } catch ( std::exception& e ) {
        fprintf(stderr,"ERROR: file %s read exc: %s\n",argv[1],e.what());
        return 1;
    }

    const char* next = "NEXTFILE";
    if ( ! rfsr.getNextFile().empty() ) {
        next = rfsr.getNextFile().c_str();
        fprintf(stderr,"NOTE: the file %s seems to be finished.\n", argv[1]);
    }

    fprintf(stderr,"file %s should ends with:\n",argv[1]);
    printf("# SUMMARY: lines: %u, crc32: %08x, next: %s\n",
           rfsr.getLines(), rfsr.getCrc32(), next);
    return retval;
}
