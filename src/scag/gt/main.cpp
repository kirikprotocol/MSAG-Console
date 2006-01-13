/*
    SibInfoConsalting
    Rule Engine test application
    created by Green: green@sibinco.ru
    
*/

#include "renscript.hpp"
#include <core/threads/Thread.hpp>
#include <thread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <logger/Logger.h>


using namespace std;
using namespace smsc::core::threads;

int going = 1; 
int cpflag;

bool stopProcess=false;

TestRunner ppr;

class commandChecker:public Thread
{

int Execute()
{
   char buff[256];
   while(1) 
   {
    
	printf( "Enter q to quit: " );
    
	fflush( stdout );
    
	fgets( buff, 255, stdin );
    
	printf( "Got command: %s\n", buff );
    
	if( buff[strlen(buff)-1]=='\n' ) 
		    buff[strlen(buff)-1] = 0;

	if( !strcmp( buff, "q" ) ) 
	{
    	    kill( getpid(), SIGINT );
    	    stopProcess=true;
    	    return 1;
	}
    }  
}

};

commandChecker cc;


void stop(int)
{
 stopProcess=true;
}

int main(int argc, char *argv[])
{

     sigset(SIGINT,stop);
     //init();
 
    // start command key thread
    cc.Start();

 if(argc>1)
 {
  ppr.scriptfilename = argv[1];
  ppr.Start();
 }

      cc.WaitFor();
     ppr.WaitFor();

  //smsc::logger::Logger::Shutdown();

 return 0;
}