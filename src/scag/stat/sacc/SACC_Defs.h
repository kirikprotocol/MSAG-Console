#ifndef _SACC_Defs_H_
#define _SACC_Defs_H_

/*****************************************************************************
 *****************************************************************************
 *																			 
 * Module Name:																  
 * ============
 *
 *			SACC_Defs.h
 *
 * Purpose:			
 * ========
 *
 *	 		Global settings  header file 
 *			
 * Author(s) & Creation Date:	
 * ==========================
 *
 *			Gregory Panin, 2006/03/03
 *  Copyright (c): 
 *  =============
 * 			EyeLine Communications
 * 			All rights reserved.
 */

namespace scag{
namespace stat{
namespace sacc{

#define MAX_ABONENT_NUMBER_LENGTH 	25
#define MAX_SESSION_KEY_LENGTH 		45
#define MAX_TEXT_MESSAGE_LENGTH 	1024
#define MAX_BILLING_CURRENCY_LENGTH	20
#define MAX_NUMBERS_TEXT_LENGTH 	MAX_TEXT_MESSAGE_LENGTH
#define MAX_EMAIL_ADDRESS_LENGTH	MAX_TEXT_MESSAGE_LENGTH

#define SACC_SEND_TRANSPORT_EVENT 		0x0001
#define SACC_SEND_BILL_EVENT      		0x0002	
#define SACC_SEND_ALARM_EVENT	  		0x0003
#define SACC_SEND_OPERATOR_NOT_FOUND_ALARM  	0x0004
#define SACC_SEND_SESSION_EXPIRATION_TIME_ALARM 0x0005

#define SACC_SEND_ALARM_MESSAGE	  		0x0101
#define EV_SERVER_LOGGER_CATHEGORY_NAME  "evtsvr"

	
}//sacc
}//stat
}//scag

#endif