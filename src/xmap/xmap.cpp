#include "xmap.h"
#include "common.h"
//#include "callProcessor.h"
#include "MapStateMashine.h"
//#include "queue.h"
//#include "mmqueue.h"
//#include "dinfo.h"
#include "piper.hpp"
#include "syncque.hpp"
#include "smet.hpp"
#include "../src/core/threads/Thread.hpp"


using namespace std;

using namespace smsc::core::threads;
#include "config.h"

int going = 1; 
int cpflag;
bool stopProcess=false;
smsc::logger::Logger *logger;

//CallProcessor * cprocessor;
CMapStateMashine * mapSTM;
CPiper ppr;
UssdSmeRunner SMEThr;

ConfigXml config;
ostream & operator << (ostream & out, const DOMElement & elem);

qMessage_t message_store;

MapDialogContainer * g_mdc;

class commandChecker:public Thread
{

public:
 std::string strold;



 void trim(std::string & str)
 {
  int p=str.find(' ');
  while(p==0)
  {
   str.erase(p,1);
   p=str.find(' ');
  }
 }
 std::string  token(std::string  str, const char* delimeter,int num)
 {

  std::string retval="";
  int index=0;
  int p=0;
  while(index<=num)
  {
   trim(str);
   p=str.find(delimeter);

   if(p!=-1)
   {
    char pd[256];
    memset(pd,0,256);
    str.copy(pd ,p);
    retval = pd;
    str.erase(0,p);
   }
   else
   {
    retval = str;
   }
   index++;
  };


  return retval;
 };

  
  
 
 int Execute()
 {
  strold="";
  
    
        

   char buff[256];
   while(1) 
   {

    //message_store.map_statemachine_event.Wait(GLOBAL_SWITCHERS_TIMEOUT);
    
    printf( "Enter q to quit: " );
    fflush( stdout );
    fgets( buff, 255, stdin );
    printf( "Got command: %s\n", buff );
    if( buff[strlen(buff)-1]=='\n' ) buff[strlen(buff)-1] = 0;

    if( !strcmp( buff, "q" ) ) 
    {
     kill( getpid(), SIGINT );
     stopProcess=true;
     return 1;
    }
    else
    {
     if( !strcmp( buff, "help" ) ) 
     {
      printf("\n");
      printf("xmap [autorunned javascript]\n");
      printf("commands:\n");
      printf("   q    - quit\n");
      printf("   start     - run script (not implemented)\n");
 
     }
     if( !strcmp( buff, "start" ) ) 
     {
      ppr.Start();

     }
      
      
    }

    
    
   }

  
  

  }
  
};

commandChecker cc;

int init()
{
 Logger::Init();
 logger =  Logger::getInstance("micromap");
 
 g_mdc = new MapDialogContainer();
 
 message_store.bindOK=0;
    message_store.config = new ConfigXml();

 if(!message_store.config->readXMLConfig("xmap.xml"))
  return 0;

// cprocessor  = new CallProcessor(&message_store);
/// cprocessor->connect();

 mapSTM = new CMapStateMashine(&message_store);
 mapSTM->SetDialogContainer(g_mdc);
 mapSTM->connect();

 
 return 1;
 
}
void stop(int)
{
 stopProcess=true;
}
int main(int argc, char *argv[])
{

 sigset(SIGINT,stop);
 if(!init())
  return 0;

  
 message_store.map_statemachine_event.Signal();
 message_store.call_processor_event.Signal();
 
 cc.Start();
// cprocessor->Start();
 mapSTM->Start();
 SMEThr.Start();

 if(argc>1)
 {

  ppr.scriptfilename = argv[1];
  // start script interpreter
  ppr.Start();
 }

// cprocessor->WaitFor(); 
 mapSTM->WaitFor();
 SMEThr.WaitFor();
      cc.WaitFor();
     ppr.WaitFor();

 delete mapSTM;

 delete g_mdc;

// delete cprocessor;
// if(message_store.config )
//  delete message_store.config;

 
// delete message_store.pDialogs;

 return 0;
}