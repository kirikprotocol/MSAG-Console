#include <cassert>
#include "CommonSettings.h"
#include "informer/io/UTF8.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/ConfigWrapper.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

CommonSettings* CommonSettings::instance_ = 0;

CommonSettings::CommonSettings( unsigned licenseLimit ) :
utf8_(0),
incStatBank_(0),
licenseLimit_(licenseLimit)
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

    const ConfigWrapper conf(cfg, smsc::logger::Logger::getInstance("config"));
    path_ = conf.getString("storePath");
    if (path_.empty()) path_ = "./";
    else if (path_[path_.size()-1] != '/') path_.append("/");

    statpath_ = conf.getString("statPath");
    if (statpath_.empty()) statpath_ = "./";
    else if (statpath_[statpath_.size()-1] != '/') statpath_.append("/");

    unrespondedMessagesMax_ = conf.getInt("unrespondedMessagesMax",100,10,10000);
    responseWaitTime_ = conf.getInt("responseWaitTime",30,10,120);
    svcType_ = conf.getString("svcType","Info");
    protocolId_ = conf.getInt("protocolId",1,1,0);
    slicedMessageSize_ = conf.getInt("slicedMessageSize",160,120,250);
    inputMinQueueSize_ = conf.getInt("inputMinQueueSize",10,0,100);
    inputTransferChunkSize_ = conf.getInt("inputTransferChunkSize",1000,100,10000);
    receiptExtraWaitTime_ = conf.getInt("receiptExtraWaitTime",30,5,120);
    retryMinTimeToLive_ = conf.getInt("retryMinTimeToLive",30,10,120);
    skippedIdsMinCacheSize_ = conf.getInt("skippedIdsMinCacheSize",5,0,1000);
    skippedIdsChunkSize_ = conf.getInt("skippedIdsChunkSize",256,100,10000);
    inputJournalRollingPeriod_ = conf.getInt("inputJournalRollingPeriod",300,30,1200);
    operationalJournalRollingPeriod_ = conf.getInt("operationalJournalRollingPeriod",300,30,1200);
    inputTransferThreadCount_ = conf.getInt("inputTransferThreadCount",30,10,100);
    resendIOThreadCount_ = conf.getInt("resendIOThreadCount",10,10,30);
    validityPeriodDefault_ = conf.getInt("validityPeriodDefault",3600,1000,100000);
}

} // informer
} // smsc
