#include "CommonSettingsIniter.h"
#include "CommonSettings.h"
#include "DeadLockWatch.h"
#include "TimezoneGroup.h"
#include "util/config/Config.h"
#include "informer/io/ConfigWrapper.h"
#include "informer/io/InfosmeException.h"

using smsc::util::config::Config;
using smsc::util::config::CStrSet;

namespace eyeline {
namespace informer {

void CommonSettingsIniter::init( CommonSettings& cs,
                                 smsc::util::config::Config& cfg,
                                 SnmpManager*     snmp,
                                 DeadLockWatcher* dlwatch,
                                 UTF8*            utf,
                                 bool             archive )
{
    cs.snmp_ = snmp;
    cs.dlwatcher_ = dlwatch;
    cs.archive_ = archive;
    cs.utf8_ = utf;
    
    // dead lock watcher will be started later
    // cs.dlwatcher_->start();

    const ConfigWrapper conf(cfg,cs.log_);
    cs.emergency_ = conf.getBool("emergency",false);
    if ( cs.emergency_ && cs.archive_ ) {
        throw InfosmeException( EXC_CONFIG, "archive daemon can not have 'emergency' flag");
    }

    cs.path_ = conf.getString("storePath");
    if (cs.path_.empty()) cs.path_ = "./";
    else if (cs.path_[cs.path_.size()-1] != '/') {
        cs.path_.append("/");
    }

    cs.archivePath_ = conf.getString("archivePath");
    if (cs.archivePath_.empty()) cs.archivePath_ = "./";
    else if (cs.archivePath_[cs.archivePath_.size()-1] != '/') {
        cs.archivePath_.append("/");
    }

    if ( cs.archive_ ) {
        cs.path_.swap(cs.archivePath_);
    }

    cs.statpath_ = conf.getString("statPath");
    if (cs.statpath_.empty()) cs.statpath_ = "./";
    else if (cs.statpath_[cs.statpath_.size()-1] != '/') {
        cs.statpath_.append("/");
    }

    cs.unrespondedMessagesMax_ = conf.getInt("unrespondedMessagesMax",100,10,10000);
    cs.responseWaitTime_ = conf.getInt("responseWaitTime",30,10,120);
    cs.svcType_ = conf.getString("svcType","Info");
    cs.protocolId_ = conf.getInt("protocolId",1,1,0);
    cs.slicedMessageSize_ = conf.getInt("slicedMessageSize",160,120,250);
    cs.inputMinQueueTime_ = conf.getInt("inputMinQueueSize",5,0,100);
    cs.inputTransferChunkTime_ = conf.getInt("inputTransferChunkSize",60,10,300);
    cs.inputRequestAverageTime_ = conf.getInt("inputRequestAverageTime",1,1,30);
    cs.receiptExtraWaitTime_ = conf.getInt("receiptExtraWaitTime",30,5,120);
    cs.retryMinTimeToLive_ = conf.getInt("retryMinTimeToLive",30,10,120);
    cs.skippedIdsMinCacheSize_ = conf.getInt("skippedIdsMinCacheSize",5,0,1000);
    cs.skippedIdsChunkSize_ = conf.getInt("skippedIdsChunkSize",256,100,10000);
    cs.inputJournalRollingPeriod_ = conf.getInt("inputJournalRollingPeriod",300,30,1200);
    cs.operationalJournalRollingPeriod_ = conf.getInt("operationalJournalRollingPeriod",300,30,1200);
    cs.smscJournalRollingPeriod_ = conf.getInt("smscJournalRollingPeriod",10,10,1200);
    cs.inputTransferThreadCount_ = conf.getInt("inputTransferThreadCount",30,10,100);
    cs.resendIOThreadCount_ = conf.getInt("resendIOThreadCount",10,10,30);
    cs.regionRandomizationJitter_ = conf.getInt("regionRandomizationJitter",12451,100,2000000);
    
    cs.validityPeriodDefault_ = conf.getInt("validityPeriodDefault",3600,300,100000);
    cs.messageTimeToLiveDefault_ = conf.getInt("messageTimeToLiveDefault",3600,300,100000);

    cs.inputJournalRollingSpeed_ = conf.getInt("inputJournalRollingSpeed",30,1,100000);
    cs.operationalJournalRollingSpeed_ = conf.getInt("operationalJournalRollingSpeed",30,1,100000);
    cs.smscJournalRollingSpeed_ = conf.getInt("smscJournalRollingSpeed",30,1,100000);

    // cs.regionSpeedLimitNBins_ = conf.getInt("regionSpeedLimitNBins",25,10,100);
    // cs.regionSpeedLimitInterval_ = conf.getInt("regionSpeedLimitInterval",5,1,10);
    cs.regionSpeedLimitSpeedup_ = conf.getInt("regionSpeedLimitSpeedup",10,5,200);
    // cs.regionSpeedLimitBaseSpeed_ = conf.getInt("regionSpeedLimitBaseSpeed",1,1,200);

    cs.recalcTTLRequested_ = conf.getBool("recalcTTLRequested",true);

#ifdef CHECKCONTENTION
    const unsigned cl = conf.getInt("contentionLimit",300000,10000,10000000);
    smsc::core::synchronization::Mutex::setContentionLimit(cl);
#endif

    loadTimezones( cs );
}


void CommonSettingsIniter::loadTimezones( CommonSettings& cs )
{
    cs.tzgroups_.push_back( new TimezoneGroup(TimezoneGroup::TZ_UNKNOWN) );
    TimezoneGroup* tzrussia = new TimezoneGroup(TimezoneGroup::TZ_RUSSIA);
    cs.tzgroups_.push_back( tzrussia );
    cs.tzmap_.insert( std::make_pair("",tzrussia) );

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
                smsc_log_warn(cs.log_,"missing param 'tzalgo' in '%s'",i->c_str());
            }
            for ( std::vector< TimezoneGroup* >::iterator j = cs.tzgroups_.begin();
                  j != cs.tzgroups_.end(); ++j ) {
                if ( (*j)->getId() == tzid ) {
                    smsc_log_debug(cs.log_,"adding tz='%s' to tzgroup=%d",i->c_str(),tzid);
                    cs.tzmap_.insert(std::make_pair(*i,*j));
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

}
}
