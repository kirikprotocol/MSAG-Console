#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include "system/smsc.hpp"

int main(int argc,char* argv[])
{
  smsc::system::Smsc *app=new smsc::system::Smsc;
  smsc::system::SmscConfigs cfgs;
  smsc::util::config::Manager::init("config.xml");
  cfgs.cfgman=&cfgs.cfgman->getInstance();
  smsc::util::config::smeman::SmeManConfig smemancfg;
  smemancfg.load("sme.xml");
  cfgs.smemanconfig=&smemancfg;

  app->init(cfgs);
  try{
    app->run();
  }catch(std::exception& e)
  {
    fprintf(stderr,"top level exception: %s\n",e.what());
    exit(-1);
  }
  catch(...)
  {
    fprintf(stderr,"FATAL EXCEPTION!\n");
    exit(-0);
  }
  fprintf(stderr,"quiting smsc\n");
  delete app;
  fprintf(stderr,"app deleted\n");
  return 0;
}
