//
// Dmitry Bukin.
// PageFile cleanup:
// 1. the correction of free pages chain;
// 2. the correction of unfinished used chains;

#include <memory>
#include "core/buffers/PageFile.hpp"
using smsc::core::buffers::File;

#include "logger/Logger.h"

int main( int argc, char** argv )
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* slog = smsc::logger::Logger::getInstance("main");

    if ( argc < 2 ) {
        fprintf( stderr, "Usage: %s PAGEFILENAME", argv[0] );
        return 1;
    }
    
    // opening a pagefile
    std::auto_ptr< smsc::core::buffers::PageFile > pf( new smsc::core::buffers::PageFile );

    pf->Open( argv[1] );
    
    File::offset_type pageIdx = 0;
    std::vector< unsigned char > data;
    while ( true ) {
        File::offset_type realIdx;
        File::offset_type nextIdx = pf->Read( pageIdx, data, &realIdx);
        smsc_log_debug( slog, "read %llx, real=%llx, next=%llx, data=%lu",
                        static_cast<long long>(pageIdx),
                        static_cast<long long>(realIdx),
                        static_cast<long long>(nextIdx),
                        static_cast<unsigned long>(data.size()) );
        if ( nextIdx == 0 ) break;
        pageIdx = nextIdx;
    }
    return 0;
}
    
