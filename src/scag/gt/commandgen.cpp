// test2.cpp : Defines the entry point for the console application.
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <scag/transport/smpp/SmppCommand.h>

#include "commandgen.h"

using namespace std;
using namespace smsc::sms;
using namespace scag;


#include <sms/sms.h>
#include <scag/transport/SCAGCommand.h>

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


void  String2SCAGCommang(scag::transport::smpp::SmppCommand &cmd, 
       std::string cmname,
       std::string str_oa,
       uint8_t oa_tn,
       uint8_t oa_np,
       std::string str_da,
       uint8_t da_tn,
       uint8_t da_np,
       uint32_t dialogid)
{
 int command_index=-1;
 for(int i=0;i<SMPP_COMMANDS_COUNT;i++)
 {
  if(cmname.compare(SMPP_COMMANDS[i])==0)
  {
   command_index=i;
  }
 }
 SMS sms;
 smsc::sms::Address oa,da ;
 oa.setNumberingPlan(oa_np);
 oa.setTypeOfNumber(oa_tn);

 da.setNumberingPlan(da_np);
 da.setTypeOfNumber(da_tn);


 sms.setDestinationAddress(da);
 sms.setOriginatingAddress(oa);


 switch(command_index) 
 {

 case SMPPCI_deliver_sm:  
  cmd =  scag::transport::smpp::SmppCommand::makeDeliverySm(sms,dialogid);
  break;
 case SMPPCI_deliver_sm_resp: 
  cmd =  scag::transport::smpp::SmppCommand::makeDeliverySmResp("12345",dialogid,1);
  break;
 case SMPPCI_data_sm:   
  //
  break;
 case SMPPCI_data_sm_resp:  
  //
  break;
 case SMPPCI_submit_sm:  
  cmd =  scag::transport::smpp::SmppCommand::makeSubmitSm(sms,dialogid);
  break;
 case SMPPCI_submit_sm_resp: 
 cmd =  scag::transport::smpp::SmppCommand::makeSubmitSmResp("54321",dialogid,1);
  break;
 case SMPPCI_submit_multi:  
  //cmd =  scag::transport::smpp::SmppCommand::makeSubmitMultiResp(sms,dialogid);
  break;
 case SMPPCI_query_sm:   
  break;
 case SMPPCI_query_sm_resp: 
  break;
 default:
  break;
 }
 
  
}

