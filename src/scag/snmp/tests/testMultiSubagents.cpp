#include <cstdio>
#include <cstdlib>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

const unsigned waitTime = 1000;

extern "C" {

    static int status = 0;
    struct timeval agentStartTime;
    static oid status_oid[] = {1,3,6,1,4,1,26757,9999,1,-1};
    static oid uptime_oid[] = {1,3,6,1,4,1,26757,9999,2,-1};

    int testMultiStatusHandler(netsnmp_mib_handler *handler,
                               netsnmp_handler_registration *reginfo,
                               netsnmp_agent_request_info *reqinfo,
                               netsnmp_request_info *requests);
    int testMultiUptimeHandler(netsnmp_mib_handler *handler,
                               netsnmp_handler_registration *reginfo,
                               netsnmp_agent_request_info *reqinfo,
                               netsnmp_request_info *requests);

} // extern C


void updateOid(oid *oidVal, int oidLen, int instance)
{
    printf("New OID: ");
    for ( int i = 0; i < oidLen; ++i ) {
        if ( oidVal[i] == oid(-1) ) oidVal[i] = instance;
        printf(".%lu",oidVal[i]);
    }
    printf("\n");
}


void init_instance( int instance )
{
    netsnmp_handler_registration *reginfo;

    updateOid(status_oid,OID_LENGTH(status_oid),instance);
    reginfo = netsnmp_create_handler_registration("status",
                                                  testMultiStatusHandler,
                                                  status_oid,
                                                  OID_LENGTH(status_oid),
                                                  HANDLER_CAN_RWRITE);
    netsnmp_register_instance(reginfo);

    updateOid(uptime_oid,OID_LENGTH(uptime_oid),instance);
    reginfo = netsnmp_create_handler_registration("uptime",
                                                  testMultiUptimeHandler,
                                                  uptime_oid,
                                                  OID_LENGTH(uptime_oid),
                                                  HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
}


/// a test of several subagents running at the same IP
int main( int argc, char** argv )
{
    char* agentx_socket = 0;
    int optind = 1;

    while ( optind < argc )
    {
        if ( argv[optind][0] != '-' ) break;
        int ch = argv[optind][1];
        switch (ch) {
        case 'D':
            if ( ++optind >= argc ) {
                fprintf(stderr,"option -D requires an argument\n");
                ::exit(-1);
            }
            debug_register_tokens(argv[optind]);
            snmp_set_do_debugging(1);
            break;
            /*
        case 'f':
            dont_fork = 1;
            break;
             */
        case 'H':
            netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
                                   NETSNMP_DS_AGENT_NO_ROOT_ACCESS, 1);
            init_agent("testMulti");        /* register our .conf handlers */
            init_instance(1);
            init_snmp("testMulti");
            fprintf(stderr, "Configuration directives understood:\n");
            read_config_print_usage("  ");
            exit(0);
            /*
        case 'M':
            agentx_subagent = 0;
            break;
        case 'L':
            use_syslog = 0; // use stderr
            break;
             */
        case 'x':
            if ( ++optind >= argc ) {
                fprintf(stderr,"option -x requires an argument\n");
                std::exit(-1);
            }
            agentx_socket = argv[optind];
            break;
        default:
            fprintf(stderr,"unknown option %c\n", ch);
            std::exit(-1);
        }
        ++optind;
    }

    if ( optind >= argc ) {
        fprintf(stderr,"Please specify an instance number (integer 0..N)\n");
        std::exit(-1);
    }
    int instance = atoi(argv[optind]);

    printf("initializing\n");

    if (NULL != agentx_socket)
        netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID,
                              NETSNMP_DS_AGENT_X_SOCKET, agentx_socket);

    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID,
                           NETSNMP_DS_AGENT_ROLE, 1);  // we are a subagent

    // handler for uptime
    status = instance;
    gettimeofday(&agentStartTime,NULL);
    agentStartTime.tv_sec -= 1;
    agentStartTime.tv_usec += 1000000;

    init_agent("testMulti");  // initialize the agent library
    ::init_instance( instance );   // initialize mib code
    init_snmp("testMulti");   // read .conf files

    printf("now sleeping for %u seconds\n",waitTime);

    const time_t started = time(0);
    while ( time(0) < time_t(started + waitTime) ) {
        agent_check_and_process(1);
    }
    snmp_shutdown("testMulti");
    return 0;
}


    extern "C"
    int testMultiStatusHandler(netsnmp_mib_handler *handler,
                               netsnmp_handler_registration *reginfo,
                               netsnmp_agent_request_info *reqinfo,
                               netsnmp_request_info *requests)
{
    int *it = &status;
    int *it_save;

    switch (reqinfo->mode) {
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) it, sizeof(*it));
        break;
    case MODE_SET_RESERVE1:
        printf("hello MODE_SET_RESERVE1\n");
        if (requests->requestvb->type != ASN_INTEGER)
        {
            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
        }
        break;
    case MODE_SET_RESERVE2:
        printf("hello MODE_SET_RESERVE2\n");
        memdup((u_char **) & it_save, (u_char *) it, sizeof(u_long));
        if (it_save == NULL)
        {
            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
            return SNMP_ERR_NOERROR;
        }
        netsnmp_request_add_list_data(requests, netsnmp_create_data_list(INSTANCE_HANDLER_NAME, it_save, free));
        break;
    case MODE_SET_ACTION:
        printf("hello MODE_SET_ACTION\n");
        status = *(requests->requestvb->val.integer);
        break;
    case MODE_SET_UNDO:
    case MODE_SET_COMMIT:
    case MODE_SET_FREE: break;
    }
    if (handler->next && handler->next->access_method)
    {
        return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }
    return SNMP_ERR_NOERROR;
}


int testMultiUptimeHandler(netsnmp_mib_handler *handler,
                           netsnmp_handler_registration *reginfo,
                           netsnmp_agent_request_info *reqinfo,
                           netsnmp_request_info *requests)
{
    struct timeval diff, now;
    u_long agentUpTime;

    switch (reqinfo->mode) {
    case MODE_GET:
        gettimeofday(&now, NULL);
        diff.tv_sec = now.tv_sec - agentStartTime.tv_sec - 1;
        diff.tv_usec = now.tv_usec - agentStartTime.tv_usec + 1000000;
        agentUpTime = ((u_long) diff.tv_sec) * 100 + diff.tv_usec / 10000;
        snmp_set_var_typed_value(requests->requestvb, ASN_TIMETICKS, (u_char *)&agentUpTime, sizeof(agentUpTime));
        break;
    case MODE_SET_RESERVE1:
    case MODE_SET_RESERVE2:
    case MODE_SET_ACTION:
    case MODE_SET_UNDO:
    case MODE_SET_COMMIT:
    case MODE_SET_FREE:
        break;
    }
    if (handler->next && handler->next->access_method)
    {
        return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }
    return SNMP_ERR_NOERROR;
}
