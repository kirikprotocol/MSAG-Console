#include "util/config/Manager.h"
#include "util/debug.h"
#include "MessageStoreTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "store/StoreManager.h"
#include "store/StoreExceptions.h"
#include <cstdlib>
#include <ctime>
#include <sstream>

namespace smsc  {
namespace test  {
namespace store {

using namespace std;
using namespace smsc::test::util; //auto_sync, etc
using namespace smsc::test::sms; //constants, SmsUtil
using namespace smsc::store; //StoreManager, CreateMode
using namespace smsc::sms; //constants, SMSId, SMS, AddressValue, EService, ...
using log4cpp::Category;
using smsc::util::Logger;
using smsc::util::AssertException;

MessageStoreTestCases::MessageStoreTestCases(MessageStore* _msgStore,
	bool _loadTest, CheckList* _chkList) : msgStore(_msgStore), chkList(_chkList),
	loadTest(_loadTest), mask(0xffffffffffffffff), check(true)
{
	__require__(msgStore);
	//__require__(chkList);
	if (loadTest)
	{
		mask ^= 0x100000; //��������� ������ �� SMPP_MESSAGE_PAYLOAD
		check = false; //��������� �������� � �������� � ��������
	}
}

Category& MessageStoreTestCases::getLog()
{
	static Category& log = Logger::getCategory("MessageStoreTestCases");
	return log;
}

const char* str(SMSId id)
{
	ostringstream s;
	s << id;
	return s.str().c_str();
}

#define __set_str__(field, len) \
	auto_ptr<char> tmp_##field = rand_char(len); \
	p.set##field(tmp_##field.get()); \
	if (check) { \
		char res_##field[len + 1]; \
		p.get##field(res_##field); \
		__require__(!strcmp(res_##field, tmp_##field.get())); \
	}

#define __set_int_body_tag__(tagName, value) \
	uint32_t tmp_##tagName = value; \
	__trace__("set_int_body_tag: " #tagName); \
	p.setIntProperty(Tag::tagName, tmp_##tagName); \
	if (check) { \
		__require__(p.getIntProperty(Tag::tagName) == tmp_##tagName); \
	}

#define __set_str_body_tag__(tagName, length) \
	int len_##tagName = length; \
	auto_ptr<char> tmp_##tagName = rand_char(len_##tagName); \
	__trace__("set_str_body_tag: " #tagName); \
	p.setStrProperty(Tag::tagName, tmp_##tagName.get()); \
	if (check) { \
		__require__(p.getStrProperty(Tag::tagName) == tmp_##tagName.get()); \
	}

#define __set_bin_body_tag__(tagName, length) \
	int len_##tagName = length; \
	auto_ptr<uint8_t> tmp_##tagName = rand_uint8_t(len_##tagName); \
	__trace__("set_bin_body_tag: " #tagName); \
	p.setBinProperty(Tag::tagName, (char*) tmp_##tagName.get(), len_##tagName); \
	if (check) { \
		unsigned len; \
		const char* val = p.getBinProperty(Tag::tagName, &len); \
		__require__(len == len_##tagName && !memcmp(val, tmp_##tagName.get(), len)); \
	}

#define __set_bin_body_tag2__(tagName, length, value) \
	int len_##tagName = length; \
	char* val_##tagName  = value; \
	__trace__("set_bin_body_tag: " #tagName); \
	p.setBinProperty(Tag::tagName, val_##tagName, len_##tagName); \
	if (check) { \
		unsigned len; \
		const char* val = p.getBinProperty(Tag::tagName, &len); \
		__require__(len == len_##tagName && !memcmp(val, val_##tagName, len)); \
	}

void MessageStoreTestCases::storeCorrectSms(SMSId* idp, SMS* smsp, int num)
{
	TCSelector s(num, 15);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			SMS sms;
			SMS& p = sms;
			SmsUtil::setupRandomCorrectSms(&sms, mask, check);
			switch(s.value())
			{
				case 1: //������ ����������
					__tc__("storeCorrectSms");
					break;
				case 2: //originatingAddress ����������� �����
					//�������� SMPP v3.4 ������ ���� 0, �� �� ������ 1
					__tc__("storeCorrectSms.origAddrMarginalLength");
					sms.setOriginatingAddress(1, 10, 20, "*");
					break;
				case 3: //originatingAddress ������������ �����
					{
						__tc__("storeCorrectSms.origAddrMarginalLength");
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setOriginatingAddress(MAX_ADDRESS_LENGTH, 20, 30,
							addr.get());
					}
					break;
				case 4: //destinationAddress ����������� �����
					__tc__("storeCorrectSms.destAddrMarginalLength");
					sms.setDestinationAddress(1, 30, 40, "*");
					break;
				case 5: //destinationAddress ������������ �����
					{
						__tc__("storeCorrectSms.destAddrMarginalLength");
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setDestinationAddress(MAX_ADDRESS_LENGTH, 40, 50, 
							addr.get());
					}
					break;
				case 6: //dealiasedDestinationAddress ����������� �����
					__tc__("storeCorrectSms.dealiasedDestAddrMarginalLength");
					sms.setDealiasedDestinationAddress(1, 30, 40, "*");
					break;
				case 7: //dealiasedDestinationAddress ������������ �����
					{
						__tc__("storeCorrectSms.dealiasedDestAddrMarginalLength");
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setDealiasedDestinationAddress(MAX_ADDRESS_LENGTH,
							40, 50, addr.get());
					}
					break;
				case 8: //������ imsi (��. GSM 09.02 ����� 12.2) � ������ msc (???)
					__tc__("storeCorrectSms.origDescMarginal");
					sms.setOriginatingDescriptor(0, NULL, 0, NULL, 10);
					break;
				case 9: //������ imsi (��. GSM 09.02 ����� 12.2) � ������ msc (???)
					__tc__("storeCorrectSms.origDescMarginal");
					sms.setOriginatingDescriptor(0, "*", 0, "*", 20);
					break;
				case 10: //imsi � msc ������ ������������ �����
					{
						__tc__("storeCorrectSms.origDescMarginal");
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						sms.setOriginatingDescriptor(
							MAX_ADDRESS_LENGTH, mscAddr.get(),
							MAX_ADDRESS_LENGTH, imsiAddr.get(), 30);
					}
					break;
				case 11: //������ ���� ���������
					__tc__("storeCorrectSms.bodyEmpty");
					sms.getMessageBody() = Body();
					break;
				case 12: //short_message � message_payload ������������ �����
					{
						if (loadTest)
						{
							__tc__("storeCorrectSms");
						}
						else
						{
							__tc__("storeCorrectSms.bodyMaxLength");
							__set_int_body_tag__(SMPP_SM_LENGTH, MAX_SM_LENGTH);
							__set_bin_body_tag__(SMPP_SHORT_MESSAGE, MAX_SM_LENGTH);
							__set_bin_body_tag__(SMPP_MESSAGE_PAYLOAD, MAX_PAYLOAD_LENGTH);
						}
					}
					break;
				case 13: //������ serviceType, NULL �����������
					__tc__("storeCorrectSms.serviceTypeMarginal");
                   	sms.setEServiceType("");
					break;
				case 14: //serviceType ������������ �����
					{
						__tc__("storeCorrectSms.serviceTypeMarginal");
						auto_ptr<char> type = rand_char(MAX_SERVICE_TYPE_LENGTH);
						sms.setEServiceType(type.get());
					}
					break;
				case 15: //receipted_message_id ������������ �����
					{
						__tc__("storeCorrectSms.rcptMsgIdMaxLength");
						__set_str_body_tag__(SMPP_RECEIPTED_MESSAGE_ID, MAX_MSG_ID_LENGTH);
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			SMSId smsId = msgStore->getNextId();
			__trace2__("storeCorrectSms(%d): smsId = %s", s.value(), str(smsId));
			msgStore->createSms(sms, smsId, CREATE_NEW);
			__tc_ok__;
			if (idp != NULL && smsp != NULL)
			{
				*idp = smsId;
				*smsp = sms;
			}
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
			*idp = 0;
			SmsUtil::clearSms(smsp);
		}
	}
}

void MessageStoreTestCases::storeSimilarSms(SMSId* idp, SMS* smsp,
	const SMSId existentId, const SMS& existentSms, int num)
{
	TCSelector s(num, 7);
	__decl_tc__;
	//messageReference
	uint8_t msgRef = existentSms.getMessageReference();
	//originatingAddress
	const Address& origAddr = existentSms.getOriginatingAddress();
	AddressValue origAddrValue;
	uint8_t origAddrLength = origAddr.getValue(origAddrValue);
	uint8_t origAddrType = origAddr.getTypeOfNumber();
	uint8_t origAddrPlan = origAddr.getNumberingPlan();
	//destinationAddress
	const Address& destAddr = existentSms.getDestinationAddress();
	AddressValue destAddrValue;
	uint8_t destAddrLength = destAddr.getValue(destAddrValue);
	uint8_t destAddrType = destAddr.getTypeOfNumber();
	uint8_t destAddrPlan = destAddr.getNumberingPlan();
	//����� �����
	int randAddrLength = rand1(MAX_ADDRESS_VALUE_LENGTH);
	auto_ptr<char> randAddrValue = rand_char(randAddrLength);
	for (; s.check(); s++)
	{
		try
		{
			SMS sms;
			SmsUtil::setupRandomCorrectSms(&sms, mask, check);
			CreateMode flag = ETSI_REJECT_IF_PRESENT;
			uint8_t shift = rand1(254);
			switch(s.value())
			{
				case 1: //������� ������ � msgRef
					__tc__("storeSimilarSms.diffMsgRef");
					sms.setMessageReference(msgRef + shift);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddr);
					break;
				case 2: //������� ������ � origAddrType
					__tc__("storeSimilarSms.diffOrigAddr.diffType");
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddrLength,
						origAddrType + shift, origAddrPlan, origAddrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 3: //������� ������ � origAddrPlan
					__tc__("storeSimilarSms.diffOrigAddr.diffPlan");
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddrLength, origAddrType,
						origAddrPlan + shift, origAddrValue);
					sms.setDestinationAddress(destAddr);
					break;
				case 4: //������� ������ � origAddrValue
					__tc__("storeSimilarSms.diffOrigAddr.diffValue");
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(randAddrLength, origAddrType,
						origAddrPlan, randAddrValue.get());
					sms.setDestinationAddress(destAddr);
					break;
				case 5: //������� ������ � destAddrType
					__tc__("storeSimilarSms.diffDestAddr.diffType");
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddrLength,
						destAddrType + shift, destAddrPlan, destAddrValue);
					break;
				case 6: //������� ������ � destAddrPlan
					__tc__("storeSimilarSms.diffDestAddr.diffPlan");
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(destAddrLength, destAddrType,
						destAddrPlan + shift, destAddrValue);
					break;
				case 7: //������� ������ � destAddrValue
					__tc__("storeSimilarSms.diffDestAddr.diffValue");
					sms.setMessageReference(msgRef);
					sms.setOriginatingAddress(origAddr);
					sms.setDestinationAddress(randAddrLength, destAddrType,
						destAddrPlan, randAddrValue.get());
					break;
				default:
					__unreachable__("Invalid num");
			}
			SMSId smsId = msgStore->getNextId();
			__trace2__("storeSimilarSms(%d): smsId = %s", s.value(), str(smsId));
			msgStore->createSms(sms, smsId, flag);
			if (smsId == existentId)
			{
				__tc_fail__(s.value());
			}
			__tc_ok_cond__;
			if (idp != NULL && smsp != NULL)
			{
				*idp = smsId;
				*smsp = sms;
			}
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
			*idp = 0;
			SmsUtil::clearSms(smsp);
		}
	}
}

void MessageStoreTestCases::storeDuplicateSms(SMSId* idp, SMS* smsp,
	const SMSId existentId, const SMS& existentSms)
{
	__decl_tc__;
	__tc__("storeDuplicateSms");
	try
	{
		SMS sms(existentSms);
		SMSId id = msgStore->getNextId();
		__trace2__("storeDuplicateSms(): smsId = %s", str(id));
		msgStore->createSms(sms, id, CREATE_NEW);
		if (id == existentId)
		{
			__tc_fail__(101);
		}
		__tc_ok_cond__;
		if (idp != NULL && smsp != NULL)
		{
			*idp = id;
			*smsp = sms;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
		*idp = 0;
		SmsUtil::clearSms(smsp);
	}
}

void MessageStoreTestCases::storeRejectDuplicateSms(const SMS& existentSms)
{
	__decl_tc__;
	__tc__("storeRejectDuplicateSms");
	try
	{
		SMS sms;
		SmsUtil::setupRandomCorrectSms(&sms, mask, check);
		//����������� � SMPP v3.4
		//�������� GSM 03.40 ����� 9.2.3.25 ������ ���������: TP-MR, TP-DA, OA.
		sms.setMessageReference(existentSms.getMessageReference());
		sms.setOriginatingAddress(existentSms.getOriginatingAddress());
		//sms.setDestinationAddress(existentSms.getDestinationAddress());
		sms.setDealiasedDestinationAddress(existentSms.getDealiasedDestinationAddress());
		SMSId smsId = msgStore->getNextId();
		__trace2__("storeRejectDuplicateSms(): smsId = %s", str(smsId));
		msgStore->createSms(sms, smsId, ETSI_REJECT_IF_PRESENT);
		__tc_fail__(101);
	}
	catch(DuplicateMessageException&)
	{
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void MessageStoreTestCases::storeReplaceCorrectSms(SMSId* idp, SMS* existentSms)
{
	__decl_tc__;
	__tc__("storeReplaceCorrectSms");
	try
	{
		SMS sms;
		SmsUtil::setupRandomCorrectSms(&sms, mask, check);
		//����������� � GSM 03.40
		//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
		//destination address and service_type. ��������� ������ ���� � 
		//ENROTE state.
		sms.setOriginatingAddress(existentSms->getOriginatingAddress());
		sms.setDestinationAddress(existentSms->getDestinationAddress());
		sms.setDealiasedDestinationAddress(existentSms->getDealiasedDestinationAddress());
		EService serviceType;
		existentSms->getEServiceType(serviceType);
		sms.setEServiceType(serviceType);
		SMSId smsId = msgStore->getNextId();
		__trace2__("storeReplaceCorrectSms(): smsId = %s", str(smsId));
		msgStore->createSms(sms, smsId, SMPP_OVERWRITE_IF_PRESENT);
		__tc_ok__;
		//������������ ��������� �������� �� existentSms
		sms.attempts = existentSms->getAttemptsCount();
		sms.lastResult = existentSms->getLastResult();
		sms.lastTime = existentSms->getLastTime();
		//������� ���������
		*idp = smsId;
		*existentSms = sms;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void MessageStoreTestCases::storeReplaceSmsInFinalState(SMSId* idp, SMS* smsp,
	const SMS& existentSms)
{
	__decl_tc__;
	__tc__("storeReplaceSmsInFinalState");
	try
	{
		//����������� � GSM 03.40
		//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
		//destination address and service_type � ��������� ������ ���� � 
		//ENROTE state. � ��������� ������ ��������� ����� ���������.
		SMS sms(existentSms);
		SMSId smsId = msgStore->getNextId();
		__trace2__("storeReplaceSmsInFinalState(): smsId = %s", str(smsId));
		msgStore->createSms(sms, smsId, SMPP_OVERWRITE_IF_PRESENT);
		__tc_ok__;
		if (idp != NULL && smsp != NULL)
		{
			*idp = smsId;
			*smsp = sms;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
		*idp = 0;
		SmsUtil::clearSms(smsp);
	}
}

void MessageStoreTestCases::storeIncorrectSms(int num)
{
	__decl_tc__;
	__tc__("storeIncorrectSms");
	//����������� ���������� ��������:
	//������������ ������ DELIVERED
	//������������ ������ EXPIRED
	//������������ ������ UNDELIVERABLE
	//������������ ������ DELETED
	//���� ���������� ��� ����������
	//waitTime > validTime
	__tc_ok__;
}

void MessageStoreTestCases::storeAssertSms(int num)
{
	TCSelector s(num, 11);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			SMS sms;
			SMS& p = sms;
			SmsUtil::setupRandomCorrectSms(&sms, mask, check);
			switch(s.value())
			{
				case 1: //������ destinationAddress
					__tc__("storeAssertSms.destAddrMarginal");
					sms.setDestinationAddress(0, 0, 0, NULL);
					break;
				case 2: //������ destinationAddress
					__tc__("storeAssertSms.destAddrMarginal");
					sms.setDestinationAddress(0, 20, 30, "*");
					break;
				case 3: //������ dealiasedDestinationAddress
					__tc__("storeAssertSms.dealiasedDestAddrMarginal");
					sms.setDealiasedDestinationAddress(0, 20, 30, "*");
					break;
				case 4: //originatingAddress ������ ������������ �����
					{
						__tc__("storeAssertSms.origAddrMarginal");
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setOriginatingAddress(MAX_ADDRESS_LENGTH + 1, 20, 30, addr.get());
					}
					break;
				case 5: //destinationAddress ������ ������������ �����
					{
						__tc__("storeAssertSms.destAddrMarginal");
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setDestinationAddress(MAX_ADDRESS_LENGTH + 1, 20, 30, addr.get());
					}
					break;
				case 6: //dealiasedDestinationAddress ������ ������������ �����
					{
						__tc__("storeAssertSms.dealiasedDestAddrMarginal");
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setDealiasedDestinationAddress(MAX_ADDRESS_LENGTH + 1, 20, 30, addr.get());
					}
					break;
				case 7: //msc ����� � originatingDescriptor ������ ������������ �����
					{
						__tc__("storeAssertSms.origDescMscAddrGreaterMaxLength");
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setOriginatingDescriptor(MAX_ADDRESS_LENGTH + 1, addr.get(), 1, "*", 1);
					}
					break;
				case 8: //imsi ����� � originatingDescriptor ������ ������������ �����
					{
						__tc__("storeAssertSms.origDescImsiAddrGreaterMaxLength");
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						sms.setOriginatingDescriptor(1, "*", MAX_ADDRESS_LENGTH + 1, addr.get(), 1);
					}
					break;
				case 9: //msc ����� � Descriptor ������ ������������ �����
					{
						__tc__("storeAssertSms.descMscAddrGreaterMaxLength");
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						Descriptor(MAX_ADDRESS_LENGTH + 1, addr.get(), 1, "*", 1);
					}
					break;
				case 10: //imsi ����� � Descriptor ������ ������������ �����
					{
						__tc__("storeAssertSms.descImsiAddrGreaterMaxLength");
						auto_ptr<char> addr = rand_char(MAX_ADDRESS_LENGTH + 1);
						Descriptor(1, "*", MAX_ADDRESS_LENGTH + 1, addr.get(), 1);
					}
					break;
				case 11: //service_type ������ ������������ �����
					{
						__tc__("storeAssertSms.serviceTypeGreaterMaxLength");
						__set_str__(EServiceType, MAX_SERVICE_TYPE_LENGTH + 1);
					}
					break;
				/*
				case 11: //short_message ������ ������������ �����
					{
						__tc__("storeAssertSms.smGreaterMaxLength");
						__set_int_body_tag__(SMPP_SM_LENGTH, MAX_SM_LENGTH + 1);
						__set_bin_body_tag__(SMPP_SHORT_MESSAGE, MAX_SM_LENGTH + 1);
					}
					break;
				case 12: //message_payload ������ ������������ �����
					{
						__tc__("storeAssertSms.payloadGreaterMaxLength");
						__set_bin_body_tag__(SMPP_MESSAGE_PAYLOAD, MAX_PAYLOAD_LENGTH + 1);
					}
					break;
				case 14: //receipted_message_id ������ ������������ �����
					{
						__tc__("storeAssertSms.rcptMsgIdGreaterMaxLength");
						__set_str_body_tag__(SMPP_RECEIPTED_MESSAGE_ID, MAX_MSG_ID_LENGTH + 1);
					}
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			//SMSId smsId = msgStore->store(sms);
			__tc_fail__(s.value());
		}
		catch(AssertException&)
		{
			//ok
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
		__tc_ok_cond__;
	}
}

void MessageStoreTestCases::changeExistentSmsStateEnrouteToEnroute(
	const SMSId id, SMS* sms, int num)
{
	TCSelector s(num, 5);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			Descriptor dst;
			dst.setSmeNumber(rand0(65535));
			uint8_t lastResult = rand1(255);
			time_t nextTryTime = time(NULL) + rand0(3600);
			switch(s.value())
			{
				case 1: //������ imsi � msc (��������, SME �� ������ imsi � msc)
					__tc__("changeExistentSmsStateEnrouteToEnroute.destDescMarginal");
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					break;
				case 2: //������ imsi � msc (��������, SME �� ������ imsi � msc)
					__tc__("changeExistentSmsStateEnrouteToEnroute.destDescMarginal");
					dst.setMsc(0, "*");
					dst.setImsi(0, "*");
					break;
				case 3: //imsi � msc ������ ������������ �����
					{
						__tc__("changeExistentSmsStateEnrouteToEnroute.destDescMarginal");
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						dst.setMsc(MAX_ADDRESS_LENGTH, mscAddr.get());
						dst.setImsi(MAX_ADDRESS_LENGTH, imsiAddr.get());
					}
					break;
				case 4: //nextTryTime � ������� �������
					__tc__("changeExistentSmsStateEnrouteToEnroute.nextTimePast");
					nextTryTime -= rand0(3600);
					break;
				case 5: //lastResult �� �����
					__tc__("changeExistentSmsStateEnrouteToEnroute.lastResultZero");
					lastResult = 0;
					break;
				default:
					__unreachable__("Invalid num");
			}
			__trace2__("changeExistentSmsStateEnrouteToEnroute(%d): smsId = %s", s.value(), str(id));
			msgStore->changeSmsStateToEnroute(id, dst, lastResult, nextTryTime);
			__tc_ok__;
			sms->setNextTime(nextTryTime);
			//hack, ��� ������� ���������
			sms->destinationDescriptor = dst;
			sms->lastResult = lastResult;
			sms->attempts++;
			sms->lastTime = time(NULL);
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

//hack, ��� ������� ���������
#define SET_DELIVERED_SMS sms->state = DELIVERED; \
	sms->destinationDescriptor = dst; \
	sms->lastTime = time(NULL); \
	sms->setNextTime(0); \
	sms->lastResult = 0; \
	sms->attempts++;

//hack, ��� ������� ���������
#define SET_UNDELIVERABLE_SMS sms->state = UNDELIVERABLE; \
	sms->destinationDescriptor = dst; \
	sms->lastTime = time(NULL); \
	sms->setNextTime(0); \
	sms->lastResult = lastResult; \
	sms->attempts++;

void MessageStoreTestCases::changeExistentSmsStateEnrouteToFinal(
	const SMSId id, SMS* sms, int num)
{
	TCSelector s(num, 9);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			Descriptor dst;
			SmsUtil::setupRandomCorrectDescriptor(&dst);
			dst.setSmeNumber(rand0(65535));
			uint8_t lastResult = rand1(255);
			switch(s.value())
			{
				case 1: //DELIVERED, ������ imsi � msc
					__tc__("changeExistentSmsStateEnrouteToFinal.stateDeliveredDestDescMarginal");
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					__trace2__("changeExistentSmsStateEnrouteToFinal(%d): smsId = %s, state = DELIVERED", s.value(), str(id));
					msgStore->changeSmsStateToDelivered(id, dst);
					SET_DELIVERED_SMS
					break;
				case 2: //DELIVERED, ������ imsi � msc
					__tc__("changeExistentSmsStateEnrouteToFinal.stateDeliveredDestDescMarginal");
					dst.setMsc(0, "*");
					dst.setImsi(0, "*");
					__trace2__("changeExistentSmsStateEnrouteToFinal(%d): smsId = %s, state = DELIVERED", s.value(), str(id));
					msgStore->changeSmsStateToDelivered(id, dst);
					SET_DELIVERED_SMS
					break;
				case 3: //DELIVERED, imsi � msc ������ ������������ �����
					{
						__tc__("changeExistentSmsStateEnrouteToFinal.stateDeliveredDestDescMarginal");
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						dst.setMsc(MAX_ADDRESS_LENGTH, mscAddr.get());
						dst.setImsi(MAX_ADDRESS_LENGTH, imsiAddr.get());
					}
					__trace2__("changeExistentSmsStateEnrouteToFinal(%d): smsId = %s, state = DELIVERED", s.value(), str(id));
					msgStore->changeSmsStateToDelivered(id, dst);
					SET_DELIVERED_SMS
					break;
				case 4: //UNDELIVERABLE, ������ imsi � msc
					__tc__("changeExistentSmsStateEnrouteToFinal.stateUndeliverableDestDescMarginal");
					dst.setMsc(0, NULL);
					dst.setImsi(0, NULL);
					__trace2__("changeExistentSmsStateEnrouteToFinal(%d): smsId = %s, state = UNDELIVERABLE", s.value(), str(id));
					msgStore->changeSmsStateToUndeliverable(id, dst, lastResult);
					SET_UNDELIVERABLE_SMS
					break;
				case 5: //UNDELIVERABLE, ������ imsi � msc
					__tc__("changeExistentSmsStateEnrouteToFinal.stateUndeliverableDestDescMarginal");
					dst.setMsc(0, "*");
					dst.setImsi(0, "*");
					__trace2__("changeExistentSmsStateEnrouteToFinal(%d): smsId = %s, state = UNDELIVERABLE", s.value(), str(id));
					msgStore->changeSmsStateToUndeliverable(id, dst, lastResult);
					SET_UNDELIVERABLE_SMS
					break;
				case 6: //UNDELIVERABLE, imsi � msc ������ ������������ �����
					{
						__tc__("changeExistentSmsStateEnrouteToFinal.stateUndeliverableDestDescMarginal");
						auto_ptr<char> mscAddr = rand_char(MAX_ADDRESS_LENGTH);
						auto_ptr<char> imsiAddr = rand_char(MAX_ADDRESS_LENGTH);
						dst.setMsc(MAX_ADDRESS_LENGTH, mscAddr.get());
						dst.setImsi(MAX_ADDRESS_LENGTH, imsiAddr.get());
					}
					__trace2__("changeExistentSmsStateEnrouteToFinal(%d): smsId = %s, state = UNDELIVERABLE", s.value(), str(id));
					msgStore->changeSmsStateToUndeliverable(id, dst, lastResult);
					SET_UNDELIVERABLE_SMS
					break;
				case 7: //UNDELIVERABLE, lastResult �� �����
					__tc__("changeExistentSmsStateEnrouteToFinal.stateUndeliverableLastResultEmpty");
					lastResult = 0;
					__trace2__("changeExistentSmsStateEnrouteToFinal(%d): smsId = %s, state = UNDELIVERABLE", s.value(), str(id));
					msgStore->changeSmsStateToUndeliverable(id, dst, lastResult);
					SET_UNDELIVERABLE_SMS
					break;
				case 8: //EXPIRED
					__tc__("changeExistentSmsStateEnrouteToFinal.stateExpired");
					__trace2__("changeExistentSmsStateEnrouteToFinal(%d): smsId = %s, state = EXPIRED", s.value(), str(id));
					msgStore->changeSmsStateToExpired(id);
					sms->setNextTime(0);
					//hack, ��� ������� ���������
					sms->state = EXPIRED;
					//sms->lastTime = time(NULL);
					break;
				case 9: //DELETED
					__tc__("changeExistentSmsStateEnrouteToFinal.stateDeleted");
					__trace2__("changeExistentSmsStateEnrouteToFinal(%d): smsId = %s, state = DELETED", s.value(), str(id));
					msgStore->changeSmsStateToDeleted(id);
					sms->setNextTime(0);
					//hack, ��� ������� ���������
					sms->state = DELETED;
					//sms->lastTime = time(NULL);
					break;
				default:
					__unreachable__("Invalid num");
			}
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void MessageStoreTestCases::changeFinalSmsStateToAny(const SMSId id, int num)
{
	TCSelector s(num, 5);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			Descriptor dst;
			SmsUtil::setupRandomCorrectDescriptor(&dst);
			uint8_t lastResult = rand1(255);
			time_t nextTryTime = time(NULL);
			switch(s.value())
			{
				case 1: //ENROUTE
					__tc__("changeFinalSmsStateToAny.stateEnrote");
					__trace2__("changeFinalSmsStateToAny(%d): smsId = %s, state = ENROUTE", s.value(), str(id));
					msgStore->changeSmsStateToEnroute(id, dst, lastResult, nextTryTime);
					break;
				case 2: //DELIVERED
					__tc__("changeFinalSmsStateToAny.stateDelivered");
					__trace2__("changeFinalSmsStateToAny(%d): smsId = %s, state = DELIVERED", s.value(), str(id));
					msgStore->changeSmsStateToDelivered(id, dst);
					break;
				case 3: //UNDELIVERABLE
					__tc__("changeFinalSmsStateToAny.stateUndeliverable");
					__trace2__("changeFinalSmsStateToAny(%d): smsId = %s, state = UNDELIVERABLE", s.value(), str(id));
					msgStore->changeSmsStateToUndeliverable(id, dst, lastResult);
					break;
				case 4: //EXPIRED
					__tc__("changeFinalSmsStateToAny.stateExpired");
					__trace2__("changeFinalSmsStateToAny(%d): smsId = %s, state = EXPIRED", s.value(), str(id));
					msgStore->changeSmsStateToExpired(id);
					break;
				case 5: //DELETED
					__tc__("changeFinalSmsStateToAny.stateDeleted");
					__trace2__("changeFinalSmsStateToAny(%d): smsId = %s, state = DELETED", s.value(), str(id));
					msgStore->changeSmsStateToDeleted(id);
					break;
				default:
					__unreachable__("Invalid num");
			}
			__tc_fail__(s.value());
		}
		catch(NoSuchMessageException&)
		{
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

#define __prepare_for_replace_sms__ \
	int len = rand1(MAX_SM_LENGTH); \
	uint8_t buf[MAX_SM_LENGTH]; \
	uint8_t* sm = buf; \
	rand_uint8_t(len, sm); \
	uint8_t deliveryReport = rand0(255); \
	time_t validTime = time(NULL) + 100; \
	time_t nextTime = time(NULL) + 50;

void MessageStoreTestCases::replaceCorrectSms(const SMSId id, SMS* sms, int num)
{
	TCSelector s(num, 6);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			__prepare_for_replace_sms__;
			switch(s.value())
			{
				case 1: //�������� ��� ��������� ����
					__tc__("replaceCorrectSms.replaceAll");
					break;
				case 2: //�������� schedule_delivery_time ��� ���������
					__tc__("replaceCorrectSms.nextTimeUnchanged");
					nextTime = 0;
					break;
				case 3: //�������� validity_period ��� ���������
					__tc__("replaceCorrectSms.validTimeUnchanged");
					validTime = 0;
					break;
				case 4: //������ ���� ���������
					__tc__("replaceCorrectSms.smMarginal");
					len = 0;
					break;
				case 5: //������ ���� ���������
					__tc__("replaceCorrectSms.smMarginal");
					len = 0;
					sm = NULL;
					break;
				case 6: //���� ��������� ������������ �����
					__tc__("replaceCorrectSms.smMarginal");
					len = MAX_SM_LENGTH;
					rand_uint8_t(len, sm);
					break;
				default:
					__unreachable__("Invalid num");
			}
			__trace2__("replaceCorrectSms(%d): smsId = %s", s.value(), str(id));
			msgStore->replaceSms(id, sms->getOriginatingAddress(),
                sm, len, deliveryReport, validTime, nextTime);
			__tc_ok__;
			//������ �����, ����� � ������� ������ �������
			SMS& p = *sms;
			__set_bin_body_tag2__(SMPP_SHORT_MESSAGE, len, (char*) sm);
			__set_int_body_tag__(SMPP_SM_LENGTH, len);
			sms->setDeliveryReport(deliveryReport);
			if (validTime)
			{
				sms->setValidTime(validTime);
			}
			if (nextTime)
			{
				sms->setNextTime(nextTime);
			}
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void MessageStoreTestCases::replaceIncorrectSms(const SMSId id,
	const SMS& sms, int num)
{
	//��������� �������� ������ ���������� �������:
	//���� ���������� ��� ����������
	//waitTime > validTime

	TCSelector s(num, 3);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			Address addr(sms.getOriginatingAddress());
			__prepare_for_replace_sms__;
			switch(s.value())
			{
				case 1: //������� ������ � TypeOfNumber
					__tc__("replaceIncorrectSms.diffType");
					addr.setTypeOfNumber(addr.getTypeOfNumber() + 1);
					break;
				case 2: //������� ������ � NumberingPlan
					__tc__("replaceIncorrectSms.diffPlan");
					addr.setNumberingPlan(addr.getNumberingPlan() + 1);
					break;
				case 3: //������� ������ � Value
					{
						__tc__("replaceIncorrectSms.diffValue");
						int len = rand2(10, MAX_ADDRESS_VALUE_LENGTH);
						auto_ptr<char> val = rand_char(len);
						addr.setValue(len, val.get());
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			__trace2__("replaceIncorrectSms(%d): smsId = %s", s.value(), str(id));
			msgStore->replaceSms(id, addr, sm, len, deliveryReport, validTime, nextTime);
			__tc_fail__(s.value());
		}
		catch(NoSuchMessageException&)
		{
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void MessageStoreTestCases::replaceFinalSms(const SMSId id, const SMS& sms)
{
	__decl_tc__;
	__tc__("replaceIncorrectSms.replaceFinalSms");
	try
	{
		__prepare_for_replace_sms__;
		__trace2__("replaceFinalSms(): smsId = %s", str(id));
		msgStore->replaceSms(id, sms.getOriginatingAddress(), sm, len,
			deliveryReport, validTime, nextTime);
		__tc_fail__(101);
	}
	catch(NoSuchMessageException&)
	{
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void MessageStoreTestCases::loadExistentSms(const SMSId id, const SMS& sms)
{
	__decl_tc__;
	__tc__("loadSms.existentSms");
	try
	{
		SMS _sms;
		__trace2__("loadExistentSms(): smsId = %s", str(id));
		msgStore->retriveSms(id, _sms);
		if (&sms == NULL || &_sms == NULL)
		{
			__tc_fail__(101);
		}
		else
		{
			__tc_fail2__(SmsUtil::compareMessages(sms, _sms), 0);
			__tc_ok_cond__;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void MessageStoreTestCases::loadNonExistentSms(const SMSId id)
{
	__decl_tc__;
	__tc__("loadSms.nonExistentSms");
	try
	{
		SMS sms;
		__trace2__("loadNonExistentSms(): smsId = %s", str(id));
		msgStore->retriveSms(id, sms);
		__tc_fail__(101);
	}
	catch(NoSuchMessageException&)
	{
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void MessageStoreTestCases::deleteExistentSms(const SMSId id)
{
	__decl_tc__;
	__tc__("deleteSms.existentSms");
	try
	{
		__trace2__("deleteExistentSms(): smsId = %s", str(id));
		msgStore->destroySms(id);
		__tc_ok__;
	}
	catch (...)
	{
		__tc_fail__(100);
		error();
	}
}
	
void MessageStoreTestCases::deleteNonExistentSms(const SMSId id)
{
	__decl_tc__;
	__tc__("deleteSms.nonExistentSms");
	try
	{
		__trace2__("deleteNonExistentSms(): smsId = %s", str(id));
		msgStore->destroySms(id);
		__tc_fail__(101);
	}
	catch (NoSuchMessageException&)
	{
		__tc_ok__;
	}
	catch (...)
	{
		__tc_fail__(100);
		error();
	}
}

void MessageStoreTestCases::checkReadyForRetrySms(const vector<SMSId*>& ids,
	const vector<SMS*>& sms, int num)
{
	TCSelector s(num, 4);
	//�������� �������
	time_t minNextTime, middleNextTime, maxNextTime;
	checkNextRetryTime(ids, sms, minNextTime, middleNextTime, maxNextTime);
	//�������� ������ MessageStore::getReadyForRetry()
	for (; s.check(); s++)
	{
		switch(s.value())
		{
			case 1: //������ minNextTime
				compareReadyForRetrySmsList(ids, sms, minNextTime - 1, 10);
				break;
			case 2: //����� middleNextTime
				compareReadyForRetrySmsList(ids, sms, middleNextTime, 20);
				break;
			case 3: //����� ������-�� �������� �������
				compareReadyForRetrySmsList(ids, sms,
					(minNextTime + maxNextTime) / 2, 30);
				break;
			case 4: //������ maxNextTime
				compareReadyForRetrySmsList(ids, sms, maxNextTime + 1, 40);
				break;
			default:
				__unreachable__("Invalid num");
		}
	}
}
void MessageStoreTestCases::checkNextRetryTime(const vector<SMSId*>& ids, 
	const vector<SMS*>& sms, time_t& minNextTime, time_t& middleNextTime,
	time_t& maxNextTime)
{
	__decl_tc__;
	__tc__("checkReadyForRetrySms.nextRetryTime");
	bool found = false;
	minNextTime = 0;
	middleNextTime = 0;
	maxNextTime = 0;
	for (int i = 0; i < sms.size(); i++)
	{
ostringstream os;
os << "smsId = " << *ids[i] << ", nextTime = " << sms[i]->getNextTime();
__trace2__("checkNextRetryTime(): %s", os.str().c_str());
		if (sms[i]->getState() != ENROUTE)
		{
			continue;
		}
		if (!sms[i]->getNextTime())
		{
			continue;
		}
		if (!found)
		{
			found = true;
			minNextTime = sms[i]->getNextTime();
			middleNextTime = sms[i]->getNextTime();
			maxNextTime = sms[i]->getNextTime();
			continue;
		}
		if (sms[i]->getNextTime() < minNextTime)
		{
			middleNextTime = minNextTime;
			minNextTime = sms[i]->getNextTime();
			continue;
		}
		if (sms[i]->getNextTime() > maxNextTime)
		{
			middleNextTime = maxNextTime;
			maxNextTime = sms[i]->getNextTime();
			continue;
		}
	}
	//MessageStore::getNextRetryTime()
	try
	{
		time_t nextTime = msgStore->getNextRetryTime();
		__trace2__("@@@ checkReadyForRetrySms(): nextTime = %ld", nextTime);
		if (!found && nextTime != 0)
		{
			__tc_fail__(1);
		}
		else if (found && nextTime != minNextTime)
		{
			__trace2__("checkReadyForRetrySms(): nextTime = %ld, minNextTime = %ld",
				nextTime, minNextTime);
			__tc_fail__(2);
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void MessageStoreTestCases::compareReadyForRetrySmsList(const vector<SMSId*>& ids, 
	const vector<SMS*>& sms, time_t t, int shift)
{
	__decl_tc__;
	__tc__("checkReadyForRetrySms.readyForRetrySmsList");
	//������� � ������������� ids
	typedef map<SMSId, time_t> IdMap;
	IdMap enroteIds;
	for (int i = 0; i < sms.size(); i++)
	{
		if (sms[i]->getState() == ENROUTE && sms[i]->getNextTime() > 0 &&
			sms[i]->getNextTime() <= t)
		{
			enroteIds[*ids[i]] = sms[i]->getNextTime();
		}
	}
	//������� ������
	bool sortOrderOk = true;
	bool idFound = true;
	try
	{
		SMSId id;
		time_t prevTime = 0;
		IdIterator* it = msgStore->getReadyForRetry(t);
		while (it->getNextId(id))
		{
			IdMap::iterator idIt = enroteIds.find(id);
			if (idIt == enroteIds.end())
			{
				idFound = false;
				continue;
			}
			time_t curTime = idIt->second;
			if (curTime < prevTime)
			{
				sortOrderOk = false;
				continue;
			}
			prevTime = curTime;
			enroteIds.erase(idIt);
			//log.debug("dbid = %d, msid = %d", id, enroteIds[i].smsId);
		}
		delete it;
	}
	catch (...)
	{
		__tc_fail__(100);
		error();
	}
	if (!idFound)
	{
		__tc_fail__(shift + 1);
	}
	if (!sortOrderOk)
	{
		__tc_fail__(shift + 2);
	}
	if (enroteIds.size())
	{
//log.debug("dbid!!! = %d", id);
		__tc_fail__(shift + 3);
	}
	__tc_ok_cond__;
}

}
}
}

