#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include "system/smsc.hpp"

int main(int argc,char* argv[])
{
  smsc::system::Smsc *app=new smsc::system::Smsc;
  app->init();
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
  return 0;
}
