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
#include "SACC_Defs.h"


namespace scag{
namespace stat{
namespace sacc{


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
	uint8_t  pAbonentNumber[MAX_ABONENT_NUMBER_LENGTH];	//uint8_t pAbonentAddrabonent_addr	1	(max 25)	��ப� ASCII (���ᨢ ���⮢)	���� ������� � �ଠ�.[TON].[NPI].[ADRESS]
	uint64_t lDateTime;;//timestamp	2	8	8 ���⭮� 楫�� �᫮ (long)	�६� ��ࠢ�� ᮮ�饭�� �� ������� � �ࢨ�� ��� ����祭�� �����ମ� �� �ࢨ�. �६� � UTC, ������⢮ �����ᥪ㭤 � ��砫� ����. 
	uint32_t iOperatorId;//operator_id	3	4	4 ���⭮� 楫�� �᫮	Id ������
	uint32_t iProviderId;//provider_id	4	4	4 ���⭮� 楫�� �᫮	Id �ࢨ� �஢�����
	uint32_t  iServiceId;;//service_id	5	4	4 ���⭮� 楫�� �᫮	Id �ࢨ�
	uint8_t   pSessionKey[MAX_SESSION_KEY_LENGTH];//session_id	6	(max 45)	��ப� ASCII (���ᨢ ���⮢)	�������� ���� ��ᨨ.
	uint8_t   cProtocolId;//protocol_id	7	1	1 ���⭮� 楫�� �᫮	��⮪�� 1-smpp/sms2-smpp/ussd3-http4-mms
	uint8_t   cProtocolCommandId;//protocol_command_id	8	1	1 ���⭮� 楫�� �᫮	������� ��⮪���
	uint16_t  cProtocoStatusId;//protocol_command_status	9	2	2 ���⭮� 楫�� �᫮	����� ������� ��⮪���

	uint32_t iMediaType;//media_type;//	10	4	4 ���⭮� 楫�� �᫮
	uint32_t iCategoryId;//category_id	11	4	4 ���⭮� 楫�� �᫮
	float_t  fBillVaue;//bill_value	12	4	4 ���⭮� ����⢥���� �᫮ (ANSI/IEEE 754-1985)
	uint8_t pBillingCureency[MAX_SEND_BILL_EVT_CURR_LEN];//currency	13	(max 10)	��ப� ASCII (���ᨢ ���⮢)

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

	uint16_t pMessageText[MAX_TEXT_MESSAGE_LENGTH];//message	10	(max 1024)	��ப� UTF-16 Big Endian (���ᨢ ���⮢, ������ ᨬ���� ᮮ⢥����� 2 ����, Byte Order Marker �� ���⠢�����)
	char     cDirection;//direction	11	1	������, "I" ��� "O"
	uint32_t iEventId;//event_id	12	4	4 ���⭮� 楫�� �᫮

}SEND_ALARM_EVENT_t;

typedef struct 
{

}SEND_ALARM_MESSAGE_t;

}//scag
}//stat
}//sacc