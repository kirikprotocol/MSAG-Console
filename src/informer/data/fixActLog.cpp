#include "logger/Logger.h"
#include "ActivityLog.h"
#include "core/synchronization/MutexReportContentionRealization.h"

using namespace eyeline::informer;

int main( int argc, const char** argv )
{
    bool zipped = false;
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );

    for ( const char** argp = argv+1 ; *argp != 0; ++argp ) {
        if ( !strcmp(*argp,"-h") || !strcmp(*argp,"--help")) {
            fprintf(stderr,
                    "Usage: %s [-h|--help] [--zip|--nozip] actlog ...\n",
                    *argv );
            exit(0);
        } else if ( !strcmp(*argp,"--zip") ) {
            zipped = true;
        } else if ( !strcmp(*argp,"--nozip") ) {
            zipped = false;
        } else {
            // a path to join logs
            try {
                ActivityLog::fixLog( *argp, zipped );
            } catch ( std::exception& e ) {
                fprintf(stderr,"exc: %s",e.what());
            }
        }
    }
    return 0;
}
