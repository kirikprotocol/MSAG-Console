#include <cassert>
#include "CommonSettings.h"
#include "informer/io/UTF8.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/ConfigWrapper.h"
#include "util/config/Config.h"
#include "TimezoneGroup.h"
#include "logger/Logger.h"

using smsc::util::config::Config;
using smsc::util::config::CStrSet;

namespace eyeline {
namespace informer {

CommonSettings* CommonSettings::instance_ = 0;

CommonSettings::CommonSettings( unsigned licenseLimit ) :
log_(smsc::logger::Logger::getInstance("comset")),
utf8_(0),
snmp_(0),
incStatBank_(0),
licenseLimit_(licenseLimit),
// regionSpeedLimitNBins_(25),
// regionSpeedLimitInterval_(5),
regionSpeedLimitSpeedup_(10),
// regionSpeedLimitBaseSpeed_(1),
stopping_(false),
archive_(false),
emergency_(false)
{
    assert(instance_ == 0);
    instance_ = this;
}


CommonSettings::~CommonSettings()
{
    for ( std::vector<TimezoneGroup*>::iterator i = tzgroups_.begin();
          i != tzgroups_.end(); ++i ) {
        delete *i;
    }
    tzgroups_.clear();
    delete utf8_;
    instance_ = 0;
}


// void CommonSettings::init( const std::string& path, const std::string& statpath )
void CommonSettings::init( smsc::util::config::Config& cfg,
                           SnmpManager*                snmp,
                           bool                        archive )
{
    snmp_ = snmp;
    archive_ = archive;
    utf8_ = new UTF8();

    const ConfigWrapper conf(cfg,log_);
    emergency_ = conf.getBool("emergency",false);
    if ( emergency_ && archive_ ) {
        throw InfosmeException( EXC_CONFIG, "archive daemon can not have 'emergency' flag");
    }

    path_ = conf.getString("storePath");
    if (path_.empty()) path_ = "./";
    else if (path_[path_.size()-1] != '/') path_.append("/");

    archivePath_ = conf.getString("archivePath");
    if (archivePath_.empty()) archivePath_ = "./";
    else if (archivePath_[archivePath_.size()-1] != '/') {
        archivePath_.append("/");
    }

    if ( archive_ ) {
        path_.swap(archivePath_);
    }

    statpath_ = conf.getString("statPath");
    if (statpath_.empty()) statpath_ = "./";
    else if (statpath_[statpath_.size()-1] != '/') statpath_.append("/");

    unrespondedMessagesMax_ = conf.getInt("unrespondedMessagesMax",100,10,10000);
    responseWaitTime_ = conf.getInt("responseWaitTime",30,10,120);
    svcType_ = conf.getString("svcType","Info");
    protocolId_ = conf.getInt("protocolId",1,1,0);
    slicedMessageSize_ = conf.getInt("slicedMessageSize",160,120,250);
    inputMinQueueTime_ = conf.getInt("inputMinQueueSize",5,0,100);
    inputTransferChunkTime_ = conf.getInt("inputTransferChunkSize",60,10,300);
    inputRequestAverageTime_ = conf.getInt("inputRequestAverageTime",1,1,30);
    receiptExtraWaitTime_ = conf.getInt("receiptExtraWaitTime",30,5,120);
    retryMinTimeToLive_ = conf.getInt("retryMinTimeToLive",30,10,120);
    skippedIdsMinCacheSize_ = conf.getInt("skippedIdsMinCacheSize",5,0,1000);
    skippedIdsChunkSize_ = conf.getInt("skippedIdsChunkSize",256,100,10000);
    inputJournalRollingPeriod_ = conf.getInt("inputJournalRollingPeriod",300,30,1200);
    operationalJournalRollingPeriod_ = conf.getInt("operationalJournalRollingPeriod",300,30,1200);
    smscJournalRollingPeriod_ = conf.getInt("smscJournalRollingPeriod",10,10,1200);
    inputTransferThreadCount_ = conf.getInt("inputTransferThreadCount",30,10,100);
    resendIOThreadCount_ = conf.getInt("resendIOThreadCount",10,10,30);
    regionRandomizationJitter_ = conf.getInt("regionRandomizationJitter",12451,100,2000000);
    
    validityPeriodDefault_ = conf.getInt("validityPeriodDefault",3600,300,100000);
    messageTimeToLiveDefault_ = conf.getInt("messageTimeToLiveDefault",3600,300,100000);

    inputJournalRollingSpeed_ = conf.getInt("inputJournalRollingSpeed",30,1,100000);
    operationalJournalRollingSpeed_ = conf.getInt("operationalJournalRollingSpeed",30,1,100000);
    smscJournalRollingSpeed_ = conf.getInt("smscJournalRollingSpeed",30,1,100000);

    // regionSpeedLimitNBins_ = conf.getInt("regionSpeedLimitNBins",25,10,100);
    // regionSpeedLimitInterval_ = conf.getInt("regionSpeedLimitInterval",5,1,10);
    regionSpeedLimitSpeedup_ = conf.getInt("regionSpeedLimitSpeedup",10,5,200);
    // regionSpeedLimitBaseSpeed_ = conf.getInt("regionSpeedLimitBaseSpeed",1,1,200);

    recalcTTLRequested_ = conf.getBool("recalcTTLRequested",true);

#ifdef CHECKCONTENTION
    const unsigned cl = conf.getInt("contentionLimit",300000,10000,10000000);
    smsc::core::synchronization::Mutex::setContentionLimit(cl);
#endif

    loadTimezones();
}


void CommonSettings::loadTimezones()
{
    tzgroups_.push_back( new TimezoneGroup(TimezoneGroup::TZ_UNKNOWN) );
    TimezoneGroup* tzrussia = new TimezoneGroup(TimezoneGroup::TZ_RUSSIA);
    tzgroups_.push_back( tzrussia );
    tzmap_.insert( std::make_pair("",tzrussia) );

    const char* tzfilename = "timezones.xml";
    try {

        std::auto_ptr<Config> tzcfg( Config::createFromFile(tzfilename) );

        if ( !tzcfg.get()) {
            throw InfosmeException(EXC_CONFIG,"config file '%s' is ill-formed",tzfilename);
        }

        std::auto_ptr< CStrSet > tzset(tzcfg->getRootSectionNames());
        for ( CStrSet::iterator i = tzset->begin(); i != tzset->end(); ++i ) {
            std::auto_ptr< Config > sect(tzcfg->getSubConfig(i->c_str(),true) );
            int tzid = TimezoneGroup::TZ_RUSSIA;
            try {
                const char* algo = sect->getString("tzalgo");
                if ( !algo ||
                     !strcmp(algo,"Russia") ||
                     !strcmp(algo,"russia") ) {
                    tzid = TimezoneGroup::TZ_RUSSIA;
                } else {
                    tzid = TimezoneGroup::TZ_UNKNOWN;
                }
            } catch ( std::exception ) {
                smsc_log_warn(log_,"missing param 'tzalgo' in '%s'",i->c_str());
            }
            for ( std::vector< TimezoneGroup* >::iterator j = tzgroups_.begin();
                  j != tzgroups_.end(); ++j ) {
                if ( (*j)->getId() == tzid ) {
                    smsc_log_debug(log_,"adding tz='%s' to tzgroup=%d",i->c_str(),tzid);
                    tzmap_.insert(std::make_pair(*i,*j));
                    break;
                }
            }
        }
    } catch ( InfosmeException& ) {
        throw;
    } catch ( std::exception& e ) {
        throw InfosmeException(EXC_CONFIG,"exc on tz: %s",e.what());
    }
}


const TimezoneGroup* CommonSettings::lookupTimezoneGroup( const char* name ) const
{
    if ( !name ) { name = ""; }
    TzMap::const_iterator i = tzmap_.find(name);
    if ( i == tzmap_.end() ) {
        smsc_log_debug(log_,"timezone '%s' has not mapping to any tzgroup",name);
        return 0;
    }
    return i->second;
}

} // informer
} // smsc
