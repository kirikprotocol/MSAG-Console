#include <cassert>
#include "CommonSettings.h"
#include "informer/io/UTF8.h"
#include "informer/io/InfosmeException.h"
#include "util/config/Config.h"

namespace {
using namespace eyeline::informer;
using namespace smsc::util::config;

struct ConfigWrap
{
    ConfigWrap( Config& cfg, smsc::logger::Logger* thelog ) : cfg_(cfg), log_(thelog) {}
    unsigned getInt(const char* name, unsigned def, unsigned min, unsigned max) const {
        unsigned val;
        try {
            val = cfg_.getInt(name);
        } catch ( HashInvalidKeyException& e ) {
            smsc_log_warn(log_,"the parameter '%s' is not found, using %u", name, def);
            val = def;
        }
        if (max>=min) {
            if (val<min) {
                smsc_log_warn(log_,"the parameter '%s' (%u) is less than %u, using %u",
                              name, val, min, min);
                val = min;
            } else if (val>max) {
                smsc_log_warn(log_,"the parameter '%s' (%u) is greater than %u, using %u",
                              name, val, max, max);
                val = max;
            }
        }
        return val;
    }

    std::string getString( const char* name,
                           const char* def = 0,
                           bool strip = true )
    {
        if (!name) {
            throw InfosmeException(EXC_LOGICERROR,"NULL passed in getString()");
        }
        const char* val;
        try {
            val = cfg_.getString(name);
        } catch (HashInvalidKeyException&) {
            if (!def) {
                throw InfosmeException(EXC_CONFIG,"mandatory parameter '%s' is not found",name);
            }
            smsc_log_warn(log_,"the parameter '%s' is not found, using '%s'",
                          name, def);
            val = def;
        }
        if (strip) {
            while (*val == ' ' || *val == '\t' || *val == '\n') ++val;
        }
        size_t len = strlen(val);
        if (strip) {
            for ( const char* p = val+len; len>0; ) {
                const char* v = p-1;
                if ( *v == ' ' || *v == '\t' || *v == '\n' ) {
                    --len;
                    continue;
                }
                break;
            }
        }
        return std::string(val,len);
    }
    
private:
    Config&               cfg_;
    smsc::logger::Logger* log_;
};

}


namespace eyeline {
namespace informer {

CommonSettings* CommonSettings::instance_ = 0;

CommonSettings::CommonSettings() :
utf8_(0),
incStatBank_(0)
{
    assert(instance_ == 0);
    instance_ = this;
}


CommonSettings::~CommonSettings()
{
    delete utf8_;
    instance_ = 0;
}


// void CommonSettings::init( const std::string& path, const std::string& statpath )
void CommonSettings::init( smsc::util::config::Config& cfg )
{
    utf8_ = new UTF8();

    ::ConfigWrap conf(cfg, smsc::logger::Logger::getInstance("config"));
    path_ = conf.getString("storePath");
    if (path_.empty()) path_ = "./";
    else if (path_[path_.size()-1] != '/') path_.push_back('/');

    statpath_ = conf.getString("statPath");
    if (statpath_.empty()) statpath_ = "./";
    else if (statpath_[statpath_.size()-1] != '/') statpath_.push_back('/');

    unrespondedMessagesMax_ = conf.getInt("unrespondedMessagesMax",100,10,10000);
    responseWaitTime_ = conf.getInt("responseWaitTime",30,10,120);
    svcType_ = conf.getString("svcType","Info");
    protocolId_ = conf.getInt("protocolId",1,1,0);
    slicedMessageSize_ = conf.getInt("slicedMessageSize",160,120,250);
    inputMinQueueSize_ = conf.getInt("inputMinQueueSize",10,0,100);
    inputTransferChunkSize_ = conf.getInt("inputTransferChunkSize",1000,100,10000);
    receiptExtraWaitTime_ = conf.getInt("receiptExtraWaitTime",30,5,120);
    retryMinTimeToLive_ = conf.getInt("retryMinTimeToLive",30,10,120);
    skippedIdsMinCacheSize_ = conf.getInt("skippedIdsMinCacheSize",10,0,1000);
    skippedIdsChunkSize_ = conf.getInt("skippedIdsChunkSize",256,100,10000);
    inputJournalRollingPeriod_ = conf.getInt("inputJournalRollingPeriod",300,30,1200);
    operationalJournalRollingPeriod_ = conf.getInt("operationalJournalRollingPeriod",300,30,1200);
    inputTransferThreadCount_ = conf.getInt("inputTransferThreadCount",30,10,100);
    resendIOThreadCount_ = conf.getInt("resendIOThreadCount",10,10,30);
}

} // informer
} // smsc
