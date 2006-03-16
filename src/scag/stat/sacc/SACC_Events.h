#ifndef _SACC_Events_H_
#define _SACC_Events_H_

/*****************************************************************************
 *****************************************************************************
 *																			 
 * Module Name:																  
 * ============
 *
 *			SACC_Events.h
 *
 * Purpose:			
 * ========
 *
 *                      Header File for SACC_Events.cpp
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

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif


#include "SACC_Defs.h"


namespace scag{
namespace stat{
namespace sacc{


typedef struct
{
	uint16_t sEventType;
	uint8_t  pAbonentNumber[MAX_ABONENT_NUMBER_LENGTH];	
	uint8_t  cCommandId;		
	uint8_t  cProtocolId;		
	uint16_t sCommandStatus;
	uint64_t lDateTime;
	uint32_t iServiceProviderId;
	uint32_t iServiceId;

}SACC_EVENT_HEADER_t;

typedef struct
{
	SACC_EVENT_HEADER_t Header;
	
	uint32_t iOperatorId;
	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];//512*32
	uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];

}SACC_TRAFFIC_INFO_EVENT_t;

typedef struct
{
	SACC_EVENT_HEADER_t Header;

	uint32_t iOperatorId;
	uint32_t iMediaResourceType;
	uint32_t iPriceCatId;
	float    fBillingSumm; 
	uint8_t  pBillingCurrency[MAX_BILLING_CURRENCY_LENGTH];
			 

}SACC_BILLING_INFO_EVENT_t;

typedef struct
{
	SACC_EVENT_HEADER_t Header;

	//uint16_t sEventType;					
	uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];
	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];

}SACC_OPERATOR_NOT_FOUND_ALARM_t;

typedef struct
{

	SACC_EVENT_HEADER_t Header;

	uint32_t iOperatorId;
	//uint16_t sEventType;					
	uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];
	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];

}SACC_SESSION_EXPIRATION_TIME_ALARM_t;

typedef struct
{
	uint16_t sEventType;
	uint8_t  pAbonentsNumbers[MAX_NUMBERS_TEXT_LENGTH];
	uint16_t pAddressEmail[MAX_EMAIL_ADDRESS_LENGTH];
	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];

}SACC_ALARM_MESSAGE_t;

}//sacc namespace
}//stat namespace 
}//scag namesoace

#endif