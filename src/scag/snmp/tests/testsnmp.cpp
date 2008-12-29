#include <time.h>
#include <string>
#include <stdio.h>
#include "logger/Logger.h"

#ifdef SNMP
#include "scag/snmp/SnmpWrapper.h"
using namespace scag2::snmp;
#endif

int main()
{
    smsc::logger::Logger::Init();

#ifdef SNMP
    const std::string socket = "tcp:localhost:2705";
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
