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


struct SACC_EVENT_HEADER_t
{
	uint16_t sEventType;
	uint8_t  pAbonentNumber[MAX_ABONENT_NUMBER_LENGTH];	
	uint8_t  cCommandId;		
	uint8_t  cProtocolId;		
	uint16_t sCommandStatus;
	uint64_t lDateTime;
	uint32_t iServiceProviderId;
	uint32_t iServiceId;

};

struct SACC_TRAFFIC_INFO_EVENT_t
{
	SACC_EVENT_HEADER_t Header;
	
	uint32_t iOperatorId;
	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];//512*32
	uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];

	SACC_TRAFFIC_INFO_EVENT_t()
	{
		memset(&Header,0,sizeof(SACC_EVENT_HEADER_t));
		memset(pMessageText,0,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
		memset(pSessionKey,0,MAX_SESSION_KEY_LENGTH);
		iOperatorId=0;
	};

	SACC_TRAFFIC_INFO_EVENT_t(const SACC_TRAFFIC_INFO_EVENT_t & src)
	{
		memcpy(&Header,src.Header,sizeof(SACC_EVENT_HEADER_t));
		memcpy(pMessageText,src.pMessageText,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
		memcpy(pSessionKey,src.pSessionKey,MAX_SESSION_KEY_LENGTH);
		iOperatorId=src.iOperatorId;
	}

};

struct SACC_BILLING_INFO_EVENT_t
{
	SACC_EVENT_HEADER_t Header;

	uint32_t iOperatorId;
	uint32_t iMediaResourceType;
	uint32_t iPriceCatId;
	float    fBillingSumm; 
	uint8_t  pBillingCurrency[MAX_BILLING_CURRENCY_LENGTH];
			 
	SACC_BILLING_INFO_EVENT_t()
	{
		memset(&Header,0,sizeof(SACC_EVENT_HEADER_t));
		memset(pBillingCurrency,0,MAX_BILLING_CURRENCY_LENGTH);
		iOperatorId=0;
		iMediaResourceType=0;
		iPriceCatId=0;
		fBillingSumm=0; 
	}	
	SACC_BILLING_INFO_EVENT_t(const SACC_BILLING_INFO_EVENT_t & src)
	{
		memcpy(&Header,src.Header,sizeof(SACC_EVENT_HEADER_t));
		memcpy(pBillingCurrency,src.pBillingCurrency,MAX_BILLING_CURRENCY_LENGTH);
		iOperatorId=src.iOperatorId ;
		iMediaResourceType=src.iMediaResourceType;
		iPriceCatId=src.iPriceCatId;
		fBillingSumm=src.fBillingSumm; 
	}	
};

struct SACC_OPERATOR_NOT_FOUND_ALARM_t
{
	SACC_EVENT_HEADER_t Header;

	//uint16_t sEventType;					
	uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];
	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];

	SACC_OPERATOR_NOT_FOUND_ALARM_t()
	{
		memset(&Header,0,sizeof(SACC_EVENT_HEADER_t));
		memset(pSessionKey,0,MAX_SESSION_KEY_LENGTH);
		memset(pMessageText,0,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));

	}

	SACC_OPERATOR_NOT_FOUND_ALARM_t(const SACC_OPERATOR_NOT_FOUND_ALARM_t& src)
	{
		memcpy(&Header, src.Header,sizeof(SACC_EVENT_HEADER_t));
		memcpy(pSessionKey,src.pSessionKey ,MAX_SESSION_KEY_LENGTH);
		memcpy(pMessageText,src.pMessageText ,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));

	}
};

struct SACC_SESSION_EXPIRATION_TIME_ALARM_t
{

	SACC_EVENT_HEADER_t Header;

	uint32_t iOperatorId;
	//uint16_t sEventType;					
	uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];
	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];
	SACC_SESSION_EXPIRATION_TIME_ALARM_t()
	{
		memset(&Header,0,sizeof(SACC_EVENT_HEADER_t));
		memset(pSessionKey,0,MAX_SESSION_KEY_LENGTH);
		memset(pMessageText,0,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
		iOperatorId=0;
	}
	SACC_SESSION_EXPIRATION_TIME_ALARM_t(const SACC_SESSION_EXPIRATION_TIME_ALARM_t& src)
	{
		memcpy(&Header, src.Header,sizeof(SACC_EVENT_HEADER_t));
		memcpy(pSessionKey,src.pSessionKey ,MAX_SESSION_KEY_LENGTH);
		memcpy(pMessageText,src.pMessageText ,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
		iOperatorId=src.iOperatorId;
	}

};

struct SACC_ALARM_MESSAGE_t
{
	uint16_t sEventType;
	uint8_t  pAbonentsNumbers[MAX_NUMBERS_TEXT_LENGTH];
	uint16_t pAddressEmail[MAX_EMAIL_ADDRESS_LENGTH];
	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];

	SACC_ALARM_MESSAGE_t()
	{
		memset(pAbonentsNumbers,0,MAX_NUMBERS_TEXT_LENGTH;);
		memset(pAddressEmail,0,MAX_EMAIL_ADDRESS_LENGTH*sizeof(uint16_t));
		memset(pMessageText,0,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
		sEventType=0;
	}
	SACC_ALARM_MESSAGE_t(const SACC_ALARM_MESSAGE_t & src)
	{
		memset(pAbonentsNumbers,src.pAbonentsNumbers ,MAX_NUMBERS_TEXT_LENGTH;);
		memset(pAddressEmail,src.pAddressEmail ,MAX_EMAIL_ADDRESS_LENGTH*sizeof(uint16_t));
		memset(pMessageText,src.pMessageText ,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
		sEventType=src.sEventType;

	}

};

}//sacc namespace
}//stat namespace 
}//scag namesoace

#endif