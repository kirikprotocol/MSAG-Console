// utility to load config from xml files

#include <vector>
#include <string>
#include "util/config/ConfigView.h"
#include "util/config/Config.h"
#include "util/config/ConfString.h"
#include "scag/pvss/api/core/client/ClientConfig.h"
#include "FlooderConfig.h"
#include "scag/pvss/common/ScopeType.h"

namespace {

void readClientConfig( smsc::logger::Logger*                    logger,
                       scag2::pvss::core::client::ClientConfig& clientConfig,
                       smsc::util::config::Manager&             manager )
{
    smsc::util::config::ConfigView cview(manager,"PvssClient");
    try {
        clientConfig.setEnabled( cview.getBool("enabled") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.enabled> missed. Default value is %d", clientConfig.isEnabled());
    }

    try {
        clientConfig.setHost( smsc::util::config::ConfString(cview.getString("host")).str() );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.host> missed. Defaul value is %s", clientConfig.getHost().c_str());
    }
    try { 
        clientConfig.setPort( cview.getInt("port"));
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.port> missed. Defaul value is %d", int(clientConfig.getPort())&0xffff );
    }
    try { 
        clientConfig.setIOTimeout( cview.getInt("ioTimeout") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.ioTimeout> missed. Defaul value is %d", clientConfig.getIOTimeout());
    }
    try { 
        clientConfig.setInactivityTime( cview.getInt("pingTimeout")*1000 );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.pingTimeout> missed. Defaul value is %d / 1000", clientConfig.getInactivityTime() );
    }
    try { 
        clientConfig.setConnectTimeout( cview.getInt("reconnectTimeout")*1000 );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.reconnectTimeout> missed. Defaul value is %d / 1000", clientConfig.getConnectTimeout() );
    }
    try { 
        clientConfig.setChannelQueueSizeLimit( cview.getInt("maxWaitingRequestsCount") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.maxWaitingRequestsCount> missed. Defaul value is %d", clientConfig.getChannelQueueSizeLimit() );
    }
    try { 
        clientConfig.setConnectionsCount( cview.getInt("connections") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.connections> missed. Defaul value is %d", clientConfig.getConnectionsCount());
    }
    try { 
        unsigned connPerThread = cview.getInt("connPerThread");
        clientConfig.setMaxReaderChannelsCount( connPerThread );
        clientConfig.setMaxWriterChannelsCount( connPerThread );
        clientConfig.setReadersCount( (clientConfig.getConnectionsCount()-1) / connPerThread + 1 );
        clientConfig.setWritersCount( (clientConfig.getConnectionsCount()-1) / connPerThread + 1 );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.connPerThread> missed. Defaul value is %d/%d", clientConfig.getMaxReaderChannelsCount(), clientConfig.getMaxWriterChannelsCount());
    }
}


void readFlooderConfig( smsc::logger::Logger* logger,
                        scag2::pvss::flooder::FlooderConfig& flooderConfig,
                        smsc::util::config::Manager& manager )
{
    smsc::util::config::ConfigView fview(manager,"Flooder");
    try {
        flooderConfig.setAsyncMode( fview.getBool("async") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.async> missed. Defaul value is %d", flooderConfig.getAsyncMode() );
    }
    try { 
        flooderConfig.setSpeed( fview.getInt("speed") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.speed> missed. Defaul value is %d", flooderConfig.getSpeed());
    }
    try { 
        flooderConfig.setAddressesCount( fview.getInt("addressesCount") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.addressesCount> missed. Defaul value is %d", flooderConfig.getAddressesCount());
    }
    try { 
        flooderConfig.setAddressOffset( fview.getInt("addressOffset") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.addressOffset> missed. Defaul value is %d", flooderConfig.getAddressOffset());
    }
    try { 
        const unsigned propertiesCount = fview.getInt("properties");
        std::vector< std::string > properties;
        for ( unsigned i = 0; i < propertiesCount; ++i ) {
            char pbuf[50];
            snprintf(pbuf,sizeof(pbuf),"property.%d",i);
            properties.push_back( smsc::util::config::ConfString(fview.getString(pbuf)).str() );
        }
        flooderConfig.setPropertyPatterns(properties);
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.properties> missed or broken. Default number of properties is %d", flooderConfig.getPropertyPatterns().size());
    }

    if ( flooderConfig.getPropertyPatterns().size() <= 0 ) {
        smsc_log_error(logger, "cannot proceed: no property patterns specified");
        abort();
    }

    try {
        flooderConfig.setCommands( smsc::util::config::ConfString(fview.getString("commands")).str() );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.commands> missed. Default value is %s", flooderConfig.getCommands().c_str());
    }

    try {
        flooderConfig.setOneCommandPerAbonent( fview.getBool("oneCommandPerAbonent") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.oneCommandPerAbonent> missed. Default value is %d",
                      flooderConfig.getOneCommandPerAbonent() ? 1 : 0 );
    }

    try {
        flooderConfig.setMaxSpeed( fview.getBool("maxSpeed") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.maxSpeed> missed. Default value is %d",
                      flooderConfig.getMaxSpeed() ? 1 : 0 );
    }

    try {
        std::string format = fview.getString("addressPrefix");
        // checking that address prefix is numeric
        char* endptr;
        unsigned pfx = unsigned(strtoul(format.c_str(),&endptr,10));
        if ( *endptr != '\0' ) throw std::runtime_error("wrong address prefix");
        char buf[40];
        snprintf(buf,sizeof(buf),"%u",pfx);
        if ( strlen(buf) > 10 ) throw std::runtime_error("too long prefix");
        format = buf;
        snprintf(buf,sizeof(buf),"%s%%0%uu",format.c_str(),unsigned(11-format.size()));
        flooderConfig.setAddressFormat(buf);
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.addressPrefix> missed or wrong. Default value is %s", flooderConfig.getAddressFormat().c_str());
    }

    try {
        std::string stype = fview.getString("scopeType");
        scag2::pvss::ScopeType st = scag2::pvss::scopeTypeFromString(stype.c_str());
        if ( st == scag2::pvss::ScopeType(0) ) {
            throw std::runtime_error("wrong scopeType");
        } else {
            flooderConfig.setScopeType(st);
        }
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.scopeType> missed or wrong. Default value is %s", scopeTypeToString(flooderConfig.getScopeType()) );
    }
}

}
