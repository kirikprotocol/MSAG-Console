#include <stdio.h>
#include "ClosedGroupsManager.hpp"
#include "logger/Logger.h"

using namespace smsc::closedgroups;

void Check(ClosedGroupsInterface* cgm,const char* addr)
{
  printf("%s:%s\n",addr,cgm->Check(1,addr)?"true":"false");
}

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  try{
    ClosedGroupsManager::Init();
    ClosedGroupsInterface* cgm=ClosedGroupsManager::getInstance();
    cgm->Load("ClosedGroups.xml");
    Check(cgm,"+79131111111");
    Check(cgm,"+79131111112");
    Check(cgm,"+79132222222");
    Check(cgm,"+79132222223");
    Check(cgm,"+79142222223");
    Check(cgm,"+7914222222");
    Check(cgm,"+791422222222");
  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
  }
  ClosedGroupsManager::Shutdown();
  return 0;
}
