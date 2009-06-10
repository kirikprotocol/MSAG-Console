#ifndef _SACC_Defs_H_
#define _SACC_Defs_H_

/*****************************************************************************
 *****************************************************************************
 *                                                                           
 * Module Name:                                                               
 * ============
 *
 *          SACC_Defs.h
 *
 * Purpose:         
 * ========
 *
 *          Global settings  header file 
 *          
 * Author(s) & Creation Date:   
 * ==========================
 *
 *          Gregory Panin, 2006/03/03
 *  Copyright (c): 
 *  =============
 *          EyeLine Communications
 *          All rights reserved.
 */

namespace scag{
namespace stat{
namespace sacc{

#define MAX_ABONENT_NUMBER_LENGTH   25
#define MAX_SESSION_KEY_LENGTH      45
#define MAX_TEXT_MESSAGE_LENGTH     1024
#define MAX_BILLING_CURRENCY_LENGTH 20
#define MAX_BILLING_SUMM_LENGTH     40
#define MAX_NUMBERS_TEXT_LENGTH     1024
#define MAX_EMAIL_ADDRESS_LENGTH    1024
#define DELEVIRY_TIME_LENGTH        16
#define PACKET_TYPE_LENGTH          50
#define MAX_KEYWORDS_TEXT_LENGTH    1024

#define SACC_SEND_TRANSPORT_EVENT       0x0001
#define SACC_SEND_BILL_EVENT            0x0002  
#define SACC_SEND_ALARM_EVENT           0x0003
#define SACC_SEND_OPERATOR_NOT_FOUND_ALARM      0x0004
#define SACC_SEND_SESSION_EXPIRATION_TIME_ALARM 0x0005

#define SACC_SEND_ALARM_MESSAGE         0x0101
#define EV_SERVER_LOGGER_CATHEGORY_NAME  "evtsvr"

typedef enum
{
    sec_transport = 0x0001,
    sec_bill = 0x0002,  
    sec_alarm= 0x0003,
    sec_operator_not_found = 0x0004,
    sec_session_expired= 0x0005,
    sec_alarm_message = 0x0101

}SaccEventsCommandIds;

typedef enum
{
    pci_smppSubmitSm=1,
    pci_smppSubmitSmResp,
    pci_smppDeliverSm,
    pci_smppDeliverSmResp,
    pci_smppReceived,
    
    pci_httpRequest,
    pci_httpResponse,
    pci_httpDeliver

}SaccProtocolCommandIds;

typedef enum
{
    pid_smpp_sms=1,
    pid_smpp_ussd,
    pid_http,
    pid_mms
}SaccProtocolIds;

typedef enum
{
    bci_open=1,
    bci_commit,
    bci_rollback,
    bci_rollback_by_timeout

}SaccBillCommandIds;

typedef enum
{
    eid_operatorNotFound=1,
    eid_routeNotFound=2,
    eid_sessionExpared=3
    
}SaccEventsIds;

typedef enum
{
    scl_info=0,
    scl_notify=1,
    scl_warn=2,
    scl_alarm=3
}SaccCriticalityLevels;
/**/
    

}//sacc
}//stat
}//scag

#endif
