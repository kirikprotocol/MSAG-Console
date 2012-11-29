#include "logger/Logger.h"
#include "ActivityLog.h"
#include "core/synchronization/MutexReportContentionRealization.h"

using namespace eyeline::informer;

int main( int argc, const char** argv )
{
    bool zipped = false;
    int level = smsc::logger::Logger::LEVEL_INFO;
    bool loginit = false;

    int retval = 0;
    for ( const char** argp = argv+1 ; *argp != 0; ++argp ) {
        if ( !strcmp(*argp,"-h") || !strcmp(*argp,"--help")) {
            fprintf(stderr,
                    "Usage: %s [-h|--help] [--debug] [--zip|--nozip] actlog ...\n",
                    *argv );
            exit(0);
        } else if ( !strcmp(*argp,"--zip") ) {
            zipped = true;
        } else if ( !strcmp(*argp,"--nozip") ) {
            zipped = false;
        } else if ( !strcmp(*argp,"--debug") ) {
            level = smsc::logger::Logger::LEVEL_DEBUG;
        } else {
            // a path to join logs
            if (!loginit) {
                smsc::logger::Logger::initForTest
                    ( smsc::logger::Logger::LogLevel(level) );
                loginit = true;
            }
            try {
                if ( ActivityLog::fixLog( *argp, zipped ) ) {
                    fprintf(stderr,"file '%s' fixed\n",*argp);
                } else {
                    fprintf(stderr,"file '%s' checked\n",*argp);
                }
            } catch ( std::exception& e ) {
                fprintf(stderr,"EXC: file '%s' %s",*argp,e.what());
                retval = 1;
            }
        }
    }
    return retval;
}
