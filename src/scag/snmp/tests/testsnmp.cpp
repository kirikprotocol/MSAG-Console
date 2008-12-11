#include <time.h>
#include <string>
#include "logger/Logger.h"
#include "scag/snmp/SnmpWrapper.h"

using namespace scag2::snmp;

int main()
{
    smsc::logger::Logger::Init();

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
    return 0;
}
