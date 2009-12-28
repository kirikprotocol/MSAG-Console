#include "TaskTypes.hpp"
#include "ConfString.h"
#include "util/Exception.hpp"
#include "util/config/ConfigView.h"

namespace {
smsc::logger::Logger* log_ = 0;
smsc::core::synchronization::Mutex logMtx;
}


namespace smsc {
namespace infosme {

using namespace smsc::util;
using namespace smsc::util::config;

int Message::stringToRegionId( const std::string& regionId )
{
    int regId = -1;
    if ( regionId == "regDefault" ) {
        // ok
    } else if ( regionId.empty() ) {
        throw Exception("regionId is empty");
    } else {
        const char* regIdStr = regionId.c_str();
        char* endptr;
        regId = int(std::strtol(regIdStr,&endptr,10));
        if ( regId < -1 ) {
            throw Exception("regionId value is wrong: %s",regIdStr);
        } else if ( regId == -1 ) {
            throw Exception("regionId equal to reserved value -1");
        } else if ( *endptr != '\0' ) {
            throw Exception("regionId not fully converted from '%s'",regIdStr);
        }
    }
    return regId;
}


std::string Message::regionIdToString( int regionId )
{
    char buf[30];
    if ( regionId >= 0 ) {
        sprintf(buf,"%d",regionId);
    } else if ( regionId == -1 ) {
        sprintf(buf,"regDefault");
    } else {
        throw Exception("region %d is wrong",regionId);
    }
    return buf;
}


TaskInfo::TaskInfo( uint32_t id ) :
uid(id), enabled(true), priority(0),
retryOnFail(false), replaceIfPresent(false),
trackIntegrity(false), transactionMode(false), keepHistory(false),
saveFinalState(false),
bGenerationSuccess(true),
flash(false),
endDate(-1), validityPeriod(-1), validityDate(-1),
activePeriodStart(-1), activePeriodEnd(-1), activeWeekDays(0),
dsTimeout(0), dsUncommitedInProcess(1), dsUncommitedInGeneration(1),
messagesCacheSize(100), messagesCacheSleep(0), useDataSm(false),
useUssdPush(false)
{}


void TaskInfo::init( ConfigView* config )
{
    if ( !log_ ) {
        MutexGuard mg(logMtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("is2.info");
    }

    try {
        if ( !config ) throw ConfigException("config is null");

        static const unsigned MAX_PRIORITY_VALUE = 1000;

        try { delivery = config->getBool("delivery"); }
        catch (...) { delivery = false; }

        try { name = ConfString(config->getString("name")).str(); }
        catch (...) {}
    
        enabled = config->getBool("enabled");
        priority = config->getInt("priority");
        if ( priority <= 0 || priority > MAX_PRIORITY_VALUE ) {
            throw ConfigException("priority should be positive and less than %u.",
                                  MAX_PRIORITY_VALUE);
        }

        try {
            address = ConfString(config->getString("address")).str();
        } catch (...) {
            smsc_log_warn(log_, "Task %u: <address> parameter missed. "
                          "Using global definitions", uid );
        }
        
        if ( ! address.empty() ) {
            smsc::sms::Address a;
            if ( !convertMSISDNStringToAddress(address.c_str(), a) ) {
                throw ConfigException("Task %u: wrong \"address\" value '%s'",
                                      uid, address.c_str() );
            }
        }

        retryOnFail = config->getBool("retryOnFail");
        replaceIfPresent = config->getBool("replaceMessage");
        transactionMode = config->getBool("transactionMode");
        trackIntegrity = config->getBool("trackIntegrity");
        keepHistory = config->getBool("keepHistory");
        try { saveFinalState = config->getBool("saveFinalState"); }
        catch ( std::exception& e ) { saveFinalState = false; }
        try { flash = config->getBool("flash");
        } catch(std::exception& e) { flash = false; }

        endDate = parseDateTime(ConfString(config->getString("endDate")).str().c_str());
    
        if (retryOnFail) {
            retryPolicy = ConfString(config->getString("retryPolicy")).str();
            if (retryPolicy.empty()) {
                throw ConfigException("retry time specified incorrectly.");
            }
        }
        validityPeriod = parseTime(ConfString(config->getString("validityPeriod")).str().c_str());
        validityDate = parseDateTime(ConfString(config->getString("validityDate")).str().c_str());
        if ( validityPeriod <= 0 && validityDate <= 0 ) {
            throw ConfigException("message validity period/date specified incorrectly.");
        }
        activePeriodStart = parseTime(ConfString(config->getString("activePeriodStart")).str().c_str());
        activePeriodEnd = parseTime(ConfString(config->getString("activePeriodEnd")).str().c_str());
        if ( (activePeriodStart < 0 && activePeriodEnd >= 0) ||
             (activePeriodStart >= 0 && activePeriodEnd < 0)
             /* ||
              (info.activePeriodStart >= 0 && info.activePeriodEnd >= 0 && //remove by request
              info.activePeriodStart >= info.activePeriodEnd)*/ ) {
            throw ConfigException("active period specified incorrectly.");
        }
        
        {
            std::string awd;
            try {
                awd = ConfString(config->getString("activeWeekDays")).str();
            } catch (...) {
                smsc_log_warn(log_, "Task %u: <activeWeekDays> parameter missed. "
                              "Using default: Mon,Tue,Wed,Thu,Fri", uid);
                activeWeekDays.weekDays = 0x7c;
            }
            if ( !awd.empty() && !activeWeekDays.setWeekDays(awd.c_str()) ) {
                throw ConfigException("active week days set listed incorrectly: '%s'",awd.c_str());
            }
        }
        
        if ( !delivery ) {
            querySql = ConfString(config->getString("query")).str();
            if (querySql.empty()) {
                throw ConfigException("sql query is empty");
            }
            msgTemplate = ConfString(config->getString("template")).str();
            if ( msgTemplate.empty() ) {
                throw ConfigException("message template is empty");
            }
        }

        if (replaceIfPresent)
        {
            try         { svcType = ConfString(config->getString("svcType")).str(); }
            catch (...) {}
        }

        try { dsTimeout = config->getInt("dsTimeout"); }
        catch(...) { dsTimeout = 0; }
        if (dsTimeout < 0) dsTimeout = 0;
  
        try { dsUncommitedInProcess = config->getInt("uncommitedInProcess"); }
        catch(...) { dsUncommitedInProcess = 1; }
        if (dsUncommitedInProcess < 0) dsUncommitedInProcess = 1;

        try { dsUncommitedInGeneration = config->getInt("uncommitedInGeneration"); }
        catch(...) { dsUncommitedInGeneration = 1; }
        if (dsUncommitedInGeneration < 0) dsUncommitedInGeneration = 1;

        try { messagesCacheSize = config->getInt("messagesCacheSize"); }
        catch(...) { messagesCacheSize = 100; }
        if (messagesCacheSize <= 0) messagesCacheSize = 100;

        try { messagesCacheSleep = config->getInt("messagesCacheSleep"); }
        catch(...) { messagesCacheSleep = 1; }
        if (messagesCacheSleep <= 0) messagesCacheSleep = 1;

        try { useDataSm = config->getBool("useDataSm"); }
        catch (...) { useDataSm = false; }

        try { bGenerationSuccess = config->getBool("messagesHaveLoaded"); }
        catch (...) { bGenerationSuccess = false; }

        try { useUssdPush = config->getBool("useUssdPush"); }
        catch (...) { useUssdPush = false; }
        if ( useUssdPush ) {
            // overriding things
            transactionMode = true;
            flash = false;
            useDataSm = false;
        }

    } catch ( std::exception& e ) {
        throw ConfigException("Task %u: %s", uid, e.what());
    }
}


bool TaskInfo::convertMSISDNStringToAddress(const char* string, smsc::sms::Address& addr) const
{
    try {
        const smsc::sms::Address converted(string);
        addr = converted;
    } catch (...) {
        return false;
    }
    return true;
}

}
}
