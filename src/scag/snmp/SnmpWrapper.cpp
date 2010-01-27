#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <string.h>
#include "SnmpWrapper.h"

namespace {

const char* msagnamed = "msagd";
const char* msagname = "msag";

// oid snmptrap_oid[] = {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};
oid alertMessage_oid[] = { 1,3,6,1,4,1,26757,2,5, 0 };
oid alertSeverity_oid[] = { 1,3,6,1,4,1,26757,2,7, 0 };
oid alertObjCategory_oid[] = { 1,3,6,1,4,1,26757,2,9, 0 };
oid alertId_oid[] = { 1,3,6,1,4,1,26757,2,8, 0 };

// types of traps
oid msagNewAlertFFMR_oid[] = { 1,3,6,1,4,1,26757,2,0,4 };
oid msagClearAlertFFMR_oid[] = { 1,3,6,1,4,1,26757,2,0,5 };
oid msagLoadConfig_oid[] = { 1,3,6,1,4,1,26757,2,0,6 };
oid msagTraffic_oid[] = { 1,3,6,1,4,1,26757,2,0,7 };
oid msagSMPPEndPointTraffic_oid[] = { 1,3,6,1,4,1,26757,2,0,8 };
oid msagSMPPQueueLimit_oid[] = { 1,3,6,1,4,1,26757,2,0,9 };
oid msagSessionLimit_oid[] = { 1,3,6,1,4,1,26757,2,0,10 };
oid msagSMPPConnect_oid[] = { 1,3,6,1,4,1,26757,2,0,11 };

void init_msag()
{
    // do nothing right now
}

}

namespace scag2 {
namespace snmp {

SnmpWrapper::SnmpWrapper( const std::string& socket ) :
log_(0)
{
    log_ = smsc::logger::Logger::getInstance( "snmp" );
    // FIXME: uncomment
    // snmp_disable_stderrlog();
    netsnmp_ds_set_boolean( NETSNMP_DS_APPLICATION_ID,
                            NETSNMP_DS_AGENT_ROLE, 1 ); // we are a subagent

    // trimming the socket
    std::string theSocket(socket);
    {
        const std::string whitespaces(" \t\v\r\n");
        size_t b = theSocket.find_first_not_of(whitespaces);
        if ( b != std::string::npos ) {
            theSocket.erase(theSocket.begin(),theSocket.begin()+b);
        }
        b = theSocket.find_last_not_of(whitespaces);
        if ( b != std::string::npos ) {
            ++b;
            theSocket.erase(theSocket.begin()+b,theSocket.end());
        }
    }
    if ( ! theSocket.empty() ) {
        netsnmp_ds_set_string( NETSNMP_DS_APPLICATION_ID,
                               NETSNMP_DS_AGENT_X_SOCKET, theSocket.c_str() ); // setting a connection
    }

    init_agent( ::msagnamed );  // initialize the agent library
    ::init_msag();             // initialize mib code
    init_snmp( ::msagname );   // read .conf files
    smsc_log_info(log_,"snmp wrapper inited");
}


SnmpWrapper::~SnmpWrapper()
{
    snmp_shutdown( ::msagname );
    snmp_shutdown( ::msagnamed );
    smsc_log_info(log_,"snmp wrapper shutdowned");
}


void SnmpWrapper::sendTrap( const TrapRecord& rec )
{
    switch ( rec.recordType ) {

    case ( TrapRecord::Trap ) : {

        oid* poid = 0;
        size_t poidlen = 0;
        switch (rec.status) {

#define TRAPTCASE(x,y) case (x) : { poid = y; poidlen = OID_LENGTH(y); break; }
            TRAPTCASE(TrapRecord::TRAPTNEWALERT,msagNewAlertFFMR_oid);
            TRAPTCASE(TrapRecord::TRAPTCLRALERT,msagClearAlertFFMR_oid);
            TRAPTCASE(TrapRecord::TRAPTLOADCFG,msagLoadConfig_oid);
            TRAPTCASE(TrapRecord::TRAPTTRAFFIC,msagTraffic_oid);
            TRAPTCASE(TrapRecord::TRAPTSMPPTRAF,msagSMPPEndPointTraffic_oid);
            TRAPTCASE(TrapRecord::TRAPTSMPPQLIM,msagSMPPQueueLimit_oid);
            TRAPTCASE(TrapRecord::TRAPTSESSLIM,msagSessionLimit_oid);
            TRAPTCASE(TrapRecord::TRAPTSMPPCONN,msagSMPPConnect_oid);
            /*
        case (TrapRecord::TRAPTNEWALERT) : {
            poid = msagNewAlertFFMR_oid;
            poidlen = OID_LENGTH(msagNewAlertFFMR_oid);
            break;
        }
        case (TrapRecord::TRAPTCLRALERT) : {
            poid = msagClearAlertFFMR_oid;
            poidlen = OID_LENGTH(msagClearAlertFFMR_oid);
            break;
        }
             */
#undef TRAPTCASE
        default : {
            smsc_log_warn( log_, "trap(id=%s, object=%s, severity=%d, text=%s): unknown status=%d",
                           rec.id.c_str(),
                           rec.category.c_str(),
                           rec.severity,
                           rec.text.c_str(),
                           rec.status );
            break;
        }
        } // switch rec.status
        if ( ! poid ) break;

        smsc_log_debug( log_, "trap(id=%s, cat=%s, severity=%d, text=%s)",
                        rec.id.c_str(),
                        rec.category.c_str(),
                        rec.severity,
                        rec.text.c_str() );

        netsnmp_variable_list  *var_list = NULL;
        u_char* buf; size_t buflen;
        if ( rec.text.length() ) {
            buf = (u_char*)rec.text.c_str(); buflen = rec.text.length();
            snmp_varlist_add_variable(&var_list,
                                      alertMessage_oid, 
                                      OID_LENGTH(alertMessage_oid),
                                      ASN_OCTET_STR,
                                      buf,
                                      buflen );
        }
        {
            uint32_t n = htonl( uint32_t(rec.severity) );
            buf = (u_char*)&n; buflen = sizeof(n);
            snmp_varlist_add_variable(&var_list,
                                      alertSeverity_oid,
                                      OID_LENGTH(alertSeverity_oid),
                                      ASN_INTEGER,
                                      buf,
                                      buflen );
        }
        if ( rec.category.length() ) {
            buf = (u_char*)rec.category.c_str(); buflen = rec.category.length();
            snmp_varlist_add_variable(&var_list,
                                      alertObjCategory_oid,
                                      OID_LENGTH(alertObjCategory_oid),
                                      ASN_OCTET_STR,
                                      buf,
                                      buflen);
        }
        if ( rec.id.length() ) {
            buf = (u_char*)rec.id.c_str(); buflen = rec.id.length();
            snmp_varlist_add_variable(&var_list,
                                      alertId_oid,
                                      OID_LENGTH(alertId_oid),
                                      ASN_OCTET_STR,
                                      buf,
                                      buflen );
        }

        send_enterprise_trap_vars( SNMP_TRAP_ENTERPRISESPECIFIC, 1,
                                   poid, (int)poidlen, var_list );
        snmp_free_varbind(var_list);
        break;
    }

    case ( TrapRecord::Notification ) : {
        smsc_log_warn( log_, "notification trap is not impl yet" );
        break;
    }

    case ( TrapRecord::StatusChange ) : {
        smsc_log_warn( log_, "status change trap is not impl yet" );
        break;
    }
    default :
        smsc_log_warn( log_, "unknown trap record type" );

    } // switch on record type

}

}
}
