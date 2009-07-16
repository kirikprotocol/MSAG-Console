#include <time.h>
#include <string>
#include <stdio.h>
#include "logger/Logger.h"

#ifdef SNMP
#include "scag/snmp/SnmpWrapper.h"
using namespace scag2::snmp;
#endif

int main( int argc, char** argv )
{
    smsc::logger::Logger::Init();

#ifdef SNMP
    std::string socket = "tcp:217.73.202.90:162";
    if ( argc > 1 ) {
        socket = argv[1];
    }
    SnmpWrapper snmp( socket );

    TrapRecord trap;
    trap.recordType = TrapRecord::Trap;
    trap.status = TrapRecord::STATNEW;
    trap.id = "echo1";
    trap.category = "SME";
    trap.severity = TrapRecord::MAJOR;
    trap.text = "entity disconnected";

    snmp.sendTrap( trap );

    struct timespec tm;
    tm.tv_sec = 5;
    tm.tv_nsec = 0;
    nanosleep( &tm, NULL );
#else
    printf( "NOTE: SNMP build property is not set, so this test does nothing\n" );
#endif
    return 0;
}
