#include "SmscConnector.h"
#include "ConfString.h"
#include "Task.h"
#include "Message.h"
#include "util/config/ConfigView.h"

namespace smsc {
namespace infosme2 {

smsc::sme::SmeConfig SmscConnector::readSmeConfig( smsc::util::config::ConfigView& config )
{
    smsc::sme::SmeConfig rv;
    rv.host = ConfString(config.getString("host","host was not defined")).str();
    rv.sid = ConfString(config.getString("sid","id was not defined")).str();
    rv.port = config.getInt("port","port was not defined");
    rv.timeOut = config.getInt("timeout","timeout was not defined");

    try {
        rv.password = ConfString(config.getString("password","password was not defined")).str();
    } catch ( smsc::util::config::ConfigException& e ) {}
    try {
        ConfString systemType(config.getString("systemType","system type was not defined"));
        rv.setSystemType( systemType.str() );
    } catch ( smsc::util::config::ConfigException& e ) {}
    try {
        rv.interfaceVersion = config.getInt("interfaceVersion","interface version was not defined");
    } catch (smsc::util::config::ConfigException&) {}
    try {
        ConfString ar(config.getString("rangeOfAddress","range of address was not defined"));
        rv.setAddressRange(ar.str());
    } catch (smsc::util::config::ConfigException&) {}

    return rv;
}


SmscConnector::SmscConnector( const std::string& smscId,
                              const smsc::sme::SmeConfig& cfg ) :
log_(smsc::logger::Logger::getInstance("is2.conn")),
id_(smscId)
{
    smsc_log_debug(log_,"ctor '%s'",id_.c_str());
}


SmscConnector::~SmscConnector()
{
    // FIXME: stop()
    smsc_log_debug(log_,"dtor '%s'",id_.c_str());
}


void SmscConnector::updateConfig( const smsc::sme::SmeConfig& config )
{
    smsc_log_debug(log_,"updateConfig '%s', not impl yet",id_.c_str());
}


bool SmscConnector::send( const Task& task, const Message& msg )
{
    smsc_log_debug(log_,"send '%s' for task %u/'%s', msg %llx is not impl",
                   id_.c_str(), task.getId(), task.getName().c_str(),
                   msg.id );
    // FIXME: impl
    return false;
}

}
}
