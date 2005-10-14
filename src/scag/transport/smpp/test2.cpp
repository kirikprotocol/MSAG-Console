#include <stdio.h>
#include "SmeAcceptor.h"
#include "SmeSocket.h"
#include "SmppChannel.h"
#include "SmppChannelRegistrator.h"
#include "SmppCommand.h"
#include "SmppCommandQueue.h"
#include "SmppEntity.h"
#include "SmppIOBase.h"
#include "SmppManager.h"
#include "SmppProxy.h"
#include "SmppReader.h"
#include "SmppSMInterface.h"
#include "SmppSocket.h"
#include "SmppSocketManager.h"
#include "SmppTypes.h"
#include "SmppWriter.h"
#include "SmscConnector.h"
#include "SmscSocket.h"
#include "scag/config/ConfigManager.h"
#include "scag/re/RuleEngine.h"
#include "scag/sessions/SessionManager.h"

using namespace scag::transport::smpp;
int main(int argc,char* argv[])
{
  delete new char;
  try{
    smsc::logger::Logger::Init();
    scag::config::ConfigManager::Init();
    scag::re::RuleEngine::Init(".");
    scag::sessions::SessionManager::Init(scag::config::ConfigManager::Instance().getSessionManConfig());
  }catch(std::exception& e)
  {
    printf("init exception:%s\n",e.what());
    return -1;
  }
  try{
    SmppManager sm;
    sm.Init("smpp.xml");
    sm.LoadRoutes("routes.xml");
    printf("press enter...");
    char buf[32];
    fgets(buf,sizeof(buf),stdin);
    sm.StopProcessing();
  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
  }
  return 0;
}
