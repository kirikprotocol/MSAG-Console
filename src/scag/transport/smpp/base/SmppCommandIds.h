#ifndef _SCAG_TRANSPORT_SMPP_SMPPCOMMANDIDS_H
#define _SCAG_TRANSPORT_SMPP_SMPPCOMMANDIDS_H

namespace scag2 {
namespace transport {
namespace smpp {

typedef enum CommandId
{
    UNKNOWN,                //0
    DELIVERY,               //1
    DELIVERY_RESP,          //2
    SUBMIT,                 //3
    SUBMIT_RESP,            //4
    FORWARD,                //5
    GENERIC_NACK,           //6
    QUERY,                  //7
    QUERY_RESP,             //8
    UNBIND,                 //9
    UNBIND_RESP,            //10
    REPLACE,                //11
    REPLACE_RESP,           //12
    CANCEL,                 //13
    CANCEL_RESP,            //14
    ENQUIRELINK,            //15
    ENQUIRELINK_RESP,       //16
    SUBMIT_MULTI_SM,        //17
    SUBMIT_MULTI_SM_RESP,   //18
    BIND_TRANSCEIVER,       //19
    BIND_RECIEVER_RESP,     //20
    BIND_TRANSMITTER_RESP,  //21
    BIND_TRANCIEVER_RESP,   //22
    DATASM,                 //23
    DATASM_RESP,            //24
    PROCESSEXPIREDRESP,     //25
    ALERT_NOTIFICATION      //26
} CommandId;


/*
enum CommandStatus{
  CMD_OK=0,
  CMD_ERR_TEMP=1,
  CMD_ERR_PERM=2,
  CMD_ERR_FATAL=3,
  CMD_ERR_RESCHEDULENOW=4
};
*/

enum DataSmDirection {
    dsdUnknown = 1,
    dsdSrv2Srv,
    dsdSrv2Sc,
    dsdSc2Srv,
    dsdSc2Sc
};

} //smpp
} //transport
} //scag

#endif /* !_SCAG_TRANSPORT_SMPP_SMPPCOMMANDIDS_H */
