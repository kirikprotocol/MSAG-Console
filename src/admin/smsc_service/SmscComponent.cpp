#include "SmscComponent.h"
#include "profiler/profiler.hpp"
#include <mscman/MscManager.h>
#include <resourcemanager/ResourceManager.hpp>

namespace smsc {
    namespace admin {
        namespace smsc_service {


            using namespace smsc::profiler;
            using namespace smsc::core::buffers;
            using smsc::mscman::MscManager;
            using smsc::mscman::MscInfo;

            const char PROFILE_PARAMS_DELIMITER = ',';


            SmscComponent::SmscComponent(SmscConfigs &all_configs)
            : configs(all_configs),
                            logger(Logger::getCategory("smsc.admin.smsc_service.SmscComponent"))
            {
                Parameters empty_params;
                Parameters lookup_params;
                lookup_params["address"] = Parameter("address", StringType);
                Parameters update_params;
                update_params["address"] = Parameter("address", StringType);
                update_params["profile"] = Parameter("profile", StringType);
                Parameters cancelMessage_params;
                cancelMessage_params["ids"] = Parameter("ids", StringType);
                cancelMessage_params["sources"] = Parameter("sources", StringType);
                cancelMessage_params["destinations"] = Parameter("destinations", StringType);
                Parameters msc_params;
                msc_params["msc"] = Parameter("msc", StringType);

                Method apply_routes     ((unsigned)applyRoutesMethod,     "apply_routes",      empty_params, StringType);
                Method apply_aliases    ((unsigned)applyAliasesMethod,    "apply_aliases",     empty_params, StringType);
                Method apply_smsc_config((unsigned)applySmscConfigMethod, "apply_smsc_config", empty_params, StringType);
                Method apply_services   ((unsigned)applyServicesMethod,   "apply_services",    empty_params, StringType);

                Method lookup_profile((unsigned)lookupProfileMethod, "lookup_profile", lookup_params, StringType);
                Method update_profile((unsigned)updateProfileMethod, "update_profile", update_params, LongType);

                Method flush_statistics       ((unsigned)flushStatisticsMethod,       "flush_statistics",        empty_params,         StringType);
                Method process_cancel_messages((unsigned)processCancelMessagesMethod, "process_cancel_messages", cancelMessage_params, StringType);

                Method msc_registrate((unsigned)mscRegistrateMethod, "msc_registrate", msc_params,   StringType);
                Method msc_unregister((unsigned)mscUnregisterMethod, "msc_unregister", msc_params,   StringType);
                Method msc_block     ((unsigned)mscBlockMethod,      "msc_block",      msc_params,   StringType);
                Method msc_clear     ((unsigned)mscClearMethod,      "msc_clear",      msc_params,   StringType);
                Method msc_list      ((unsigned)mscListMethod,       "msc_list",       empty_params, StringListType);

                methods[apply_routes     .getName()] = apply_routes;
                methods[apply_aliases    .getName()] = apply_aliases;
                methods[apply_smsc_config.getName()] = apply_smsc_config;
                methods[apply_services   .getName()] = apply_services;

                methods[lookup_profile.getName()] = lookup_profile;
                methods[update_profile.getName()] = update_profile;

                methods[flush_statistics       .getName()] = flush_statistics;
                methods[process_cancel_messages.getName()] = process_cancel_messages;

                methods[msc_registrate.getName()] = msc_registrate;
                methods[msc_unregister.getName()] = msc_unregister;
                methods[msc_block     .getName()] = msc_block;
                methods[msc_clear     .getName()] = msc_clear;
                methods[msc_list      .getName()] = msc_list;

                smsc_app_runner.reset(0);
            }

            SmscComponent::~SmscComponent()
            {
                smsc_app_runner.reset(0);
            }



            Variant SmscComponent::call(const Method & method, const Arguments & args)
            throw (AdminException)
            {
                try 
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
                    case flushStatisticsMethod:
                        return Variant(flushStatistics(args).c_str());
                    case processCancelMessagesMethod:
                        return Variant(processCancelMessages(args).c_str());
                    case applySmscConfigMethod:
                        applySmscConfig();
                        return Variant("");
                    case applyServicesMethod:
                        applyServices();
                        return Variant("");
    
                    case mscRegistrateMethod:
                        mscRegistrate(args);
                        return Variant("");
                    case mscUnregisterMethod:
                        mscUnregister(args);
                        return Variant("");
                    case mscBlockMethod:
                        mscBlock(args);
                        return Variant("");
                    case mscClearMethod:
                        mscClear(args);
                        return Variant("");
                    case mscListMethod:
                        return mscList();
    
                    default:
                        logger.debug("unknown method \"%s\" [%u]", method.getName(), method.getId());
                        throw AdminException("Unknown method \"%s\"", method.getName());
                    }
                    logger.error("call \"%s\"[%u] done. Unreacheable code reached.", method.getName(), method.getId());
                    return Variant("");
                }
                catch (AdminException &e)
                {
                    throw e;
                }
                catch (...)
                {
                    throw AdminException("Unknown exception catched during call");
                }
            }

            void SmscComponent::mscRegistrate(const Arguments & args)
            {
                MscManager::getMscAdmin().registrate(args.Get("msc").getStringValue());
            }
            void SmscComponent::mscUnregister(const Arguments & args)
            {
                MscManager::getMscAdmin().unregister(args.Get("msc").getStringValue());
            }
            void SmscComponent::mscBlock(const Arguments & args)
            {
                MscManager::getMscAdmin().block(args.Get("msc").getStringValue());
            }
            void SmscComponent::mscClear(const Arguments & args)
            {
                MscManager::getMscAdmin().clear(args.Get("msc").getStringValue());
            }
            Variant SmscComponent::mscList()
            {
                const Array<MscInfo> & mscInfos = MscManager::getMscAdmin().list();
                Variant result(StringListType);
                for (int i=0; i<mscInfos.Count(); i++)
                {
                    char buffer[256];
                    snprintf(buffer, 255, "%i", mscInfos[i].failureCount);
                    std::string  buf;
                    buf += mscInfos[i].mscNum;
                    buf += ',';
                    buf += mscInfos[i].manualLock ? "true" : "false";
                    buf += ',';
                    buf += mscInfos[i].automaticLock ? "true" : "false";
                    buf += ',';
                    buf += buffer;
                    result.appendValueToStringList(buf.c_str());
                }
                return result;
/*              Variant result(StringListType);
                result.appendValueToStringList("test1");
                result.appendValueToStringList("test2");
                result.appendValueToStringList("test3");
                result.appendValueToStringList("a1");
                result.appendValueToStringList("a3");
                result.appendValueToStringList("a2");
                return result;*/
            }


            void SmscComponent::applyServices()
            throw (AdminException)
            {
                logger.info("applying new services...");
                reloadConfigsAndRestart();
                logger.info("new services applied.");
            }

            void SmscComponent::applySmscConfig()
            throw (AdminException)
            {
                logger.info("applying new configs...");
                reloadConfigsAndRestart();
                logger.info("new config applied.");
            }

            void SmscComponent::reloadConfigsAndRestart()
            throw (AdminException)
            {
                try
                {
                    stopSmsc();
                    reReadConfigs();
                    runSmsc();
                }
                catch (std::exception &e)
                {
                    logger.error("Couldn't apply new config: %s", e.what());
                    return;
                }
                catch (...)
                {
                    logger.error("Couldn't apply new config: unknown exception");
                    return;
                }
            }

            std::string SmscComponent::flushStatistics(const Arguments &args)
            throw (AdminException)
            {
                logger.debug("flushStatistics");
                try
                {
                    smsc_app_runner->getApp()->flushStatistics();
                }
                catch (std::exception &e)
                {
                    logger.error("Exception on flush statistics: %s", e.what());
                    throw AdminException("Exception on flush statistics");
                }
                catch (...)
                {
                    logger.error("Unknown exception on flush statistics");
                    throw AdminException("Unknown exception on flush statistics");
                }
                return "";
            }

            typedef Array<std::string> StrArray;
            StrArray parseStringToStringList(const char * const strlist)
            {
                StrArray result;
                const char * str = strlist;
                for (const char * ptr = strchr(str, ','); ptr != 0; ptr = strchr(str, ','))
                {
                    while (str[0] == ' ')   str++;
                    const int length = ptr-str;
                    char * value_ = new char[length+1];
                    memcpy(value_, str, length*sizeof(char));
                    value_[length] = 0;
                    std::string value(value_);
                    result.Push(value);
                    str = ptr+1;
                }
                if (strlen(str) > 0)
                {
                    while (str[0] == ' ')   str++;
                    result.Push(str);
                }
                return result;
            }

            void SmscComponent::processCancelMessage(const std::string &sid, const std::string &ssrc, const std::string &sdst)
            {
                try
                {
                    SMSId id = 0;
                    sscanf(sid.c_str(), "%llx", &id);
                    Address src(ssrc.c_str());
                    Address dst(sdst.c_str());
#ifdef SMSC_DEBUG
                    char src_[16+src.getLength()];
                    src.toString(src_, sizeof(src_)/sizeof(char));
                    char dst_[16+src.getLength()];
                    dst.toString(dst_, sizeof(dst_)/sizeof(char));
                    __trace2__("processCancelMessage: %llx[%s][%s]\n", id, src_, dst_);
#endif
                    smsc_app_runner->getApp()->cancelSms(id, src, dst);
                    logger.info("message %s[%s][%s] canceled", sid.c_str(), ssrc.c_str(), sdst.c_str());
                }
                catch (...)
                {
                    logger.error("Couldn't cancel message %s[%s][%s]", sid.c_str(), ssrc.c_str(), sdst.c_str());
                }
            }

            std::string SmscComponent::processCancelMessages(const Arguments &args)
            throw (AdminException)
            {
                try
                {
                    const char * const idsStr = args.Get("ids").getStringValue();
                    const char * const sourcesStr = args.Get("sources").getStringValue();
                    const char * const destinationsStr = args.Get("destinations").getStringValue();
                    logger.info("processCancelMessages: [%s], [%s], [%s]", idsStr, sourcesStr, destinationsStr);

                    StrArray ids = parseStringToStringList(idsStr);
                    StrArray sources = parseStringToStringList(sourcesStr);
                    StrArray destinations = parseStringToStringList(destinationsStr);
                    if ((ids.Count() != sources.Count()) 
                            || (ids.Count() != destinations.Count()) 
                            || (destinations.Count() != sources.Count()))
                    {
                        logger.error("processCancelMessages failed: incorrect params");
                        throw AdminException("incorrect params");
                    }

                    for (int i = 0; i < ids.Count(); i++)
                    {
                        std::string sid(ids[i]);
                        std::string ssrc(sources[i]);
                        std::string sdst(destinations[i]);

                        processCancelMessage(sid, ssrc, sdst);
                    }

                    return "";
                }
                catch (HashInvalidKeyException &e)
                {
                    logger.error("processCancelMessages failed: not all parameters defined");
                    throw AdminException("not all parameters defined");
                }
                catch (...)
                {
                    logger.error("processCancelMessages failed: unknown reason");
                    throw AdminException("unknown exception");
                }
            }

            void SmscComponent::runSmsc()
            throw (AdminException)
            {
                smsc::core::synchronization::MutexGuard guard(mutex);
                if (smsc_app_runner.get() == 0)
                {
                    try
                    {
                        smsc_app_runner.reset(new SmscAppRunner(configs));
                        smsc_app_runner->Start();
                    }
                    catch (smsc::util::Exception &e)
                    {
                        logger.error("Exception on starting SMSC: \"%s\"", e.what());
                        throw AdminException("Exception on starting SMSC: \"%s\"", e.what());
                    }
                    catch (std::exception &e)
                    {
                        logger.error("Exception on starting SMSC: \"%s\"", e.what());
                        throw AdminException("Exception on starting SMSC: \"%s\"", e.what());
                    }
                    catch (...)
                    {
                        logger.error("Unknown exception on starting SMSC");
                        throw AdminException("Unknown exception on starting SMSC");
                    }
                }
                else
                    throw   AdminException("SMSC Application started already");
            }

            void SmscComponent::stopSmsc()
            throw (AdminException)
            {
                smsc::core::synchronization::MutexGuard guard(mutex);
                if (smsc_app_runner.get() != 0)
                {
                    try
                    {
                        smsc_app_runner->stop();
                        smsc_app_runner->WaitFor();
                        smsc_app_runner.reset(0);
                    }
                    catch (smsc::util::Exception &e)
                    {
                        logger.error("Exception on starting SMSC: \"%s\"", e.what());
                        throw AdminException("Exception on stopping SMSC: \"%s\"", e.what());
                    }
                    catch (std::exception &e)
                    {
                        logger.error("Exception on stopping SMSC: \"%s\"", e.what());
                        throw AdminException("Exception on starting SMSC: \"%s\"", e.what());
                    }
                    catch (...)
                    {
                        logger.error("Unknown exception on stopping SMSC");
                        throw AdminException("Unknown exception on starting SMSC");
                    }
                }
                else
                    throw   AdminException("SMSC Application started already");
            }

            void SmscComponent::abort()
            {
                smsc::core::synchronization::MutexGuard guard(mutex);
                if (smsc_app_runner.get() != 0)
                {
                    try
                    {
                        smsc_app_runner->abort();
                        smsc_app_runner->WaitFor();
                        smsc_app_runner.reset(0);
                    }
                    catch (smsc::util::Exception &e)
                    {
                        logger.error("Exception on starting SMSC: \"%s\"", e.what());
                        throw AdminException("Exception on aborting SMSC: \"%s\"", e.what());
                    }
                    catch (std::exception &e)
                    {
                        logger.error("Exception on aborting SMSC: \"%s\"", e.what());
                    }
                    catch (...)
                    {
                        logger.error("Unknown exception on aborting SMSC");
                    }
                }
            }

            void SmscComponent::dump()
            {
                smsc::core::synchronization::MutexGuard guard(mutex);
                if (smsc_app_runner.get() != 0)
                {
                    try
                    {
                        smsc_app_runner->dump();
                        smsc_app_runner->WaitFor();
                        smsc_app_runner.reset(0);
                    }
                    catch (smsc::util::Exception &e)
                    {
                        logger.error("Exception on starting SMSC: \"%s\"", e.what());
                        throw AdminException("Exception on dumping SMSC: \"%s\"", e.what());
                    }
                    catch (std::exception &e)
                    {
                        logger.error("Exception on dumping SMSC: \"%s\"", e.what());
                    }
                    catch (...)
                    {
                        logger.error("Unknown exception on dumping SMSC");
                    }
                }
            }

            void SmscComponent::applyRoutes()
            throw (AdminException)
            {
                configs.routesconfig->reload();
                configs.smemanconfig->reload();
                smsc_app_runner->getApp()->reloadRoutes(configs);
            }

            void SmscComponent::applyAliases()
            throw (AdminException)
            {
                configs.aliasconfig->reload();
                smsc_app_runner->getApp()->reloadAliases(configs);
            }

            void SmscComponent::reReadConfigs()
            throw (AdminException)
            {
                configs.smemanconfig->reload();
                __trace2__("reinit smsc config\n");
                configs.cfgman->reinit();
                configs.cfgman = & smsc::util::config::Manager::getInstance();
                smsc::resourcemanager::ResourceManager::init(configs.cfgman->getString("core.locales"), configs.cfgman->getString("core.default_locale"));
                __trace2__("config reinitialized\n");
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
                        switch (profile.reportoptions)
                        {
                        case smsc::profiler::ProfileReportOptions::ReportFull:
                            result += "full";
                            break;
                        case smsc::profiler::ProfileReportOptions::ReportFinal:
                            result += "final";
                            break;
                        case smsc::profiler::ProfileReportOptions::ReportNone:
                            result += "none";
                            break;
                        default:
                            result += "unknown";
                        }
                        result += PROFILE_PARAMS_DELIMITER;
                        result += profile.locale;
                        result += PROFILE_PARAMS_DELIMITER;
                        result += (profile.hide) ? "true":"false";
                        result += PROFILE_PARAMS_DELIMITER;
                        result += (profile.hideModifiable) ? "true":"false";
                        return result;
                    }
                    else
                        throw   AdminException("SMSC is not running");
                }
                catch (HashInvalidKeyException & e)
                {
                    throw AdminException("Address not defined");
                }
            }

            void setProfileFromString(Profile &profile, const char * const profileString)
            throw (AdminException)
            {
                const int PARAM_NUMBER = 5;
                
                const size_t length = (profileString) ? strlen(profileString):0;
                if (length <= 0) 
                    throw AdminException("profile options misformatted");

                char profileStr[length+1]; strcpy(profileStr, profileString);
                char delimeterStr[2]; char* currToken = 0;
                delimeterStr[0] = PROFILE_PARAMS_DELIMITER; delimeterStr[1] = 0;
                
                char* codepageStr       = strtok_r(profileStr, delimeterStr, &currToken); 
                char* reportStr         = strtok_r(NULL, delimeterStr, &currToken);
                char* localeStr         = strtok_r(NULL, delimeterStr, &currToken);
                char* hideStr           = strtok_r(NULL, delimeterStr, &currToken);
                char* hideModifiableStr = strtok_r(NULL, delimeterStr, &currToken);
                
                if (!codepageStr || !reportStr || !localeStr || 
                    !hideStr || !hideModifiableStr) 
                    throw AdminException("profile options misformatted");

                /*__trace2__("%s,%s,%s,%s,%s", 
                           codepageStr, reportStr, localeStr, hideStr, hideModifiableStr);*/
                
                if (strcmp("default", codepageStr) == 0) {
                    profile.codepage = smsc::profiler::ProfileCharsetOptions::Default;
                }
                else if (strcmp("UCS2", codepageStr) == 0) {
                    profile.codepage = smsc::profiler::ProfileCharsetOptions::Ucs2;
                }
                else
                    throw   AdminException("Unknown charset");

                if (strcmp("full", reportStr) == 0) {
                    profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportFull;
                }
                else if (strcmp("final", reportStr) == 0) {
                    profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportFinal;
                }
                else if (strcmp("none", reportStr) == 0) {
                    profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportNone;
                }
                else
                    throw   AdminException("Unknown report options");

                profile.locale = localeStr;
                
                profile.hide = 
                    (strcmp("true", hideStr) == 0) ? 1:0;
                profile.hideModifiable = 
                    (strcmp("true", hideModifiableStr) == 0) ? true:false;
            }

            bool isMask(const Address & address)
            {
                for (unsigned i=0; i<address.length; i++)
                {
                    if (address.value[i] == '?')
                        return true;
                }
                return false;
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
                        logger.debug("Update Profile:\n  %s: Address: \"%s\", codepage:%u, report options:%u, locale:%s", addressString, addr_str, profile.codepage, profile.reportoptions, profile.locale.c_str());
#endif
                        if (isMask(address))
                            return profiler->updatemask(address, profile);
                        else
                            return profiler->update(address, profile);
                    }
                    else
                        throw   AdminException("SMSC is not running");
                }
                catch (HashInvalidKeyException &e)
                {
                    throw AdminException("Address or profile params not defined");
                }
            }

        }
    }
}

