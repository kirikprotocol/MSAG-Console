#include <stdio.h>
#include <signal.h>
#include <string>

#include <util/Exception.hpp>
#include "BillingServer.h"


using namespace inmanemu::server;

BillingServer * billingServer = 0;



extern "C" void interupt(int sig)
{
    if (billingServer) billingServer->Stop();
}

int main(int argc,char* argv[])
{
  sigset(SIGINT,interupt);
  smsc::logger::Logger::Init();

  billingServer = new BillingServer();
  if (!billingServer) return 0;

  try
  {
      billingServer->Init("localhost", 10021);
  } catch (Exception& e)
  {
      printf("%s\n", e.what());
      
      delete billingServer;
      return 0;
  }
  

  billingServer->Run();


  delete billingServer;
  return 0;
}
