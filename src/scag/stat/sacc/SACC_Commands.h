#ifndef _SACC_Commands_H_
#define _SACC_Commands_H_

/*****************************************************************************
 *****************************************************************************
 *																			 
 * Module Name:																  
 * ============
 *
 *			SACC_Commands.h
 *
 * Purpose:			
 * ========
 *
 *                      Header File for SACC_Commands.cpp
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

#include "SACC_Defs.h"

#define SEND_TRANSPORT_EVENT 	0x0001
#define SEND_ BILL_EVENT    	0x0002
#define SEND_ALARM_EVENT	0x0003
#define SEND_ALARM_MESSAGE 	0x0101

#define MAX_SEND_BILL_EVT_CURR_LEN 10

typedef struct 
{
	uint16_t iCommandId;
	uint8_t  pAbonentNumber[MAX_ABONENT_NUMBER_LENGTH];
	uint64_t lDateTime;
	uint32_t iOperatorId;
	uint32_t iProviderId;
	uint32_t iServiceId;
	uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];
	uint8_t  cProtocolId;
	uint8_t  cProtocolCommandId;
	uint16_t  cProtocoStatusId;
	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];
	char     cDirection;
	
}SEND_TRANSPORT_EVENT_t;

typedef struct 
{
	uint16_t iCommandId;//command_id		2	0x0001	
	uint8_t  pAbonentNumber[MAX_ABONENT_NUMBER_LENGTH];	//uint8_t pAbonentAddrabonent_addr	1	(max 25)	Строка ASCII (массив байтов)	Адрес абонента в формате.[TON].[NPI].[ADRESS]
	uint64_t lDateTime;;//timestamp	2	8	8 байтное целое число (long)	Время отправки сообщения от платформы к сервису или получения платформой от сервиса. Время в UTC, количество миллисекунд с начала Эпохи. 
	uint32_t iOperatorId;//operator_id	3	4	4 байтное целое число	Id оператора
	uint32_t iProviderId;//provider_id	4	4	4 байтное целое число	Id сервис провайдера
	uint32_t  iServiceId;;//service_id	5	4	4 байтное целое число	Id сервиса
	uint8_t   pSessionKey[MAX_SESSION_KEY_LENGTH];//session_id	6	(max 45)	Строка ASCII (массив байтов)	Уникальный ключ сессии.
	uint8_t   cProtocolId;//protocol_id	7	1	1 байтное целое число	Протокол 1-smpp/sms2-smpp/ussd3-http4-mms
	uint8_t   cProtocolCommandId;//protocol_command_id	8	1	1 байтное целое число	Команда протокола
	uint16_t  cProtocoStatusId;//protocol_command_status	9	2	2 байтное целое число	Статус команды протокола

	uint32_t iMediaType;//media_type;//	10	4	4 байтное целое число
	uint32_t iCategoryId;//category_id	11	4	4 байтное целое число
	float_t  fBillVaue;//bill_value	12	4	4 байтное вещественное число (ANSI/IEEE 754-1985)
	uint8_t pBillingCureency[MAX_SEND_BILL_EVT_CURR_LEN];//currency	13	(max 10)	Строка ASCII (массив байтов)

}SEND_ BILL_EVENT_t;

typedef struct 
{

	uint16_t iCommandId;
	uint8_t  pAbonentNumber[MAX_ABONENT_NUMBER_LENGTH];
	uint64_t lDateTime;
	uint32_t iOperatorId;
	uint32_t iProviderId;
	uint32_t iServiceId;
	uint8_t  pSessionKey[MAX_SESSION_KEY_LENGTH];
	uint8_t  cProtocolId;
	uint8_t  cProtocolCommandId;
	uint16_t  cProtocoStatusId;

	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];//message	10	(max 1024)	Строка UTF-16 Big Endian (массив байтов, одному символу соответствует 2 байта, Byte Order Marker не проставляется)
	char     cDirection;//direction	11	1	Символ, "I" или "O"
	uint32_t iEventId;//event_id	12	4	4 байтное целое число

}SEND_ALARM_EVENT_t;

typedef struct 
{

}SEND_ALARM_MESSAGE_t;

}//scag
}//stat
}//sacc