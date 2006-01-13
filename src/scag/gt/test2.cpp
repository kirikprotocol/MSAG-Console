// test2.cpp : Defines the entry point for the console application.
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef WIN32
 #define snprintf _snprintf
#endif

using namespace std;

#include <sms/sms.h>
#include <scag/transport/SCAGCommand.h>

#define SMPP_COMMANDS_COUNT 9


const char* SMPP_COMMANDS[SMPP_COMMANDS_COUNT]={
"deliver_sm",
"deliver_sm_resp",
"data_sm",  
"data_sm_resp",

"submit_sm",
"submit_sm_resp",
"submit_multi",

"query_sm",
"query_sm_resp"};

#define SMPPCI_deliver_sm   0
#define SMPPCI_deliver_sm_resp  1
#define SMPPCI_data_sm    2
#define SMPPCI_data_sm_resp   3
#define SMPPCI_submit_sm   4
#define SMPPCI_submit_sm_resp  5
#define SMPPCI_submit_multi   6
#define SMPPCI_query_sm    7
#define SMPPCI_query_sm_resp  8


void String2SCAGCommang(std::string cmname,std::string str_oa,std::string str_da)
{
 int command_index=-1;
 for(int i=0;i<SMPP_COMMANDS_COUNT;i++)
 {
  if(cmname.compare(SMPP_COMMANDS[i])==0)
  {
   command_index=i;
   printf("YES compared with '%s'\n",SMPP_COMMANDS[i]);
  }
 }

 switch(command_index) 
 {

 case SMPPCI_deliver_sm:  
  break;
 case SMPPCI_deliver_sm_resp: 
  break;
 case SMPPCI_data_sm:   
  break;
 case SMPPCI_data_sm_resp:  
  break;
 case SMPPCI_submit_sm:  
  break;
 case SMPPCI_submit_sm_resp: 
  break;
 case SMPPCI_submit_multi:  
  break;
 case SMPPCI_query_sm:   
  break;
 case SMPPCI_query_sm_resp: 
  break;
 default:
  break;
 }
 
}

int main(int argc, char* argv[])
{
 String2SCAGCommang("data_sm_resp","g","g");
 return 0;
}
