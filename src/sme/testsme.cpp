#include <stdio.h>
#include "sme/sme.hpp"
#include "util/Exception.hpp"
#include <exception>

class TestSme:public smsc::sme::BaseSme{
public:
  TestSme(const char *host,int port,const char *sysid)
  :BaseSme(host,port,sysid){}
  bool processSms(smsc::sms::SMS *sms){}
};

int main(int argc,char* argv[])
{
  TestSme sme("smsc",9876,"1");
  try{
    if(!sme.init())throw smsc::util::Exception("connect failed!");
    sme.bindsme();
  }catch(std::exception& e)
  {
    printf("EX:%s\n",e.what());
  }
  return 0;
}
