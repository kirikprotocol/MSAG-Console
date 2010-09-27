#include <memory>
#include "InfosmeCoreV1.h"
#include "SmscSender.h"
#include "util/config/ConfigView.h"
#include "util/config/ConfString.h"

using namespace smsc::util::config;

namespace {
std::string cgetString( const ConfigView& cv, const char* tag, const char* what )
{
    std::auto_ptr<char> str(cv.getString(tag,what));
    return std::string(str.get());
}
}

namespace eyeline {
namespace informer {

InfosmeCoreV1::InfosmeCoreV1() :
log_(0), stopping_(true), started_(false)
{
    log_ = smsc::logger::Logger::getInstance("core");
}


InfosmeCoreV1::~InfosmeCoreV1()
{
    smsc_log_info(log_,"corev1 dtor");
    tp_.shutdown(0);
    smsc_log_info(log_,"leaving corev1 dtor");
}


void InfosmeCoreV1::stop()
{
    {
        if (stopping_) return;
        MutexGuard mg(startMon_);
        if (stopping_) return;
        smsc_log_info(log_,"stop() received");
        stopping_ = true;
        startMon_.notifyAll();
    }
    tp_.stopNotify();
    MutexGuard mg(startMon_);
    while (started_) {
        startMon_.wait(100);
    }
    smsc_log_debug(log_,"leaving stop()");
}


void InfosmeCoreV1::start()
{
    if (started_) return;
    MutexGuard mg(startMon_);
    if (started_) return;
    stopping_ = false;
    Start();
}


void InfosmeCoreV1::init( const ConfigView& cfg )
{
    smsc_log_info(log_,"FIXME: initing InfosmeCore");
    std::auto_ptr< ConfigView > ccv(cfg.getSubConfig("SMSCConnectors"));
    ConfString defConn(ccv->getString("default","default SMSC id not found"));
    std::auto_ptr< CStrSet > connNames(ccv->getShortSectionNames());
    if ( connNames->find(defConn.str()) == connNames->end() ) {
        throw ConfigException("default SMSC does not match any section");
    }
    for ( CStrSet::iterator i = connNames->begin(); i != connNames->end(); ++i ) {
        smsc_log_debug(log_,"processing S='%s'",i->c_str());
        std::auto_ptr< ConfigView > sect(ccv->getSubConfig(i->c_str()));
        SmscConfig smscConfig;
        readSmscConfig(smscConfig, *sect.get());
        updateSmsc( i->c_str(), &smscConfig );
    }
}


void InfosmeCoreV1::updateSmsc( const std::string& smscId,
                                const SmscConfig*  cfg )
{
    if (cfg) {
        // create/update
        SmscSender* p = 0;
        SmscSender** ptr = 0;
        MutexGuard mg(startMon_);
        try {
            ptr = smscs_.GetPtr(smscId.c_str());
            if (!ptr) {
                p = new SmscSender(*this,smscId,*cfg);
                ptr = smscs_.SetItem(smscId.c_str(),p);
            } else if (*ptr) {
                (*ptr)->updateConfig(*cfg);
                // (*ptr)->waitUntilReleased();
            } else {
                p = new SmscSender(*this,smscId,*cfg);
                *ptr = p;
            }
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"smscsender create error: %s", e.what());
            if (p) {
                smscs_.Delete(smscId.c_str());
                delete p;
            }
        }
        if (ptr && *ptr) tp_.startTask(*ptr);
    } else {
        // delete the smsc
        MutexGuard mg(startMon_);
        SmscSender* ptr = 0;
        if (smscs_.Pop(smscId.c_str(),ptr) && ptr) {
            ptr->stop();
            ptr->waitUntilReleased();
            delete ptr;
        }
    }
}


int InfosmeCoreV1::Execute()
{
    {
        MutexGuard mg(startMon_);
        started_ = true;
        stopping_ = false;
    }
    smsc_log_info(log_,"starting main loop");
    while ( !stopping_ ) {

        smsc_log_debug(log_,"FIXME: main loop pass");
        MutexGuard mg(startMon_);
        startMon_.wait(1000);
    }
    smsc_log_info(log_,"finishing main loop");
    MutexGuard mg(startMon_);
    started_ = false;
    return 0;
}


void InfosmeCoreV1::readSmscConfig( SmscConfig& cfg, const ConfigView& config )
{
    smsc::sme::SmeConfig& rv = cfg.smeConfig;
    rv.host = ::cgetString(config,"host","SMSC host was not defined");
    rv.sid = ::cgetString(config,"sid","infosme id was not defined");
    rv.port = config.getInt("port","SMSC port was not defined");
    rv.timeOut = config.getInt("timeout","connect timeout was not defined");
    try {
        rv.password = ::cgetString(config,"password","InfoSme password wasn't defined !");
    } catch (smsc::util::config::ConfigException&) {}
    try {
        const std::string systemType = ::cgetString(config,"systemType","InfoSme system type wasn't defined !");
        rv.setSystemType(systemType);
    } catch (smsc::util::config::ConfigException&) {}
    try {
        rv.interfaceVersion = config.getInt("interfaceVersion","InfoSme interface version wasn't defined!");
    } catch (smsc::util::config::ConfigException&) {}
    try {
        const std::string ar = ::cgetString(config,"rangeOfAddress","InfoSme range of address was not defined");
        rv.setAddressRange(ar);
    } catch (smsc::util::config::ConfigException&) {}
    try {
        cfg.ussdPushOp = config.getInt("ussdPushTag");
    } catch (smsc::util::config::ConfigException) {
        cfg.ussdPushOp = -1;
    }
    try {
        cfg.ussdPushVlrOp = config.getInt("ussdPushVlrTag");
    } catch (smsc::util::config::ConfigException) {
        cfg.ussdPushVlrOp = -1;
    }
}

}
}
