#include "SmppProtocolTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::test::conf::TestConfig;
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::sms; //constants
using namespace smsc::test::smpp;
using namespace smsc::test::core; //constants
using namespace smsc::test::util;

SmppProtocolTestCases::SmppProtocolTestCases(SmppFixture* _fixture)
: fixture(_fixture), chkList(_fixture->chkList) {}

Category& SmppProtocolTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppProtocolTestCases");
	return log;
}

void SmppProtocolTestCases::submitSmAssert(int num)
{
	TCSelector s(num, 4);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm pdu;
			switch (s.value())
			{
				case 1: //serviceType больше максимальной длины
					{
						__tc__("submitSm.assert.serviceTypeInvalid");
						char serviceType[MAX_SERVICE_TYPE_LENGTH + 10];
						rand_char(MAX_SERVICE_TYPE_LENGTH + 1, serviceType);
						pdu.get_message().set_serviceType(serviceType);
					}
					break;
				case 2: //пустой адрес отправителя
					{
						__tc__("submitSm.assert.destAddrLengthInvalid");
						PduAddress addr;
						addr.set_typeOfNumber(rand0(255));
						addr.set_numberingPlan(rand0(255));
						addr.set_value("");
						pdu.get_message().set_dest(addr);
					}
					break;
				case 3: //destinationAddress больше максимальной длины
					{
						__tc__("submitSm.assert.destAddrLengthInvalid");
						PduAddress addr;
						addr.set_typeOfNumber(rand0(255));
						addr.set_numberingPlan(rand0(255));
						char addrVal[MAX_ADDRESS_LENGTH + 10];
						rand_char(MAX_ADDRESS_LENGTH + 1, addrVal);
						addr.set_value(addrVal);
						pdu.get_message().set_dest(addr);
					}
					break;
				case 4: //message body больше максимальной длины
					{
						__tc__("submitSm.assert.msgLengthInvalid");
						int len = MAX_SM_LENGTH + 1;
						auto_ptr<char> msg = rand_char(len);
						pdu.get_message().set_shortMessage(msg.get(), len);
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			__dumpSubmitSmPdu__("SmppProtocolTestCases::submitSmAssert", fixture->smeInfo.systemId, &pdu);
			__tc_fail__(s.value());
		}
		catch (...)
		{
			__tc_ok__;
		}
	}
}

PduData* SmppProtocolTestCases::getNonReplaceEnrotePdu()
{
	__require__(fixture->pduReg);
	__cfg_int__(sequentialPduInterval);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		time(NULL) + sequentialPduInterval, LONG_MAX);
	//ищу корректную незамещающую и незамещенную ранее pdu
	PduData* pduData = NULL;
	while (PduMonitor* m = it->next())
	{
		if (m->getType() == DELIVERY_MONITOR && !m->pduData->replacedByPdu &&
			!m->pduData->replacePdu && m->pduData->valid &&
			m->getFlag() == PDU_REQUIRED_FLAG &&
			!m->pduData->intProps.size())
		{
			pduData = m->pduData;
			break;
		}
	}
	delete it;
	return pduData;
}

PduData* SmppProtocolTestCases::getReplaceEnrotePdu()
{
	__require__(fixture->pduReg);
	__cfg_int__(sequentialPduInterval);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		time(NULL) + sequentialPduInterval, LONG_MAX);
	//ищу замещающую и незамещенную ранее pdu
	PduData* pduData = NULL;
	while (PduMonitor* m = it->next())
	{
		if (m->getType() == DELIVERY_MONITOR && !m->pduData->replacedByPdu &&
			m->pduData->replacePdu && m->pduData->valid &&
			m->getFlag() == PDU_REQUIRED_FLAG &&
			!m->pduData->intProps.size())
		{
			pduData = m->pduData;
			break;
		}
	}
	delete it;
	return pduData;
}

PduData* SmppProtocolTestCases::getNonReplaceRescheduledEnrotePdu()
{
	__require__(fixture->pduReg);
	__cfg_int__(sequentialPduInterval);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		time(NULL) + sequentialPduInterval, LONG_MAX);
	//ищу незамещенную ранее pdu
	PduData* pduData = NULL;
	while (PduMonitor* m = it->next())
	{
		if (m->getType() == DELIVERY_MONITOR && !m->pduData->replacedByPdu &&
			m->pduData->valid && m->getFlag() == PDU_REQUIRED_FLAG &&
			!m->pduData->intProps.size())
		{
			DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
			__require__(monitor);
			//была попытка доставки
			if (monitor->getLastTime())
			{
				pduData = monitor->pduData;
				break;
			}
		}
	}
	delete it;
	return pduData;
}

PduData* SmppProtocolTestCases::getFinalPdu()
{
	__require__(fixture->pduReg);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(0, LONG_MAX);
	//ищу незамещенную ранее pdu
	PduData* pduData = NULL;
	while (PduMonitor* m = it->next())
	{
		if (m->getType() == DELIVERY_MONITOR && !m->pduData->replacedByPdu &&
			m->pduData->valid && m->getFlag() == PDU_RECEIVED_FLAG &&
			!m->pduData->intProps.size())
		{
			pduData = m->pduData;
			break;
		}
	}
	delete it;
	return pduData;
}

void SmppProtocolTestCases::submitSmCorrect(bool sync, int num)
{
	TCSelector s(num, 14);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("submitSm.correct");
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			const Address* destAlias = fixture->smeReg->getRandomAddress();
			__require__(destAlias);
			fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, *destAlias);
			PduData* existentPduData = NULL;
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //ничего особенного
					//__tc__("submitSm.correct");
					break;
				/*
				case 2: //пустой serviceType
					__tc__("submitSm.correct.serviceTypeMarginal");
					//pdu->get_message().set_serviceType(NULL);
					pdu->get_message().set_serviceType("");
					break;
				*/
				case 2: //serviceType максимальной длины
					{
						__tc__("submitSm.correct.serviceTypeMarginal");
						EService serviceType;
						rand_char(MAX_SERVICE_TYPE_LENGTH, serviceType);
						pdu->get_message().set_serviceType(serviceType);
					}
					break;
				case 3: //доставка уже должна была начаться
					{
						__tc__("submitSm.correct.waitTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(maxWaitTime), t, time(NULL), __absoluteTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						//отменить подтверждения доставки и нотификации
						pdu->get_message().set_registredDelivery(0);
					}
					break;
				case 4: //срок валидности больше максимального
					{
						__tc__("submitSm.correct.validTimeExceeded");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) + maxValidPeriod + timeCheckAccuracy, t,
							time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 5: //немедленная доставка
					__tc__("submitSm.correct.waitTimeImmediate");
					pdu->get_message().set_scheduleDeliveryTime("");
					break;
				case 6: //срок валидности по умолчанию
					__tc__("submitSm.correct.validTimeDefault");
					pdu->get_message().set_validityPeriod("");
					break;
				case 7: //пустое тело сообщения
					__tc__("submitSm.correct.smLengthMarginal");
					pdu->get_message().set_shortMessage(NULL, 0);
					//pdu->get_message().set_shortMessage("", 0);
					break;
				case 8: //тело сообщения максимальной длины
					{
						__tc__("submitSm.correct.smLengthMarginal");
						ShortMessage msg;
						rand_char(MAX_SM_LENGTH, msg);
						pdu->get_message().set_shortMessage(msg, MAX_SM_LENGTH);
					}
					break;
				/*
				case 9: //msgRef одинаковые (эквивалентно msgRef отсутствуют)
					//Согласно GSM 03.40 пункт 9.2.3.25 если совпадают
					//TP-MR, TP-DA, OA, то при ETSI_REJECT_IF_PRESENT будет ошибка.
					//Для SMPP все должно работать независимо от msgRef.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.checkMap");
							PduSubmitSm* existentPdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								existentPdu->get_message().get_dest());
							pdu->get_optional().set_userMessageReference(
								existentPdu->get_optional().get_userMessageReference());
						}
					}
					break;
				*/
				case 9: //отправка дублированного сообщения без замещения уже существующего
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.notReplace");
							PduSubmitSm* existentPdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								existentPdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								existentPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(0);
							intProps["hasSmppDuplicates"] = 1;
							existentPduData->intProps["hasSmppDuplicates"] = 1;
						}
					}
					break;
				case 10: //отправка дублированного сообщения с попыткой замещения
					//уже существующего, но service_type не совпадает.
					//Для несовпадающих source address и destination address
					//тест кейсов не делаю, т.к. возникнут проблемы с маршрутами.
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.serviceTypeNotMatch");
							PduSubmitSm* existentPdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							//pdu->get_message().set_serviceType(...);
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								existentPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 11: //отправка дублированного сообщения с замещением уже существующего
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 12: //отправка дублированного сообщения с замещением уже
					//ранее замещенного
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getReplaceEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceReplacedEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 13: //отправка дублированного сообщения с замещением уже
					//существующего, но находящегося уже в финальном состоянии.
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						PduData* finalPduData = getFinalPdu();
						if (finalPduData)
						{
							__require__(finalPduData->pdu &&
								finalPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceFinal");
							PduSubmitSm* finalPdu =
								reinterpret_cast<PduSubmitSm*>(finalPduData->pdu);
							pdu->get_message().set_serviceType(
								finalPdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								finalPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
							intProps["hasSmppDuplicates"] = 1;
							finalPduData->intProps["hasSmppDuplicates"] = 1;
						}
					}
					break;
				case 14: //отправка дублированного сообщения с замещением уже
					//существующего, но находящегося уже в процессе повторной доставки.
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceRescheduledEnrotePdu();
						if (existentPduData)
						{
							__require__(existentPduData->pdu &&
								existentPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc__("submitSm.correct.replaceRepeatedDeliveryEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendSubmitSmPdu(pdu, existentPduData, sync, &intProps);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::submitSmIncorrect(bool sync, int num)
{
	TCSelector s(num, 11);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(maxDeliveryPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("submitSm.incorrect");
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			const Address* destAlias = fixture->smeReg->getRandomAddress();
			__require__(destAlias);
			fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, *destAlias);
			switch (s.value())
			{
				case 1: //неправильный адрес отправителя
					{
						__tc__("submitSm.incorrect.sourceAddr");
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->get_message().set_source(smppAddr);
					}
					break;
				case 2: //неправильный адрес получателя
					{
						__tc__("submitSm.incorrect.destAddr");
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->get_message().set_dest(smppAddr);
					}
					break;
				case 3: //неправильный формат validity_period
					{
						__tc__("submitSm.incorrect.validTimeFormat");
						string t = pdu->get_message().get_validityPeriod();
						t[rand0(MAX_SMPP_TIME_LENGTH - 1)] = 'a';
						pdu->get_message().set_validityPeriod(t.c_str());
					}
					break;
				case 4: //неправильный формат validity_period
					{
						__tc__("submitSm.incorrect.validTimeFormat");
						string t = pdu->get_message().get_validityPeriod();
						t.erase(rand0(MAX_SMPP_TIME_LENGTH - 1), 1);
						pdu->get_message().set_validityPeriod(t.c_str());
					}
					break;
				case 5: //неправильный формат schedule_delivery_time
					{
						__tc__("submitSm.incorrect.waitTimeFormat");
						string t = pdu->get_message().get_scheduleDeliveryTime();
						t[rand0(MAX_SMPP_TIME_LENGTH - 1)] = 'a';
						pdu->get_message().set_scheduleDeliveryTime(t.c_str());
					}
					break;
				case 6: //неправильный формат schedule_delivery_time
					{
						__tc__("submitSm.incorrect.waitTimeFormat");
						string t = pdu->get_message().get_scheduleDeliveryTime();
						t.erase(rand0(MAX_SMPP_TIME_LENGTH - 1), 1);
						pdu->get_message().set_scheduleDeliveryTime(t.c_str());
					}
					break;
				case 7: //срок валидности уже закончился
					{
						__tc__("submitSm.incorrect.validTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - timeCheckAccuracy, t, time(NULL), __absoluteTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 8: //waitTime > validTime
					{
						__tc__("submitSm.incorrect.waitTimeInvalid1");
						SmppTime t;
						time_t validTime = time(NULL) + rand1(maxWaitTime);
						time_t waitTime = validTime + timeCheckAccuracy;
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 9: //waitTime > validTime
					{
						__tc__("submitSm.incorrect.waitTimeInvalid2");
						SmppTime t;
						time_t waitTime = time(NULL) + maxValidPeriod + timeCheckAccuracy;
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						pdu->get_message().set_validityPeriod("");
					}
					break;
				case 10: //waitTime > validTime
					{
						__tc__("submitSm.incorrect.waitTimeInvalid3");
						SmppTime t;
						time_t waitTime = time(NULL) + maxValidPeriod + timeCheckAccuracy;
						time_t validTime = waitTime + rand1(maxDeliveryPeriod);
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 11: //недопустимый dataCoding
					__tc__("submitSm.incorrect.dataCoding");
					pdu->get_message().set_dataCoding(rand1(255));
					break;
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppProtocolTestCases::replaceSm(bool sync, int num)
{
	TCSelector s(num, 11);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(maxDeliveryPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("replaceSm");
	for (; s.check(); s++)
	{
		try
		{
			PduReplaceSm* pdu = new PduReplaceSm();
			//выбрать случайный messageId из sms ожидающих доставки
			PduData* replacePduData = NULL;
			if (fixture->pduReg)
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				replacePduData = getNonReplaceEnrotePdu();
			}
			fixture->transmitter->setupRandomCorrectReplaceSmPdu(pdu, replacePduData);
			switch (s.value())
			{
				case 1: //ничего особенного
					//__tc__("replaceSm");
					break;
				case 2: //доставка уже должна была начаться
					{
						__tc__("replaceSm.waitTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(maxWaitTime), t, time(NULL), __absoluteTime__);
						pdu->set_scheduleDeliveryTime(t);
						//отменить подтверждения доставки и нотификации
						pdu->set_registredDelivery(0);
					}
					break;
				case 3: //срок валидности уже закончился
					{
						__tc__("replaceSm.validTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(maxDeliveryPeriod), t, time(NULL), __absoluteTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 4: //срок валидности больше максимального
					{
						__tc__("replaceSm.validTimeExceeded");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) + maxValidPeriod + timeCheckAccuracy, t,
							time(NULL), __numTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 5: //waitTime > validTime
					{
						__tc__("replaceSm.waitTimeInvalid");
						SmppTime t;
						time_t validTime = time(NULL) + rand1(maxWaitTime);
						time_t waitTime = validTime + rand1(maxWaitTime);
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 6: //пустое тело сообщения
					__tc__("replaceSm.smLengthMarginal");
					pdu->set_shortMessage(NULL, 0);
					//pdu->set_shortMessage("", 0);
					break;
				case 7: //тело сообщения максимальной длины
					{
						__tc__("replaceSm.smLengthMarginal");
						auto_ptr<char> msg = rand_char(MAX_SM_LENGTH);
						pdu->set_shortMessage(msg.get(), MAX_SM_LENGTH);
					}
					break;
				case 8: //замещаемого сообщения не существует
					{
						__tc__("replaceSm.msgIdNotExist");
						auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
						pdu->set_messageId(msgId.get());
						replacePduData = NULL;
					}
					break;
				case 9: //замещение уже ранее замещенного сообщения
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						replacePduData = getReplaceEnrotePdu();
						if (replacePduData)
						{
							__tc__("replaceSm.replaceReplacedEnrote");
							fixture->transmitter->setupRandomCorrectReplaceSmPdu(pdu,
								replacePduData);
							pdu->set_messageId(replacePduData->smsId.c_str());
						}
					}
					break;
				case 10: //замещением существующего сообщения, но находящегося
					//уже в финальном состоянии.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						PduData* finalPduData = getFinalPdu();
						if (finalPduData)
						{
							__tc__("replaceSm.replaceFinal");
							pdu->set_messageId(finalPduData->smsId.c_str());
						}
					}
					break;
				case 11: //замещение существующего сообщения, но находящегося
					//уже в процессе повторной доставки.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						replacePduData = getNonReplaceRescheduledEnrotePdu();
						if (replacePduData)
						{
							__tc__("replaceSm.replaceRepeatedDeliveryEnrote");
							fixture->transmitter->setupRandomCorrectReplaceSmPdu(pdu,
								replacePduData);
							pdu->set_messageId(replacePduData->smsId.c_str());
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendReplaceSmPdu(pdu, replacePduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

int SmppProtocolTestCases::getRandomRespDelay()
{
	__cfg_int__(timeCheckAccuracy);
	return 1000 * rand0(max(0, (int) fixture->smeInfo.timeout - timeCheckAccuracy));
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDeliverySmRespOk(
	PduDeliverySm& pdu, bool sync)
{
	__decl_tc__;
	int delay = 0; //getRandomRespDelay();
	time_t sendTime = time(NULL) + delay / 1000;
	//__tc__("sendDeliverySmResp.delay"); __tc_ok__;
	__trace2__("sendDeliverySmRespOk(): delay = %d", delay);
	try
	{
		__tc__("sendDeliverySmResp.sendOk");
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		respPdu.get_header().set_commandStatus(ESME_ROK); //No Error
		fixture->transmitter->sendDeliverySmResp(respPdu, sync, delay);
		__tc_ok__;
		return make_pair(ESME_ROK, sendTime);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
	return make_pair(0xffffffff, time(NULL));
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDeliverySmRespRetry(
	PduDeliverySm& pdu, bool sync, int num)
{
	__trace__("sendDeliverySmRespRetry()");
	TCSelector s(num, 5);
	__decl_tc__;
	try
	{
		uint32_t commandStatus = ESME_ROK;
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		respPdu.get_header().set_commandStatus(ESME_ROK);
		switch (s.value())
		{
			case 1: //не отправлять респонс
				__tc__("sendDeliverySmResp.sendRetry.notSend");
				commandStatus = 0xffffffff;
				break;
			case 2: //временная ошибка на стороне sme, запрос на повторную доставку
				__tc__("sendDeliverySmResp.sendRetry.tempAppError");
				commandStatus = ESME_RX_T_APPN;
				respPdu.get_header().set_commandStatus(commandStatus);
				fixture->transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			case 3: //переполнение очереди стороне sme
				__tc__("sendDeliverySmResp.sendRetry.msgQueueFull");
				commandStatus = ESME_RMSGQFUL;
				respPdu.get_header().set_commandStatus(commandStatus);
				fixture->transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			case 4: //отправить респонс с неправильным sequence_number
				__tc__("sendDeliverySmResp.sendRetry.invalidSequenceNumber");
				commandStatus = 0xffffffff;
				respPdu.get_header().set_sequenceNumber(INT_MAX);
				respPdu.get_header().set_commandStatus(ESME_ROK);
				fixture->transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			case 5: //отправить респонс после sme timeout
				{
					__tc__("sendDeliverySmResp.sendRetry.sendAfterSmeTimeout");
					commandStatus = 0xffffffff;
					respPdu.get_header().set_commandStatus(ESME_ROK);
					__cfg_int__(sequentialPduInterval);
					int timeout = 1000 * (fixture->smeInfo.timeout + rand2(1, sequentialPduInterval));
					__trace2__("sendAfterSmeTimeout(): sme timeout = %d, timeout = %d",
						fixture->smeInfo.timeout, timeout);
					fixture->transmitter->sendDeliverySmResp(respPdu, sync, timeout);
				}
				break;
			default:
				__unreachable__("Invalid num");
		}
		__tc_ok__;
		return make_pair(commandStatus, time(NULL));
	}
	catch(...)
	{
		__tc_fail__(s.value());
		error();
	}
	return make_pair(0xffffffff, time(NULL));
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDeliverySmRespError(
	PduDeliverySm& pdu, bool sync, int num)
{
	__decl_tc__;
	int delay = 0; //getRandomRespDelay();
	time_t sendTime = time(NULL) + delay / 1000;
	//__tc__("sendDeliverySmResp.delay"); __tc_ok__;
	__trace2__("sendDeliverySmRespError(): delay = %d", delay);
	TCSelector s(num, 3);
	try
	{
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		switch (s.value())
		{
			case 1: //отправить респонс с кодом ошибки 0x1-0x10f
				__tc__("sendDeliverySmResp.sendError.standardError");
				respPdu.get_header().set_commandStatus(rand1(0x10f));
				break;
			case 2: //отправить респонс с кодом ошибки:
				//0x110-0x3ff - Reserved for SMPP extension
				//0x400-0x4ff - Reserved for SMSC vendor specific
				__tc__("sendDeliverySmResp.sendError.reservedError");
				respPdu.get_header().set_commandStatus(rand2(0x110, 0x4ff));
				break;
			case 3: //отправить респонс с кодом ошибки >0x500 - Reserved
				__tc__("sendDeliverySmResp.sendError.outRangeError");
				respPdu.get_header().set_commandStatus(rand2(0x500, INT_MAX));
				break;
			/*
			case 4: //неустранимая ошибка на стороне sme, отказ от всех последующих сообщений
				__tc__("sendDeliverySmResp.sendError.permanentAppError");
				respPdu.get_header().set_commandStatus(ESME_RX_P_APPN);
				break;
			*/
			default:
				__unreachable__("Invalid num");
		}
		//рекурсивный вызов, чтобы предотвратить закрытие smpp сессии
		if (respPdu.get_header().get_commandStatus() == ESME_RX_P_APPN)
		{
			return sendDeliverySmRespError(pdu, sync, num);
		}
		fixture->transmitter->sendDeliverySmResp(respPdu, sync, delay);
		__tc_ok__;
		return make_pair(respPdu.get_header().get_commandStatus(), sendTime);
	}
	catch(...)
	{
		__tc_fail__(s.value());
		error();
	}
	return make_pair(0xffffffff, time(NULL));
}

}
}
}

