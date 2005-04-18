#include <stdio.h>
#include <exception>
#include "logger/Logger.h"
#include "BillingRules.hpp"
#include "sms/sms.h"
#include "util/xml/init.h"

using namespace smsc::smppgw::billing::rules;

using namespace smsc::sms;

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();

  smsc::util::regexp::RegExp::InitLocale();

  try{
    BillingRulesManager::Init("billing-rules.xml");
  }catch(std::exception& e)
  {
    printf("BRM::Init exception %s",e.what());
    return -1;
  }

  SMS s;

  s.setOriginatingAddress(".0.1.1");
  s.setDestinationAddress(".0.1.2");
  s.setSourceSmeId("srcsme");
  s.setDestinationSmeId("dstsme");
  s.setRouteId("routeId");
  s.setServiceId(1);
  s.setIntProperty(Tag::SMSC_PROVIDERID,2);

  try{

    if(BillingRulesManager::getInstance()->getBillingRule("rule1").queryBill(s))
    {
      printf("Billed. serviceId=%d\n",s.getServiceId());
    }else
    {
      printf("Not billed\n");
    }
  }catch(std::exception& e)
  {
    printf("query failed:%s",e.what());
  }

  BillingRulesManager::Uninit();
  smsc::util::xml::TerminateXerces();
  smsc::logger::Logger::Shutdown();
  return 0;
}
