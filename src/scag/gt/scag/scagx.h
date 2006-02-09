#ifndef SCAG_X_H
#define SCAG_X_H
#include <sys/types.h>

#include "core/threads/ThreadPool.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "scag/transport/smpp/router/route_manager.h"
#include "util/config/smeman/SmeManConfig.h"
#include "scag/performance.hpp"
#include "sme/SmppBase.hpp"
#include "smeman/smsccmd.h"
#include "scag/stat/StatisticsManager.h"
#include "core/buffers/XHash.hpp"
#include "logger/Logger.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/stat/Statistics.h"
#include "scag/stat/StatisticsManager.h"
#include "sms/sms.h"
#include "scag/transport/smpp/SmppManager.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"
#include "util/crc32.h"
#include "scag/util/Reffer.h"

using smsc::sms::SMS;
using smsc::core::threads::ThreadedTask;
using smsc::sme::SmeConfig;

using smsc::smeman::SmscCommand;

using namespace smsc::logger;

using scag::config::RouteConfig;
using scag::stat::SmppStatEvent;
using scag::stat::StatisticsManager;
using scag::transport::smpp::router::RouteManager;
using scag::transport::smpp::router::RouteInfo;
using std::string;
using scag::util::Reffer;
using scag::util::RefferGuard;

namespace scag{
class Scag
{
public:
  Scag():stopFlag(false){};
    ~Scag();
 void init(){};
void run()
  {
	     while(!stopFlag)
	        {
		      sleep(1);
	         }
		 
	   };
     void stop(){stopFlag=true;}

  void abortScag()
  {
    kill(getpid(),9);
  }  ;

  void dumpScag(){  abort();};
  void shutdiwn(){};

     protected:
       bool stopFlag;
			       
       };
   
   }//scag
#endif
 
			       