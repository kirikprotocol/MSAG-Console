#include "logger/Logger.h"
#include "ActivityLog.h"
#include "core/synchronization/MutexReportContentionRealization.h"

using namespace eyeline::informer;

int main( int argc, const char** argv )
{
    bool hourly = true;
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );

    for ( const char** argp = argv+1 ; *argp != 0; ++argp ) {
        if ( !strcmp(*argp,"-h") || !strcmp(*argp,"--help")) {
            fprintf(stderr,
                    "Usage: %s [-h|--help] [--hour|--day] dlvpath ...\n",
                    *argv );
            exit(0);
        } else if ( !strcmp(*argp,"--hour") ) {
            hourly = true;
        } else if ( !strcmp(*argp,"--day") ) {
            hourly = false;
        } else {
            // a path to join logs

            try {
                ActivityLog::joinLogs( *argp, hourly );
            } catch ( std::exception& e ) {
                fprintf(stderr,"exc on '%s': %s",*argp,e.what());
            }

        }
    }
    return 0;
}
