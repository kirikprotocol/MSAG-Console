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

using namespace scag::transport::smpp;

SmppChannel* sc;
SmppChannel* sme;

struct FakeRegistrator:SmppChannelRegistrator,SmppCommandQueue{
  int registerSmeChannel(const char* sysId,const char* pwd,SmppBindType bt,SmppChannel* ch)
  {
    printf("register:%s/%s\n",sysId,pwd);
    sme=ch;
    return rarOk;
  }
  int registerSmscChannel(SmppChannel* ch)
  {
    printf("register:%s\n",ch->getSystemId());
    sc=ch;
    return rarOk;
  }
  void unregisterChannel(SmppChannel* ch)
  {
    printf("Unregister:%s\n",ch->getSystemId());
  }
  void putCommand(SmppBindType ct,const SmppCommand& cmd)
  {
    printf("putCommand:%d\n",cmd->get_commandId());
    switch(cmd->get_commandId())
    {
      case SUBMIT:
        sc->putCommand(cmd);
        break;
      case SUBMIT_RESP:
        sme->putCommand(cmd);
        break;
    };
  }
  bool getCommand(SmppCommand& cmd)
  {
    return false;
  }
};


int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  try{
    SmppManager smppMan;
    smppMan.Init("smpp.xml");

    FakeRegistrator fr;
    SmppSocketManager sm(&fr,&fr);
    SmscConnectorAdmin* sca=sm.getSmscConnectorAdmin();

    SmscConnectInfo info;
    info.sysId="sunfire";
    info.hosts[0]="sunfire";
    info.hosts[1]="sunfire";
    info.ports[0]=9001;
    info.ports[1]=9001;
    info.pass="sunfire";

    sca->addSmscConnect(info);

    char buf[32];
    fgets(buf,sizeof(buf),stdin);
    sm.shutdown();

  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
  }

  return 0;
}
