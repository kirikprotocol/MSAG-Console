#include <time.h>
#include <string>
#include <stdio.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "logger/Logger.h"

#ifdef SNMP
#include "scag/snmp/SnmpWrapper.h"
using namespace scag2::snmp;
#endif

/// adapter methods between counters and snmp
MsagCounterTableElement* counterListCtor( MsagCounterTableElement* list )
{
    printf("counterListCtor: %p\n",list);
    return 0;
    // return scag2::counter::Manager::getInstance().updateSnmpCounterList(list);
}

void counterListDtor( MsagCounterTableElement* list )
{
    printf("counterListDtor: %p\n",list);
    /*
    while (list) {
        MsagCounterTableElement* next = list->next;
        delete list;
        list = next;
    }
     */
}

int main( int argc, char** argv )
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* mainlog = smsc::logger::Logger::getInstance("main");

#ifdef SNMP
    std::string socket;
    if ( argc > 1 ) {
        socket = argv[1];
    }
    smsc_log_info(mainlog,"args parsed");

    SnmpWrapper snmp( socket );
    smsc_log_info(mainlog,"snmp wrapped created");

    int cacheTimeout = 10;
    snmp.initMsag( counterListCtor,
                   counterListDtor,
                   cacheTimeout );
    smsc_log_info(mainlog,"snmp wrapped inited");

    TrapRecord trap;
    trap.recordType = TrapRecord::Trap;
    trap.status = TrapRecord::TRAPTNEWALERT;
    trap.id = "echo1";
    trap.category = "SME";
    trap.severity = TrapRecord::MAJOR;
    trap.text = "entity disconnected";

    smsc_log_info(mainlog,"invoking check and process");
    agent_check_and_process(1);
    smsc_log_info(mainlog,"returned from check and process");

    snmp.sendTrap( trap );
    smsc_log_info(mainlog,"trap sent");

    struct timespec tm;
    tm.tv_sec = 5;
    tm.tv_nsec = 0;
    nanosleep( &tm, NULL );
#else
    printf( "NOTE: SNMP build property is not set, so this test does nothing\n" );
#endif
    smsc_log_info(mainlog,"returning");
    return 0;
}
