#include "SmscComponent.h"
#include "profiler/profiler.hpp"

namespace smsc {
namespace admin {
namespace smsc_service {

using namespace smsc::profiler;
using namespace smsc::core::buffers;

const char PROFILE_PARAMS_DELIMITER = ',';

Variant SmscComponent::call(const Method & method, const Arguments & args)
	throw (AdminException)
{
  logger.debug("call \"%s\"", method.getName());
  switch (method.getId())
  {
  case applyRoutesMethod:
    logger.debug("applying routes...");
    applyRoutes();
    logger.debug("routes applied");
    return Variant("");
  case applyAliasesMethod:
    logger.debug("applying aliases...");
    applyAliases();
    logger.debug("aliases applied");
    return Variant("");
  case lookupProfileMethod:
    logger.debug("lookup profile...");
    return Variant(lookupProfile(args).c_str());
  case updateProfileMethod:
    logger.debug("update profile...");
    return Variant((long)updateProfile(args));
  default:
    logger.debug("unknown method \"%s\" [%u]", method.getName(), method.getId());
    throw AdminException("Unknown method \"%s\"", method.getName());
  }
  logger.error("call \"%s\"[%u] done. Unreacheable code reached.", method.getName(), method.getId());
  return Variant("");
}

void SmscComponent::runSmsc()
	throw (AdminException)
{
  smsc::core::synchronization::MutexGuard guard(mutex);
	if (smsc_app_runner.get() == 0)
	{
		smsc_app_runner.reset(new SmscAppRunner(configs));
		smsc_app_runner->Start();
	}
	else
		throw AdminException("SMSC Application started already");
}

void SmscComponent::stopSmsc()
	throw (AdminException)
{
  smsc::core::synchronization::MutexGuard guard(mutex);
	if (smsc_app_runner.get() != 0)
	{
		smsc_app_runner->stop();
		smsc_app_runner->WaitFor();
		smsc_app_runner.reset(0);
	}
	else
		throw AdminException("SMSC Application started already");
}

void SmscComponent::applyRoutes()
	throw (AdminException)
{
	stopSmsc();
	reReadConfigs();
	runSmsc();
}

void SmscComponent::applyAliases()
	throw (AdminException)
{
	applyRoutes();
}

void SmscComponent::reReadConfigs()
	throw (AdminException)
{
	configs.smemanconfig->reload();
	configs.cfgman->reinit();
  configs.aliasconfig->reload();
  configs.routesconfig->reload();
}

std::string SmscComponent::lookupProfile(const Arguments &args)
    throw (AdminException)
{
    try
    {
        const char * const addressString = args.Get("address").getStringValue();

        Smsc * app;
        ProfilerInterface * profiler;
        if (isSmscRunning() && (app = smsc_app_runner->getApp()) && (profiler = app->getProfiler())) 
        {
            Address address(addressString);
            //app->AliasToAddress(alias, address);
            Profile& profile(profiler->lookup(address));
            #ifdef SMSC_DEBUG
              char addr_str[smsc::sms::MAX_ADDRESS_VALUE_LENGTH+1];
              address.getValue(addr_str);
              logger.debug("lookup Profile:\n  %s: Address: \"%s\"[%u], numebring plan:%u, type of number:%u, ", addressString, addr_str, address.getLength(), address.getNumberingPlan(), address.getTypeOfNumber());
            #endif
            std::string result;
            switch (profile.codepage) 
            {
            case smsc::profiler::ProfileCharsetOptions::Default:
                result += "default";
                break;
            case smsc::profiler::ProfileCharsetOptions::Ucs2:
                result += "UCS2";
                break;
            default:
                result += "unknown";
            }
            result += PROFILE_PARAMS_DELIMITER;
            switch (profile.reportoptions) {
            case smsc::profiler::ProfileReportOptions::ReportFull:
                result += "full";
                break;
            case smsc::profiler::ProfileReportOptions::ReportNone:
                result += "none";
                break;
            default:
                result += "unknown";
            }
            return result;
        }
        else
            throw AdminException("SMSC is not running");
    }
    catch (HashInvalidKeyException & e)
    {
        throw AdminException("Address not defined");
    }
}

void setProfileFromString(Profile &profile, const char * const profileString)
    throw (AdminException)
{
    const char * const delimiter = strchr(profileString, PROFILE_PARAMS_DELIMITER);
    const size_t length = strlen(profileString);
    if (delimiter) {
        if (memcmp("default", profileString, delimiter - profileString) == 0) {
            profile.codepage = smsc::profiler::ProfileCharsetOptions::Default;
        } else if (memcmp("UCS2", profileString, delimiter - profileString) == 0) {
            profile.codepage = smsc::profiler::ProfileCharsetOptions::Ucs2;
        } else 
            throw AdminException("Unknown codepage");
    
        if (strcmp("full", delimiter+1) == 0) {
            profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportFull;
        } else if (strcmp("none", delimiter+1) == 0) {
            profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportNone;
        } else 
            throw AdminException("Unknown codepage");
    } 
    else 
        throw AdminException("profile options misformatted");
}

int SmscComponent::updateProfile(const Arguments & args) 
{
    try
    {
        const char * const addressString = args.Get("address").getStringValue();
        const char * const profileString = args.Get("profile").getStringValue();

        Smsc * app;
        ProfilerInterface * profiler;
        if (isSmscRunning() && (app = smsc_app_runner->getApp()) && (profiler = app->getProfiler())) 
        {
            Address address(addressString);
            Profile profile;
            setProfileFromString(profile, profileString);
            #ifdef SMSC_DEBUG
              char addr_str[smsc::sms::MAX_ADDRESS_VALUE_LENGTH+9];
              address.toString(addr_str, sizeof(addr_str)/sizeof(addr_str[0]));
              logger.debug("Update Profile:\n  %s: Address: \"%s\", codepage:%u, report options:%u, ", addressString, addr_str, profile.codepage, profile.reportoptions);
            #endif
            return profiler->update(address, profile);
        }
        else
            throw AdminException("SMSC is not running");
    }
    catch (HashInvalidKeyException &e)
    {
        throw AdminException("Address or profile params not defined");
    }
}

}
}
}

