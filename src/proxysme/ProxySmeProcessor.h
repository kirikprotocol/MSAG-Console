#ifndef SMSC_PROXYSME_PROCESSOR
#define SMSC_PROXYSME_PROCESSOR

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <map>

#include <util/debug.h>
#include <logger/Logger.h>
#include <util/Exception.hpp>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/IntHash.hpp>

#include <util/config/Config.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <core/threads/Thread.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include "ProxySmeException.h"
#include "ProxySmeAdmin.h"

namespace smsc { 
namespace proxysme {
    
  using namespace smsc::core::buffers;
                            
  using smsc::util::Logger;
  using smsc::util::Exception;
  using smsc::util::config::ConfigView;
  using smsc::util::config::ConfigException;

  using smsc::core::threads::Thread;
  using smsc::core::synchronization::Event;
  using smsc::core::synchronization::Mutex;

  class ProxySmeProcessor : public ProxySmeAdmin
  {
  private:
      
      log4cpp::Category   &log;
      
  protected:
        
      int     protocolId;
      char*   svcType;
      
      void init(ConfigView* config);
          //throw(ConfigException);

  public:

      ProxySmeProcessor(ConfigView* config);
          //throw(ConfigException, InitException);
      virtual ~ProxySmeProcessor();
      
      const char* getSvcType() { return (svcType) ? svcType:"ProxySme"; };
      int getProtocolId() { return protocolId; };
      
      /* ------------------------ Smsc interface ------------------------ */ 

      virtual bool processNotification(const std::string msisdn, 
                                       std::string& out);
          //throw (ProcessException);
      virtual void processResponce(const std::string msisdn, 
                                   const std::string msgid, bool responded);
          //throw (ProcessException);

      virtual void processReceipt(const std::string msgid, bool receipted);
          //throw (ProcessException);
      
      /* ------------------------ Admin interface ------------------------ */ 
    
      // nothing
  };

}}

/*extern bool compareMaskAndAddress(const std::string mask, 
                                  const std::string addr);*/

#endif // SMSC_PROXYSME_PROCESSOR


