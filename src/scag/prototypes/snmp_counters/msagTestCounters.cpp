#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <signal.h>
#include <getopt.h>

#include "logger/Logger.h"
#include "msagCounterTable_cxxiface.h"
#include "msagCounterTable.h"
#include "core/buffers/Hash.hpp"

static int keep_running;

extern "C" {
static void stop_server( int a ) {
    keep_running = 0;
}
}


struct Entry
{
    const char* name;
    uint64_t    value;
};

msagCounterTable_list_element* list_creator( msagCounterTable_list_element* old )
{
    static const Entry entries[] = {
        { "sessions.total", 100 },
        { "sessions.active", 50 },
        { "sessions.locked", 2 },
        { 0, 0 }
    };

    static smsc::logger::Logger* llog = smsc::logger::Logger::getInstance("lcreat");
    smsc_log_debug(llog,"req to create %p",old);
    static unsigned npass = 1;

    smsc::core::buffers::Hash< const Entry* > tmpHash;
    for ( unsigned i = 0; i < npass; ++i ) {
        if ( ! entries[i].name ) break;
        tmpHash.Insert( entries[i].name, &entries[i] );
    }
    
    msagCounterTable_list_element* tail = old;
    while ( tail ) {
        tmpHash.Delete(tail->data.name);
        tail->data.value += 3;
        smsc_log_debug(llog,"setting new value for %s: %lld",tail->data.name,tail->data.value);
        if (!tail->next) break;
        tail = tail->next;
    }
    char* key;
    const Entry* val;
    for ( smsc::core::buffers::Hash<const Entry*>::Iterator i(&tmpHash);
          i.Next(key,val); ) {
        msagCounterTable_list_element* elt = new msagCounterTable_list_element;
        memset(elt,0,sizeof(msagCounterTable_list_element));
        elt->data.namelen = int(strlen(val->name));
        elt->data.value = val->value;
        memcpy(elt->data.name,val->name,elt->data.namelen+1);
        smsc_log_debug(llog,"adding new element %s: %lld",elt->data.name, elt->data.value);
        if ( tail ) {
            tail->next = elt;
            tail = elt;
        } else {
            old = tail = elt;
        }
    }
    ++npass;
    return old;
}


int main( int argc, char** argv )
{
    const char* agentx_socket = 0;
    int ch;
    while ( (ch = getopt(argc,argv,"D:Hx:")) != EOF ) {
        switch (ch) {
        case 'D':
            debug_register_tokens(optarg);
            snmp_set_do_debugging(1);
            break;
        case 'H':
            netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
                                   NETSNMP_DS_AGENT_NO_ROOT_ACCESS, 1);
            init_agent("msagCounterTable");        /* register our .conf handlers */
            init_msagCounterTable();  
            init_snmp("msagCounterTable");
            fprintf(stderr, "Configuration directives understood:\n");
            read_config_print_usage("  ");
            exit(0);
        case 'x':
            agentx_socket = optarg;
            break;
        }
    }

    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );

    smsc::logger::Logger* mainlog = smsc::logger::Logger::getInstance("main");
    
    snmp_enable_stderrlog();

    smsc_log_debug(mainlog,"snmp port specs: %s\n",
                   netsnmp_ds_get_string(NETSNMP_DS_APPLICATION_ID, 
                                         NETSNMP_DS_AGENT_PORTS));

    netsnmp_ds_set_boolean( NETSNMP_DS_APPLICATION_ID,
                            NETSNMP_DS_AGENT_ROLE, 1 ); // we are a subagent

    // not needed?
    // netsnmp_enable_subagent();


    if ( agentx_socket ) {
        smsc_log_debug(mainlog,"setting agentx socket: %s", agentx_socket);
        netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID,
                              NETSNMP_DS_AGENT_X_SOCKET, agentx_socket);
    }

    smsc_log_debug(mainlog,"registering a counter table creator");
    msagCounterTable_register_list_creator( list_creator );

    init_agent("msagCounterTable");
    init_msagCounterTable();
    smsc_log_debug(mainlog,"================\n\n\n  msagCounterTable is registered\n\n\n");
    init_snmp("msagCounterTable");
    smsc_log_debug(mainlog,"================\n\n\n  init done\n\n\n");

    keep_running = 1;
    signal(SIGTERM,stop_server);
    signal(SIGINT,stop_server);
    while ( keep_running ) {
        agent_check_and_process(1);
        smsc_log_debug(mainlog,"pass at %lld",int64_t(time(0)));
    }
    snmp_shutdown("msagCounterTable");
    smsc_log_debug(mainlog,"done");
    return 0;
}
