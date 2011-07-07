#ifdef SNMP

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "SnmpManagerImpl.h"
#include "informer/io/InfosmeException.h"

namespace eyeline {
namespace informer {

namespace {
const char* informerName = "informer";
const char* informerNamed = "informerd";
}


SnmpManagerImpl::SnmpManagerImpl() :
log_(smsc::logger::Logger::getInstance("snmp")),
stopping_(false)
{
    smsc_log_debug(log_,"ctor");
}


SnmpManagerImpl::~SnmpManagerImpl()
{
    stop();
    snmp_shutdown( informerName );
    snmp_shutdown( informerNamed );
}


void SnmpManagerImpl::init( const std::string& snmpSock )
{
    smsc_log_info(log_,"snmp initialization, socket='%s'",snmpSock.c_str() );
    // snmp_disable_stderrlog();
    netsnmp_ds_set_boolean( NETSNMP_DS_APPLICATION_ID,
                            NETSNMP_DS_AGENT_ROLE, 1 ); // we are a subagent
    if ( !snmpSock.empty() ) {
        netsnmp_ds_set_string( NETSNMP_DS_APPLICATION_ID,
                               NETSNMP_DS_AGENT_X_SOCKET, snmpSock.c_str() ); // setting a connection
    }
    init_agent( informerNamed ); // initialize agent library
    init_snmp( informerName );   // read the conf file
}


void SnmpManagerImpl::start()
{
    Start();
}


void SnmpManagerImpl::sendTrap( SnmpTrap::Type     type,
                                SnmpTrap::Severity severity,
                                const char*        category,
                                const char*        objid,
                                const char*        message )
{
    // we cannot accept this trap
    if (stopping_) return;

    if (!category || !category[0] ||
        !objid || !objid[0] ||
        !message || !message[0] ) {
        throw InfosmeException( EXC_LOGICERROR, "snmp trap should have valid category/objid/message");
    }

    std::auto_ptr< SnmpTrap > ptr(new SnmpTrap);
    ptr->type = type;
    ptr->severity = severity;
    ptr->category = category;
    ptr->objid = objid;
    ptr->message = message;
    if ( !ptr->isValid() ) {
        throw InfosmeException( EXC_LOGICERROR, "snmp trap should have valid type/severity");
    }

    queue_.Push( ptr.release() );
}


void SnmpManagerImpl::stop()
{
    stopping_ = true;
    smsc_log_debug(log_,"stopping snmp thread...");
    queue_.notify();
    WaitFor();
}


int SnmpManagerImpl::Execute()
{
    oid informerConnectAlert_oid[] = { 1,3,6,1,4,1,26757,4,0,1 };
    oid informerFileIOAlert_oid[] = { 1,3,6,1,4,1,26757,4,0,2 };
    oid alertMessage_oid[] = { 1,3,6,1,4,1,26757,4,5, 0 };
    oid alertSeverity_oid[] = { 1,3,6,1,4,1,26757,4,7, 0 };
    oid alertObjCategory_oid[] = { 1,3,6,1,4,1,26757,4,9, 0 };
    oid alertId_oid[] = { 1,3,6,1,4,1,26757,4,8, 0 };

    smsc_log_info(log_,"snmp thread started");
    while ( true ) {

        const bool stop = stopping_;
        if (!stop) {
            queue_.waitForItem();
        }

        SnmpTrap* trap;
        while ( queue_.Pop(trap) ) {
            std::auto_ptr<SnmpTrap> tg(trap);

            oid* typeoid = 0;
            size_t typeoidlen = 0;
            switch (trap->type) {
#define TRAPTCASE(x,y) case (x) : { typeoid = y; typeoidlen = OID_LENGTH(y); break; }
                TRAPTCASE(SnmpTrap::TYPE_CONNECT,informerConnectAlert_oid);
                TRAPTCASE(SnmpTrap::TYPE_FILEIO,informerFileIOAlert_oid);
#undef TRAPTCASE
            default:
                smsc_log_warn(log_,"trap type %d is not implemented",int(trap->type));
                break; // cant be here
            }
            if (!typeoid) continue;

            // --- fill other fields
            netsnmp_variable_list  *var_list = NULL;

            uint32_t nsev = htonl(uint32_t(trap->severity));
            u_char* buf; size_t buflen;
            buf = (u_char*)&nsev;
            buflen = sizeof(nsev);
            snmp_varlist_add_variable(&var_list,
                                      alertSeverity_oid,
                                      OID_LENGTH(alertSeverity_oid),
                                      ASN_INTEGER,
                                      buf, buflen );

            buf = (u_char*)trap->category.c_str();
            buflen = trap->category.size();
            snmp_varlist_add_variable(&var_list,
                                      alertObjCategory_oid,
                                      OID_LENGTH(alertObjCategory_oid),
                                      ASN_OCTET_STR,
                                      buf, buflen );

            buf = (u_char*)trap->objid.c_str();
            buflen = trap->objid.size();
            snmp_varlist_add_variable(&var_list,
                                      alertId_oid,
                                      OID_LENGTH(alertId_oid),
                                      ASN_OCTET_STR,
                                      buf, buflen );

            buf = (u_char*)trap->message.c_str();
            buflen = trap->message.size();
            snmp_varlist_add_variable(&var_list,
                                      alertMessage_oid,
                                      OID_LENGTH(alertMessage_oid),
                                      ASN_OCTET_STR,
                                      buf, buflen );

            char logbuf[100];
            smsc_log_debug(log_,"sending trap %s",trap->toString(logbuf,sizeof(logbuf)));

            // --- send
            send_enterprise_trap_vars( SNMP_TRAP_ENTERPRISESPECIFIC, 1,
                                       typeoid, (int)typeoidlen, var_list );
            snmp_free_varbind(var_list);

        } // while queue_.Pop()

        if (stop) break;
    }
    smsc_log_info(log_,"snmp thread finished");
    return 0;
}

}
}

#endif
