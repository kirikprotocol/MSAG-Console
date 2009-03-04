#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>
#include <unistd.h>

#include <signal.h>
#include <string>
#include <vector>
#include <algorithm>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include "scag/config/ConfigManager.h"
#include "scag/pers/upload/PersClient.h"
#include "scag/pers/upload/PersKey.h"
#include "scag/pvss/base/Types.h"
#include "scag/util/storage/SerialBuffer.h"

using std::string;
using std::vector;
using scag::pers::util::PersClient;
using scag::pers::util::PersClientException;
using namespace smsc::util::config;
using namespace scag2::pvss;
using scag2::pvss::Property;
using scag2::pers::util::PersKey;
using scag::util::storage::SerialBufferOutOfBounds;

bool stopped = false;

extern "C" void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("pers");
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        smsc_log_info(logger, "Stopping ...");
        stopped = true;
    }
    else if(sig == SIGHUP)
    {
        smsc_log_info(logger, "Reloading logger config");
        smsc::logger::Logger::Reload();
    }
}

extern "C" void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

int sendProperty(PersClient& pc, PersKey& key, scag2::pvss::Property& prop, smsc::logger::Logger* logger) {
  try {
    int propVal = rand() % 2;
    prop.setBoolValue(static_cast<bool>(propVal));
    pc.SetProperty(PT_ABONENT, key, prop);
    return 1;
  } catch (const PersClientException& ex) {
    smsc_log_warn(logger, "error sending property to abonent '%s'", key.skey);
    return 0;
  } catch (const SerialBufferOutOfBounds &e) {
    smsc_log_warn(logger, "error sending property to abonent '%s'", key.skey);
    return 0;
  }
}

int sendProperties(int number, PersClient& pc, scag2::pvss::Property& cmbprop, scag2::pvss::Property& pmbprop, smsc::logger::Logger* logger) {
  char addr[20];
  memset(addr, 0, 20);
  sprintf(addr, "7911%07d", number);			
  PersKey key(addr);
  int sendCount = 0;
  bool sendcmb = static_cast<bool>(rand() % 2);
  bool sendpmb = static_cast<bool>(rand() % 2);
  if (!sendpmb && !sendcmb) {
    bool sendpmbcmb = static_cast<bool>(rand() % 2);
    if (sendpmbcmb) {
      sendpmb = true;
    } else {
      sendcmb = true;
    }
  }
  if (sendcmb) {
    if (!sendProperty(pc, key, cmbprop, logger)) {
      smsc_log_warn(logger, "resending cmb property to abonent '%s'", key.skey);
      sendCount += sendProperty(pc, key, cmbprop, logger);
    } else {
      ++sendCount;
    }
  }
  if (sendpmb) {
    if (!sendProperty(pc, key, pmbprop, logger)) {
      smsc_log_warn(logger, "resending pmb property to abonent '%s'", key.skey);
      sendCount += sendProperty(pc, key, pmbprop, logger);
    } else {
      ++sendCount;
    }
  }
  return sendCount > 0 ? 1 : 0;
}

int main(int argc, char* argv[])
{
    Logger::Init();
    Logger* logger = Logger::getInstance("flooder");

    atexit(atExitHandler);
    sigset_t set;    
    sigfillset(&set);
    sigdelset(&set, SIGTERM);
    sigdelset(&set, SIGINT);
    sigdelset(&set, SIGSEGV);
    sigdelset(&set, SIGBUS);
    sigdelset(&set, SIGHUP);
    sigprocmask(SIG_SETMASK, &set, NULL);
    sigset(SIGTERM, appSignalHandler);
    sigset(SIGINT, appSignalHandler);
    sigset(SIGHUP, appSignalHandler);    

    int resultCode = 0;
    try{
        smsc_log_info(logger,  "Starting up Pers flooder...");
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView clientConfig(manager, "PersClient");

        string host("phoenix");
        try { 
          host = clientConfig.getString("host");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PersClient.host> missed. Defaul value is %s", host.c_str());
        };
        int port = 27880;
        try { 
          port = clientConfig.getInt("port");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PersClient.port> missed. Defaul value is %d", port);
        };
        int timeout = 1000;
        try { 
          timeout = clientConfig.getInt("ioTimeout");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PersClient.ioTimeout> missed. Defaul value is %d", timeout);
        };
        int pingtimeout = 300;
        try { 
          pingtimeout = clientConfig.getInt("pingTimeout");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PersClient.pingTimeout> missed. Defaul value is %d", pingtimeout);
        };
        int recontimeout = 10;
        try { 
          recontimeout = clientConfig.getInt("reconnectTimeout");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PvssClient.reconnectTimeout> missed. Defaul value is %d", recontimeout);
        };
        int maxwait = 1000;
        try { 
          maxwait = clientConfig.getInt("maxWaitingRequestsCount");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PvssClient.maxWaitingRequestsCount> missed. Defaul value is %d", maxwait);
        };

        ConfigView flooderConfig(manager, "Flooder");
		

        int speed = 100;
        try { 
          speed = flooderConfig.getInt("speed");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <Flooder.speed> missed. Defaul value is %d", speed);
        };
        int addressesCount = 5000000;
        try { 
          addressesCount = flooderConfig.getInt("addressesCount");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <Flooder.addressesCount> missed. Defaul value is %d", addressesCount);
        };

        vector<int> numbers;
        numbers.reserve(addressesCount);
        for (int i = 1; i <= addressesCount; ++i) {
          numbers.push_back(i);
        }
        std::random_shuffle(numbers.begin(), numbers.end());


        smsc_log_info(logger, "Client connect to %s:%d", host.c_str(), port);  
        PersClient::Init(host.c_str(), port, timeout, pingtimeout, recontimeout, maxwait, speed);
        PersClient& pc = PersClient::Instance();

        string strPropName  = "some.string.var";
        string strPropValue = "some text";
        string intPropName  = "other.int.var";
        string cmb = "cmb.subscription";
        string pmb = "pmb.subscription";
        int intPropValue    = 354;

        unsigned number = 0;
        int sendCount = 0;
        char addr[20];
        memset(addr, 0, 20);
        Property intProp(intPropName.c_str(), intPropValue, scag2::pvss::INFINIT, 0, 0);
        Property strProp(strPropName.c_str(), strPropName.c_str(), scag2::pvss::INFINIT, 0, 0);
        Property cmbProp;
        Property pmbProp;
        cmbProp.setName(cmb.c_str());
        pmbProp.setName(pmb.c_str());
        vector<int> notsendnumbers;
        int needProfilesCount = 1000000;
        int i = 0;
        for (vector<int>::iterator n = numbers.begin(); i < needProfilesCount || n != numbers.end(); ++n, ++i) {
          if (stopped) {
            smsc_log_warn(logger, "Abort. Not all profiles created: %d", sendCount);  
            exit(0);
          }
          if (!sendProperties(*n, pc, cmbProp, pmbProp, logger)) {
            notsendnumbers.push_back(*n);  
          } else {
            ++sendCount;
          }
        }
        //if (sendCount < addressesCount) {
        if (sendCount < needProfilesCount) {
          smsc_log_warn(logger, "Not all profiles created: %d", sendCount);  
          for (vector<int>::iterator n = notsendnumbers.begin(); n != notsendnumbers.end(); ++n) {
            smsc_log_warn(logger, "profile number %d not created", *n);  
          }
        } else {
          smsc_log_info(logger, "created profiles count %d", sendCount);  
        }
    }

    catch (const PersClientException& exc) 
    {
        smsc_log_error(logger, "PersClientException: %s Exiting.", exc.what());
        resultCode = -1;
    }
    catch (const ConfigException& exc) 
    {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -2;
    }   
    catch (const Exception& exc) 
    {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (const std::exception& exc) 
    {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    }                
    catch (...) 
    {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -5;
    }   
    return resultCode;
}
