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
          default:
            logger.debug("unknown method \"%s\" [%u]", method.getName(), method.getId());
            throw AdminException("Unknown method \"%s\"", method.getName());
        }
        logger.error("call \"%s\"[%u] done. Unreacheable code reached.", method.getName(), method.getId());
        return Variant("");
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
          while (str[0] == ' ') str++;
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
          while (str[0] == ' ') str++;
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
          throw AdminException("SMSC Application started already");
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
          throw AdminException("SMSC Application started already");
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
            switch (profile.reportoptions)
            {
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
        if (delimiter)
        {
          if (memcmp("default", profileString, delimiter - profileString) == 0)
          {
            profile.codepage = smsc::profiler::ProfileCharsetOptions::Default;
          }
          else if (memcmp("UCS2", profileString, delimiter - profileString) == 0)
          {
            profile.codepage = smsc::profiler::ProfileCharsetOptions::Ucs2;
          }
          else
            throw AdminException("Unknown codepage");

          if (strcmp("full", delimiter+1) == 0)
          {
            profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportFull;
          }
          else if (strcmp("none", delimiter+1) == 0)
          {
            profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportNone;
          }
          else
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

