#include "SmppProtocolTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "system/status.h"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::test::conf::TestConfig;
using smsc::test::sms::operator<;
using smsc::test::sms::operator==;
using smsc::test::smpp::operator==;
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::smpp::DataCoding;
using namespace smsc::system;
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
		__trace2__("submitSmAssert(%d)", s.value());
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
						int len = MAX_SMPP_SM_LENGTH + 1;
						auto_ptr<char> msg = rand_char(len);
						pdu.get_message().set_shortMessage(msg.get(), len);
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			__dumpPdu__("submitSmAssert", fixture->smeInfo.systemId,
				reinterpret_cast<SmppHeader*>(&pdu));
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
	__cfg_int__(timeCheckAccuracy);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		time(NULL) + timeCheckAccuracy + 3, LONG_MAX);
	vector<PduData*> res;
	while (PduMonitor* m = it->next())
	{
		//общие проверки
		if (m->getType() != DELIVERY_MONITOR || m->pduData->replacedByPdu ||
			m->pduData->replacePdu || m->getFlag() != PDU_REQUIRED_FLAG ||
			m->pduData->intProps.count("hasSmppDuplicates") ||
			!m->pduData->strProps.count("smsId"))
		{
			continue;
		}
		//чтобы избежать пересечения с порожденными delivery reports
		uint8_t regDelivery = SmppTransmitterTestCases::getRegisteredDelivery(m->pduData);
		DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
		if (regDelivery != NO_SMSC_DELIVERY_RECEIPT &&
			(abs(monitor->getLastTime() - time(NULL)) <= timeCheckAccuracy ||
			abs(monitor->getValidTime() - time(NULL)) <= timeCheckAccuracy))
		{
			continue;
		}
		res.push_back(m->pduData);
	}
	delete it;
	return res.size() ? res[rand0(res.size() - 1)] : NULL;
}

PduData* SmppProtocolTestCases::getReplaceEnrotePdu()
{
	__require__(fixture->pduReg);
	__cfg_int__(timeCheckAccuracy);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		time(NULL) + timeCheckAccuracy + 3, LONG_MAX);
	vector<PduData*> res;
	while (PduMonitor* m = it->next())
	{
		//общие проверки
		if (m->getType() != DELIVERY_MONITOR || m->pduData->replacedByPdu ||
			!m->pduData->replacePdu || m->getFlag() != PDU_REQUIRED_FLAG ||
			m->pduData->intProps.count("hasSmppDuplicates") ||
			!m->pduData->strProps.count("smsId"))
		{
			continue;
		}
		//чтобы избежать пересечения с порожденными delivery reports
		uint8_t regDelivery = SmppTransmitterTestCases::getRegisteredDelivery(m->pduData);
		DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
		if (regDelivery != NO_SMSC_DELIVERY_RECEIPT &&
			(abs(monitor->getLastTime() - time(NULL)) <= timeCheckAccuracy ||
			abs(monitor->getValidTime() - time(NULL)) <= timeCheckAccuracy))
		{
			continue;
		}
		res.push_back(m->pduData);
	}
	delete it;
	return res.size() ? res[rand0(res.size() - 1)] : NULL;
}

PduData* SmppProtocolTestCases::getNonReplaceRescheduledEnrotePdu()
{
	__require__(fixture->pduReg);
	__cfg_int__(timeCheckAccuracy);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		time(NULL) + timeCheckAccuracy + 3, LONG_MAX);
	vector<PduData*> res;
	while (PduMonitor* m = it->next())
	{
		//общие проверки
		if (m->getType() != DELIVERY_MONITOR || m->pduData->replacedByPdu ||
			m->pduData->replacePdu || m->getFlag() != PDU_REQUIRED_FLAG ||
			m->pduData->intProps.count("hasSmppDuplicates") ||
			!m->pduData->strProps.count("smsId"))
		{
			continue;
		}
		//чтобы избежать пересечения с порожденными delivery reports
		uint8_t regDelivery = SmppTransmitterTestCases::getRegisteredDelivery(m->pduData);
		DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
		if (regDelivery != NO_SMSC_DELIVERY_RECEIPT &&
			(abs(monitor->getLastTime() - time(NULL)) <= timeCheckAccuracy ||
			abs(monitor->getValidTime() - time(NULL)) <= timeCheckAccuracy))
		{
			continue;
		}
		//была попытка доставки
		if (!monitor->getLastTime())
		{
			continue;
		}
		res.push_back(m->pduData);
	}
	delete it;
	return res.size() ? res[rand0(res.size() - 1)] : NULL;
}

PduData* SmppProtocolTestCases::getFinalPdu()
{
	__require__(fixture->pduReg);
	__cfg_int__(timeCheckAccuracy);
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(
		0, time(NULL) - timeCheckAccuracy);
	vector<PduData*> res;
	while (PduMonitor* m = it->next())
	{
		//общие проверки
		if (m->getType() != DELIVERY_MONITOR || m->pduData->replacedByPdu ||
			/* m->pduData->replacePdu || */ m->getFlag() != PDU_NOT_EXPECTED_FLAG ||
			m->pduData->intProps.count("hasSmppDuplicates") ||
			!m->pduData->strProps.count("smsId"))
		{
			continue;
		}
		//чтобы избежать пересечения с порожденными delivery reports
		uint8_t regDelivery = SmppTransmitterTestCases::getRegisteredDelivery(m->pduData);
		DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
		if (regDelivery != NO_SMSC_DELIVERY_RECEIPT &&
			(abs(monitor->getLastTime() - time(NULL)) <= timeCheckAccuracy ||
			abs(monitor->getValidTime() - time(NULL)) <= timeCheckAccuracy))
		{
			continue;
		}
		//беру заведомо хорошие sms
		if (monitor->state != SMPP_DELIVERED_STATE)
		{
			continue;
		}
		res.push_back(m->pduData);
	}
	delete it;
	return res.size() ? res[rand0(res.size() - 1)] : NULL;
}

PduData* SmppProtocolTestCases::getPduByState(SmppState state)
{
	__require__(fixture->pduReg);
	__cfg_int__(timeCheckAccuracy);
	PduFlag flag;
	PduRegistry::PduMonitorIterator* it;
	if (state == SMPP_ENROUTE_STATE)
	{
		flag = PDU_REQUIRED_FLAG;
		it = fixture->pduReg->getMonitors(time(NULL) + timeCheckAccuracy + 3, LONG_MAX);
	}
	else
	{
		flag = PDU_NOT_EXPECTED_FLAG;
		it = fixture->pduReg->getMonitors(0, time(NULL) - timeCheckAccuracy);
	}
	vector<PduData*> res;
	while (PduMonitor* m = it->next())
	{
		if (m->getType() != DELIVERY_MONITOR || m->pduData->replacedByPdu ||
			/* m->pduData->replacePdu || */ m->getFlag() != flag ||
			m->pduData->intProps.count("hasSmppDuplicates") ||
			!m->pduData->strProps.count("smsId"))
		{
			continue;
		}
		//чтобы избежать пересечения с порожденными delivery reports
		uint8_t regDelivery = SmppTransmitterTestCases::getRegisteredDelivery(m->pduData);
		DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
		if (regDelivery != NO_SMSC_DELIVERY_RECEIPT &&
			(abs(monitor->getLastTime() - time(NULL)) <= timeCheckAccuracy ||
			abs(monitor->getValidTime() - time(NULL)) <= timeCheckAccuracy))
		{
			continue;
		}
		if (monitor->state != state)
		{
			continue;
		}
		res.push_back(monitor->pduData);
	}
	delete it;
	return res.size() ? res[rand0(res.size() - 1)] : NULL;
}

void SmppProtocolTestCases::submitSmCorrect(bool sync, int num)
{
	TCSelector s(num, 11);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("submitSm.correct");
	for (; s.check(); s++)
	{
		__trace2__("submitSmCorrect(%d)", s.value());
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			const Address* destAlias = fixture->smeReg->getRandomAddress();
			__require__(destAlias);
			fixture->transmitter->setupRandomCorrectSubmitSmPdu(
				pdu, *destAlias, rand0(1));
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //ничего особенного
					//__tc__("submitSm.correct");
					break;
				case 2: //пустой serviceType
					__tc__("submitSm.correct.serviceTypeMarginal");
					//pdu->get_message().set_serviceType(NULL);
					pdu->get_message().set_serviceType("");
					pdu->get_message().set_replaceIfPresentFlag(0);
					intProps["hasSmppDuplicates"] = 1;
					break;
				case 3: //serviceType максимальной длины
					{
						__tc__("submitSm.correct.serviceTypeMarginal");
						EService serviceType;
						rand_char(MAX_SERVICE_TYPE_LENGTH, serviceType);
						pdu->get_message().set_serviceType(serviceType);
					}
					break;
				case 4: //доставка уже должна была начаться
					{
						__tc__("submitSm.correct.waitTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(maxWaitTime), t, time(NULL), __absoluteTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						//отменить подтверждения доставки и нотификации
						//pdu->get_message().set_registredDelivery(0);
					}
					break;
				case 5: //срок валидности больше максимального
					{
						__tc__("submitSm.correct.validTimeExceeded");
						SmppTime t;
						SmppUtil::time2string(time(NULL) + maxValidPeriod * 10,
							t, time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 6: //немедленная доставка
					__tc__("submitSm.correct.waitTimeImmediate");
					pdu->get_message().set_scheduleDeliveryTime("");
					break;
				case 7: //срок валидности по умолчанию
					__tc__("submitSm.correct.validTimeDefault");
					pdu->get_message().set_validityPeriod("");
					break;
				case 8: //пустое тело сообщения
					if (!pdu->get_optional().has_messagePayload())
					{
						__tc__("submitSm.correct.smLengthMarginal");
						pdu->get_message().set_shortMessage(NULL, 0);
						pdu->get_message().set_esmClass(
							pdu->get_message().get_esmClass() & ~ESM_CLASS_UDHI_INDICATOR);
						//pdu->get_message().set_shortMessage("", 0);
					}
					break;
				case 9: //shortMessage максимальной длины
					if (!pdu->get_optional().has_messagePayload())
					{
						__tc__("submitSm.correct.smLengthMarginal");
						int len = MAX_SMPP_SM_LENGTH;
						bool udhi = pdu->get_message().get_esmClass() &
							ESM_CLASS_UDHI_INDICATOR;
						uint8_t dc = pdu->get_message().get_dataCoding();
						if (fixture->smeInfo.forceDC)
						{
							bool simMsg;
							bool res = SmppUtil::extractDataCoding(dc, dc, simMsg);
							__require__(res);
						}
						auto_ptr<char> tmp = rand_text2(len, dc, udhi, false);
						pdu->get_message().set_shortMessage(tmp.get(), len);
					}
					break;
				case 10: //messagePayload максимальной длины (заодно будет отсекаться на map proxy)
					if (!pdu->get_message().size_shortMessage())
					{
						uint8_t dc = pdu->get_message().get_dataCoding();
						if (fixture->smeInfo.forceDC)
						{
							bool simMsg;
							bool res = SmppUtil::extractDataCoding(dc, dc, simMsg);
							__require__(res);
						}
						if (dc != UCS2) //иначе после транслитерации может пройти нарезка на MAP_PROXY: 37268 / 153 = 214 сегментов
						{
							__tc__("submitSm.correct.messagePayloadLengthMarginal");
							int len = MAX_PAYLOAD_LENGTH;
							bool udhi = pdu->get_message().get_esmClass() &
								ESM_CLASS_UDHI_INDICATOR;
							auto_ptr<char> tmp = rand_text2(len, dc, udhi, false);
							pdu->get_optional().set_messagePayload(tmp.get(), len);
						}
					}
					break;
				case 11: //ussd запрос
					if ((pdu->get_message().size_shortMessage() > 0 &&
							pdu->get_message().size_shortMessage() < MAX_MAP_SM_LENGTH) ||
						(pdu->get_optional().has_messagePayload() &&
						 pdu->get_optional().size_messagePayload() < MAX_MAP_SM_LENGTH))
					{
						__tc__("submitSm.correct.ussdRequest");
						pdu->get_optional().set_ussdServiceOp(rand0(255));
						//немедленная доставка, ussd не решедулится
						pdu->get_message().set_scheduleDeliveryTime("");
						//pdu->get_message().set_validityPeriod("");
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync, &intProps);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProtocolTestCases::submitSmCorrectComplex(bool sync, int num)
{
	TCSelector s(num, 7);
	__decl_tc__;
	__tc__("submitSm.correct");
	for (; s.check(); s++)
	{
		__trace2__("submitSmCorrectComplex(%d)", s.value());
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			const Address* destAlias = fixture->smeReg->getRandomAddress();
			__require__(destAlias);
			fixture->transmitter->setupRandomCorrectSubmitSmPdu(
				pdu, *destAlias, rand0(5));
			PduData* existentPduData = NULL;
			PduData::IntProps intProps;
			switch (s.value())
			{
				/*
				case 1: //msgRef одинаковые (эквивалентно msgRef отсутствуют)
					//Согласно GSM 03.40 пункт 9.2.3.25 если совпадают
					//TP-MR, TP-DA, OA, то при ETSI_REJECT_IF_PRESENT будет ошибка.
					//Для SMPP все должно работать независимо от msgRef.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu(true);
						if (existentPduData)
						{
							__tc__("submitSm.correct.checkMap");
							SmsPduWrapper existentPdu(existentPduData->pdu, 0);
							__require__(pdu->get_message().get_source() ==
								existentPdu.getSource());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								existentPdu.getDest());
							pdu->get_optional().set_userMessageReference(
								existentPdu.getMsgRef());
						}
					}
					break;
				*/
				case 1: //отправка дублированного сообщения без замещения уже существующего
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__tc__("submitSm.correct.notReplace");
							SmsPduWrapper existentPdu(existentPduData);
							pdu->get_message().set_serviceType(
								nvl(existentPdu.getServiceType()));
							__require__(pdu->get_message().get_source() ==
								existentPdu.getSource());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(existentPdu.getDest());
							pdu->get_message().set_replaceIfPresentFlag(0);
							intProps["hasSmppDuplicates"] = 1;
							existentPduData->intProps["hasSmppDuplicates"] = 1;
						}
					}
					break;
				case 2: //отправка дублированного сообщения с попыткой замещения
					//уже существующего, но service_type не совпадает.
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__tc__("submitSm.correct.serviceTypeNotMatch");
							SmsPduWrapper existentPdu(existentPduData);
							__require__(strcmp(nvl(pdu->get_message().get_serviceType()),
								nvl(existentPdu.getServiceType())));
							//pdu->get_message().set_serviceType(...);
							__require__(pdu->get_message().get_source() ==
								existentPdu.getSource());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(existentPdu.getDest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 3: //отправка дублированного сообщения с попыткой замещения
					//уже существующего, но dest_addr не совпадает.
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							SmsPduWrapper existentPdu(existentPduData);
							if (pdu->get_message().get_dest() != existentPdu.getDest())
							{
								__tc__("submitSm.correct.destAddrNotMatch");
								pdu->get_message().set_serviceType(
									nvl(existentPdu.getServiceType()));
								__require__(pdu->get_message().get_source() ==
									existentPdu.getSource());
								//pdu->get_message().set_source(...);
								__require__(pdu->get_message().get_dest() !=
									existentPdu.getDest());
								//pdu->get_message().set_dest(...);
								pdu->get_message().set_replaceIfPresentFlag(1);
							}
						}
					}
					break;
				case 4: //отправка дублированного сообщения с замещением уже существующего
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__tc__("submitSm.correct.replaceEnrote");
							SmsPduWrapper replacePdu(existentPduData);
							pdu->get_message().set_serviceType(
								nvl(replacePdu.getServiceType()));
							__require__(pdu->get_message().get_source() ==
								replacePdu.getSource());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu.getDest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 5: //отправка дублированного сообщения с замещением уже
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
							__tc__("submitSm.correct.replaceReplacedEnrote");
							SmsPduWrapper replacePdu(existentPduData);
							pdu->get_message().set_serviceType(
								nvl(replacePdu.getServiceType()));
							__require__(pdu->get_message().get_source() ==
								replacePdu.getSource());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(replacePdu.getDest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 6: //отправка дублированного сообщения с замещением уже
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
							__tc__("submitSm.correct.replaceFinal");
							SmsPduWrapper finalPdu(finalPduData);
							pdu->get_message().set_serviceType(
								nvl(finalPdu.getServiceType()));
							__require__(pdu->get_message().get_source() ==
								finalPdu.getSource());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(finalPdu.getDest());
							pdu->get_message().set_replaceIfPresentFlag(1);
							intProps["hasSmppDuplicates"] = 1;
							finalPduData->intProps["hasSmppDuplicates"] = 1;
						}
					}
					break;
				case 7: //отправка дублированного сообщения с замещением уже
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
							__tc__("submitSm.correct.replaceRepeatedDeliveryEnrote");
							SmsPduWrapper replacePdu(existentPduData);
							pdu->get_message().set_serviceType(
								nvl(replacePdu.getServiceType()));
							__require__(pdu->get_message().get_source() ==
								replacePdu.getSource());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(replacePdu.getDest());
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
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProtocolTestCases::submitSmIncorrect(bool sync, int num)
{
	TCSelector s(num, 13 /*19*/);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(maxDeliveryPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("submitSm.incorrect");
	for (; s.check(); s++)
	{
		__trace2__("submitSmIncorrect(%d)", s.value());
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			const Address* destAlias = fixture->smeReg->getRandomAddress();
			__require__(destAlias);
			fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, *destAlias, rand0(5));
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
					/*
					if (fixture->smeInfo.forceDC)
					{
						__tc__("submitSm.incorrect.dataCodingForceDcSme");
						uint8_t dcs, dc;
						bool simMsg;
						do
						{
							dcs = rand0(255);
						}
						while (SmppUtil::extractDataCoding(dcs, dc, simMsg));
						pdu->get_message().set_dataCoding(dcs);
					}
					else
					*/
					if (!fixture->smeInfo.forceDC)
					{
						__tc__("submitSm.incorrect.dataCodingNormalSme");
						while (true)
						{
							uint8_t dc = rand1(255);
							switch (dc)
							{
								case DEFAULT:
								case BINARY:
								case UCS2:
								case SMSC7BIT:
									continue;
							}
							pdu->get_message().set_dataCoding(dc);
							break;
						}
					}
					break;
				case 12: //заданы оба short_message И message_payload
					__tc__("submitSm.incorrect.bothMessageFields");
					if (!pdu->get_message().size_shortMessage())
					{
						int len = rand2(3, MAX_SMPP_SM_LENGTH);
						bool udhi = pdu->get_message().get_esmClass() &
							ESM_CLASS_UDHI_INDICATOR;
						uint8_t dc = pdu->get_message().get_dataCoding();
						if (fixture->smeInfo.forceDC)
						{
							bool simMsg;
							bool res = SmppUtil::extractDataCoding(dc, dc, simMsg);
							__require__(res);
						}
						auto_ptr<char> tmp = rand_text2(len, dc, udhi, false);
						pdu->get_message().set_shortMessage(tmp.get(), len);
					}
					if (!pdu->get_optional().has_messagePayload())
					{
						int len = rand2(3, MAX_PAYLOAD_LENGTH);
						bool udhi = pdu->get_message().get_esmClass() &
							ESM_CLASS_UDHI_INDICATOR;
						uint8_t dc = pdu->get_message().get_dataCoding();
						if (fixture->smeInfo.forceDC)
						{
							bool simMsg;
							bool res = SmppUtil::extractDataCoding(dc, dc, simMsg);
							__require__(res);
						}
						auto_ptr<char> tmp = rand_text2(len, dc, udhi, false);
						pdu->get_optional().set_messagePayload(tmp.get(), len);
					}
					break;
				case 13:
					__tc__("submitSm.incorrect.transactionRollback");
					pdu->get_message().set_serviceType("-----");
					break;
				/*
				case 13: //некорректная длина udh
					__tc__("submitSm.incorrect.udhiLength");
					pdu->get_message().set_esmClass(
						pdu->get_message().get_esmClass() | ESM_CLASS_UDHI_INDICATOR);
					{
						int len = rand1(5);
						char buf[len];
						*buf = (unsigned char) (len + rand1(10));
						pdu->get_message().set_shortMessage(buf, len);
					}
					break;
				case 13: //длина service_type больше допустимой
					{
						__tc__("submitSm.incorrect.serviceTypeLength");
						auto_ptr<char> tmp = rand_char(MAX_SERVICE_TYPE_LENGTH + 1);
						pdu->get_message().set_serviceType(tmp.get());
					}
					break;
				case 14: //длина source_addr больше допустимой
					{
						__tc__("submitSm.incorrect.sourceAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->get_message().set_source(addr);
					}
					break;
				case 15: //длина dest_addr больше допустимой
					{
						__tc__("submitSm.incorrect.destAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->get_message().set_dest(addr);
					}
					break;
				case 16: //длина validity_period больше допустимой
					{
						__tc__("submitSm.incorrect.validTimeLength");
						auto_ptr<char> tmp = rand_char(MAX_SMPP_TIME_LENGTH + 1);
						pdu->get_message().set_validityPeriod(tmp.get());
					}
					break;
				case 17: //длина schedule_delivery_time больше допустимой
					{
						__tc__("submitSm.incorrect.waitTimeLength");
						auto_ptr<char> tmp = rand_char(MAX_SMPP_TIME_LENGTH + 1);
						pdu->get_message().set_scheduleDeliveryTime(tmp.get());
					}
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProtocolTestCases::dataSmCorrect(bool sync, int num)
{
	TCSelector s(num, 8);
	__decl_tc__;
	__cfg_int__(maxValidPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("dataSm.correct");
	for (; s.check(); s++)
	{
		__trace2__("dataSmCorrect(%d)", s.value());
		try
		{
			PduDataSm* pdu = new PduDataSm();
			const Address* destAlias = fixture->smeReg->getRandomAddress();
			__require__(destAlias);
			fixture->transmitter->setupRandomCorrectDataSmPdu(pdu, *destAlias);
			PduData* existentPduData = NULL;
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //ничего особенного
					//__tc__("dataSm.correct");
					break;
				case 2: //пустой serviceType
					__tc__("dataSm.correct.serviceTypeMarginal");
					//pdu->get_data().set_serviceType(NULL);
					pdu->get_data().set_serviceType("");
					intProps["hasSmppDuplicates"] = 1;
					break;
				case 3: //serviceType максимальной длины
					{
						__tc__("dataSm.correct.serviceTypeMarginal");
						EService serviceType;
						rand_char(MAX_SERVICE_TYPE_LENGTH, serviceType);
						pdu->get_data().set_serviceType(serviceType);
					}
					break;
				case 4: //срок валидности больше максимального
					__tc__("dataSm.correct.validTimeExceeded");
					pdu->get_optional().set_qosTimeToLive(maxValidPeriod * 10);
					break;
				case 5: //пустой messagePayload
					__tc__("dataSm.correct.messagePayloadLengthMarginal");
					//pdu->get_optional().set_messagePayload(NULL, 0);
					pdu->get_optional().set_messagePayload("", 0);
					pdu->get_data().set_esmClass(
						pdu->get_data().get_esmClass() & ~ESM_CLASS_UDHI_INDICATOR);
					break;
				case 6: //messagePayload максимальной длины (заодно будет отсекаться на map proxy)
					{
						uint8_t dc = pdu->get_data().get_dataCoding();
						if (fixture->smeInfo.forceDC)
						{
							bool simMsg;
							bool res = SmppUtil::extractDataCoding(dc, dc, simMsg);
							__require__(res);
						}
						if (dc != UCS2) //иначе после транслитерации может пройти нарезка на MAP_PROXY: 37268 / 153 = 214 сегментов
						{
							__tc__("dataSm.correct.messagePayloadLengthMarginal");
							int len = MAX_SMPP_SM_LENGTH;
							bool udhi = pdu->get_data().get_esmClass() &
								ESM_CLASS_UDHI_INDICATOR;
							auto_ptr<char> tmp = rand_text2(len, dc, udhi, false);
							pdu->get_optional().set_messagePayload(tmp.get(), len);
						}
					}
					break;
				case 7: //ussd запрос
					if (pdu->get_optional().has_messagePayload() &&
						pdu->get_optional().size_messagePayload() < MAX_MAP_SM_LENGTH)
					{
						__tc__("dataSm.correct.ussdRequest");
						pdu->get_optional().set_ussdServiceOp(rand0(255));
					}
					break;
				case 8: //отправка дублированного сообщения без замещения уже существующего
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state. Но для data_sm нет replace_if_present
					if (fixture->pduReg)
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						existentPduData = getNonReplaceEnrotePdu();
						if (existentPduData)
						{
							__tc__("dataSm.correct.notReplace");
							SmsPduWrapper existentPdu(existentPduData);
							pdu->get_data().set_serviceType(
								nvl(existentPdu.getServiceType()));
							__require__(pdu->get_data().get_source() ==
								existentPdu.getSource());
							//pdu->get_data().set_source(...);
							pdu->get_data().set_dest(existentPdu.getDest());
							intProps["hasSmppDuplicates"] = 1;
							existentPduData->intProps["hasSmppDuplicates"] = 1;
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendDataSmPdu(pdu, existentPduData, sync, &intProps);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProtocolTestCases::dataSmIncorrect(bool sync, int num)
{
	TCSelector s(num, 4 /*8*/);
	__decl_tc__;
	__tc__("dataSm.incorrect");
	for (; s.check(); s++)
	{
		__trace2__("dataSmIncorrect(%d)", s.value());
		try
		{
			PduDataSm* pdu = new PduDataSm();
			const Address* destAlias = fixture->smeReg->getRandomAddress();
			__require__(destAlias);
			fixture->transmitter->setupRandomCorrectDataSmPdu(pdu, *destAlias);
			switch (s.value())
			{
				case 1: //неправильный адрес отправителя
					{
						__tc__("dataSm.incorrect.sourceAddr");
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->get_data().set_source(smppAddr);
					}
					break;
				case 2: //неправильный адрес получателя
					{
						__tc__("dataSm.incorrect.destAddr");
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->get_data().set_dest(smppAddr);
					}
					break;
				case 3: //недопустимый dataCoding
					/*
					if (fixture->smeInfo.forceDC)
					{
						__tc__("dataSm.incorrect.dataCodingForceDcSme");
						uint8_t dcs, dc;
						bool simMsg;
						do
						{
							dcs = rand0(255);
						}
						while (SmppUtil::extractDataCoding(dcs, dc, simMsg));
						pdu->get_data().set_dataCoding(dcs);
					}
					else
					*/
					if (!fixture->smeInfo.forceDC)
					{
						__tc__("dataSm.incorrect.dataCodingNormalSme");
						while (true)
						{
							uint8_t dc = rand1(255);
							switch (dc)
							{
								case DEFAULT:
								case BINARY:
								case UCS2:
								case SMSC7BIT:
									continue;
							}
							pdu->get_data().set_dataCoding(dc);
							break;
						}
					}
					break;
				case 4:
					__tc__("dataSm.incorrect.transactionRollback");
					pdu->get_data().set_serviceType("-----");
					break;
				/*
				case 5: //некорректная длина udh
					__tc__("dataSm.incorrect.udhiLength");
					pdu->get_data().set_esmClass(
						pdu->get_data().get_esmClass() | ESM_CLASS_UDHI_INDICATOR);
					{
						int len = rand1(5);
						char buf[len];
						*buf = (unsigned char) (len + rand1(10));
						pdu->get_optional().set_messagePayload(buf, len);
					}
					break;
				case 6: //длина service_type больше допустимой
					{
						__tc__("dataSm.incorrect.serviceTypeLength");
						auto_ptr<char> tmp = rand_char(MAX_SERVICE_TYPE_LENGTH + 1);
						pdu->get_data().set_serviceType(tmp.get());
					}
					break;
				case 7: //длина source_addr больше допустимой
					{
						__tc__("dataSm.incorrect.sourceAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->get_data().set_source(addr);
					}
					break;
				case 8: //длина dest_addr больше допустимой
					{
						__tc__("dataSm.incorrect.destAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->get_data().set_dest(addr);
					}
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendDataSmPdu(pdu, NULL, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

bool SmppProtocolTestCases::setDirective(SmppHeader* header, const string& dir,
	int& offset)
{
	__require__(offset >= 0);
	SmsPduWrapper pdu(header, 0);
	__trace2__("setDirective(): dc = %d, dir = '%s', offset = %d",
		(int) pdu.getDataCoding(), dir.c_str(), offset);
	//только для текстовых сообщений
	switch (pdu.getDataCoding())
	{
		case DEFAULT:
		case UCS2:
		case SMSC7BIT:
			break;
		default:
			return false;
	}
	//текст сообщения
	char* text = NULL;
	int len = 0;
	bool udhi = pdu.getEsmClass() & ESM_CLASS_UDHI_INDICATOR;
	if (pdu.get_optional().has_messagePayload())
	{
		int udhLen = udhi ?
			1 + (unsigned char) *pdu.get_optional().get_messagePayload() : 0;
		text = const_cast<char*>(pdu.get_optional().get_messagePayload()) +
			udhLen + offset;
		len = pdu.get_optional().size_messagePayload() - udhLen - offset;
	}
	else if (pdu.isSubmitSm() && pdu.get_message().size_shortMessage())
	{
		int udhLen = udhi ?
			1 + (unsigned char) *pdu.get_message().get_shortMessage() : 0;
		text = const_cast<char*>(pdu.get_message().get_shortMessage()) +
			udhLen + offset;
		len = pdu.get_message().size_shortMessage() - udhLen - offset;
	}
	if (len <= 0)
	{
		return false;
	}
	//вставить директиву
	int dirLen;
	auto_ptr<char> dirEnc = encode(dir, pdu.getDataCoding(), dirLen, false);
	if (dirLen > len)
	{
		return false;
	}
	memcpy(text, dirEnc.get(), dirLen);
	offset += dirLen;
	return true;
}

bool SmppProtocolTestCases::correctAckDirectives(SmppHeader* header,
	PduData::IntProps& intProps, int num)
{
	__require__(intProps.count("directive.offset"));
	__decl_tc__;
	TCSelector s(num, 9);
	int& offset = intProps["directive.offset"];
	string dir;
	switch (s.value())
	{
		case 1: //нет директивы
			break;
		case 2: //ack директива
			dir = "#ack#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.ack"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.ack"] = 1;
				return true;
			}
			break;
		case 3: //ack директива
			dir = "#noack##ack#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.ack"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.ack"] = 1;
				return true;
			}
			break;
		case 4: //ack директива
			dir = "#ack##ack#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.ack"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.ack"] = 1;
				return true;
			}
			break;
		case 5: //noack директива
			dir = "#noack#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.noack"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.noack"] = 1;
				return true;
			}
			break;
		case 6: //noack директива
			dir = "#ack##noack#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.noack"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.noack"] = 1;
				return true;
			}
			break;
		case 7: //noack директива
			dir = "#noack##noack#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.noack"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.noack"] = 1;
				return true;
			}
			break;
		case 8: //ack директива с битой noack
			dir = "#ack#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.ack"); __tc_ok__;
				intProps["directive.ack"] = 1;
				dir = "noack#";
				int tmp = offset;
				setDirective(header, mixedCase(dir), offset);
				offset = tmp;
				return false;
			}
			break;
		case 9: //ack директива с битой noack
			dir = "#ack#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.ack"); __tc_ok__;
				intProps["directive.ack"] = 1;
				dir = " #noack#";
				int tmp = offset;
				setDirective(header, mixedCase(dir), offset);
				offset = tmp;
				return false;
			}
			break;
		default:
			__unreachable__("Invalid num for ack directive");
	}
	return false;
}

bool SmppProtocolTestCases::correctDefDirectives(SmppHeader* header,
	PduData::IntProps& intProps, int num)
{
	__require__(intProps.count("directive.offset"));
	__decl_tc__;
	TCSelector s(num, 4);
	int& offset = intProps["directive.offset"];
	string dir;
	switch (s.value())
	{
		case 1: //нет директивы
			break;
		case 2: //немедленная доставка
			dir = "#def 0#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.def"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.def"] = 0;
				return true;
			}
			break;
		case 3: //заведомо превышает maxValidPeriod
			dir = "#def 999#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.def"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.def"] = 999 * 3600;
				return true;
			}
			break;
		case 4: //превышает максимум 999 и maxValidPeriod
			dir = "#def 1000#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.def"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.def"] = 1000 * 3600;
				return true;
			}
			break;
		default:
			__unreachable__("Invalid num for def directive");
	}
	return false;
}

bool SmppProtocolTestCases::correctTemplateDirectives(SmppHeader* header,
	PduData::IntProps& intProps, PduData::StrProps& strProps, int num)
{
	__require__(intProps.count("directive.offset"));
	__decl_tc__;
	TCSelector s(num, 8);
	int& offset = intProps["directive.offset"];
	string dir;
	SmsPduWrapper pdu(header, 0);
	switch (s.value())
	{
		case 1: //нет директивы
			break;
		case 2: //t0, без параметров
			dir = "#template=t0#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.template"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				strProps["directive.template"] = "t0";
			}
			break;
		case 3: //t1, параметр задан
			{
				string name = pdu.getDataCoding() == UCS2 ? "Мир" : "World";
				ostringstream s;
				dir = "#template=t1# {name}=";
				s << mixedCase(dir) << name << " ";
				if (setDirective(header, s.str(), offset))
				{
					__tc__("directive.correct.template"); __tc_ok__;
					__tc__("directive.mixedCase"); __tc_ok__;
					strProps["directive.template"] = "t1";
					strProps["directive.template.t1.name"] = name;
				}
			}
			break;
		case 4: //t1, параметр в кавычках и содержит пробелы
			{
				string name = pdu.getDataCoding() == UCS2 ? "Мега мир" : "Super world";
				ostringstream s;
				dir = "#template=t1# {name}=";
				s << mixedCase(dir) << "\"" << name << "\" ";
				if (setDirective(header, s.str(), offset))
				{
					__tc__("directive.correct.template"); __tc_ok__;
					__tc__("directive.mixedCase"); __tc_ok__;
					strProps["directive.template"] = "t1";
					strProps["directive.template.t1.name"] = name;
				}
			}
			break;
		case 5: //t1, параметр содержит спец символы исключая "
			{
				string name = "~!@#$%^&*()-_=+\\|{[}];:',<.>/?";
				ostringstream s;
				dir = "#template=t1# {name}=";
				s << mixedCase(dir) << "\"" << name << "\" ";
				if (setDirective(header, s.str(), offset))
				{
					__tc__("directive.correct.template"); __tc_ok__;
					__tc__("directive.mixedCase"); __tc_ok__;
					strProps["directive.template"] = "t1";
					strProps["directive.template.t1.name"] = name;
				}
			}
			break;
		case 6: //t1, параметр по умолчанию
			dir = "#template=t1# {_name_}=111 ";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.template"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				strProps["directive.template"] = "t1";
			}
			break;
		case 7: //t2, параметры заданы
			{
				string name1 = pdu.getDataCoding() == UCS2 ? "Мир" : "World";
				string name2 = pdu.getDataCoding() == UCS2 ? "Мир 2" : "World 2";
				ostringstream s;
				dir = "#template=t2# {name1}=";
				s << mixedCase(dir) << name1;
				dir = " {name2}=";
				s << mixedCase(dir) << "\"" << name2 << "\" ";
				if (setDirective(header, s.str(), offset))
				{
					__tc__("directive.correct.template"); __tc_ok__;
					__tc__("directive.mixedCase"); __tc_ok__;
					strProps["directive.template"] = "t2";
					strProps["directive.template.t2.name1"] = name1;
					strProps["directive.template.t2.name2"] = name2;
				}
			}
			break;
		case 8: //t2, дефолтный параметр не задан
			{
				string name2 = pdu.getDataCoding() == UCS2 ? "Мир" : "World";
				ostringstream s;
				dir = "#template=t2# {name2}=";
				s << mixedCase(dir) << name2 << " ";
				if (setDirective(header, s.str(), offset))
				{
					__tc__("directive.correct.template"); __tc_ok__;
					__tc__("directive.mixedCase"); __tc_ok__;
					strProps["directive.template"] = "t2";
					strProps["directive.template.t2.name2"] = name2;
				}
			}
			break;
		default:
			__unreachable__("Invalid num for template directive");
	}
	return false; //после темплейта другие директивы запрещены
}

bool SmppProtocolTestCases::correctHideDirectives(SmppHeader* header,
	PduData::IntProps& intProps, int num)
{
	__require__(intProps.count("directive.offset"));
	__decl_tc__;
	TCSelector s(num, 7);
	int& offset = intProps["directive.offset"];
	string dir;
	switch (s.value())
	{
		case 1: //нет директивы
			break;
		case 2: //hide директива
			dir = "#hide#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.hide"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.hide"] = 1;
				return true;
			}
			break;
		case 3: //hide директива
			dir = "#unhide##hide#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.hide"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.hide"] = 1;
				return true;
			}
			break;
		case 4: //unhide директива
			dir = "#unhide#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.unhide"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.hide"] = 0;
				return true;
			}
			break;
		case 5: //unhide директива
			dir = "#hide##unhide#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.unhide"); __tc_ok__;
				__tc__("directive.mixedCase"); __tc_ok__;
				intProps["directive.hide"] = 0;
				return true;
			}
			break;
		case 6: //hide директива с битой unhide
			dir = "#hide#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.hide"); __tc_ok__;
				intProps["directive.hide"] = 1;
				dir = "unhide#";
				int tmp = offset;
				setDirective(header, mixedCase(dir), offset);
				offset = tmp;
				return false;
			}
			break;
		case 7: //unhide директива с битой hide
			dir = "#unhide#";
			if (setDirective(header, mixedCase(dir), offset))
			{
				__tc__("directive.correct.unhide"); __tc_ok__;
				intProps["directive.hide"] = 0;
				dir = " #hide#";
				int tmp = offset;
				setDirective(header, mixedCase(dir), offset);
				offset = tmp;
				return false;
			}
			break;
		default:
			__unreachable__("Invalid num for ack directive");
	}
	return false;
}

void SmppProtocolTestCases::correctDirectives(SmppHeader* header, PduData::IntProps& intProps,
	PduData::StrProps& strProps, const TestCaseId& num)
{
	__require__(num.size() == 5);
	//ack/noack директивы
	bool dirCont = correctAckDirectives(header, intProps, num[1]);
	//def директивы
	if (dirCont)
	{
		dirCont = correctDefDirectives(header, intProps, num[2]);
	}
	//template директивы
	if (dirCont)
	{
		dirCont = correctTemplateDirectives(header, intProps, strProps, num[3]);
	}
	//hide директивы
	if (dirCont)
	{
		dirCont = correctHideDirectives(header, intProps, num[4]);
	}
}

void SmppProtocolTestCases::correctDirectives(bool sync, const TestCaseId& num)
{
	__decl_tc__;
	PduData::IntProps intProps;
	PduData::StrProps strProps;
	intProps["directive.offset"] = 0;
	try
	{
		//тип pdu
		TCSelector s1(num[0], 2);
		const Address* destAlias = fixture->smeReg->getRandomAddress();
		__require__(destAlias);
		switch (s1.value())
		{
			case 1: //submit_sm
				{
					__tc__("directive.submitSm");
					PduSubmitSm* pdu = new PduSubmitSm();
					fixture->transmitter->setupRandomCorrectSubmitSmPdu(
						pdu, *destAlias, rand0(1));
					correctDirectives(reinterpret_cast<SmppHeader*>(pdu),
						intProps, strProps, num);
					//отправить и зарегистрировать pdu
					fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync, &intProps, &strProps);
				}
				break;
			case 2: //data_sm
				{
					__tc__("directive.dataSm");
					PduDataSm* pdu = new PduDataSm();
					fixture->transmitter->setupRandomCorrectDataSmPdu(pdu, *destAlias);
					correctDirectives(reinterpret_cast<SmppHeader*>(pdu),
						intProps, strProps, num);
					//отправить и зарегистрировать pdu
					fixture->transmitter->sendDataSmPdu(pdu, NULL, sync, &intProps, &strProps);
				}
				break;
			default:
				__unreachable__("Invalid num for pdu type");
		}
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

template<typename T, int size>
inline int sz(T (&)[size]) { return size; }

void SmppProtocolTestCases::incorrectDirectives(SmppHeader* header,
	PduData::IntProps& intProps, int num)
{
	__decl_tc__;
	const string invalidDir[] =
	{
		"##", "###", "####", "#aaa#",
		"#ac#", "#ack2#", "# ack#", "#ack #", "#ack=#",
		"#noac#", "#noack2#", "# noack#", "#noack #", "#noack=#",
		"#def#", "#de 999#", "#def2 999#",
		"#templat=t0#", "#template2=t0#", "#template#",
		"#hid#", "#unhid#", "#hide2#", "#unhide2#", "# hide#", "# unhide#",
		"#hide=#", "#unhide=#"
	};
	const string invalidDefDir[] =
	{
		"#def #", "#def abc#", "#def -1#"
	};
	const string invalidTemplateDir[] =
	{
		"#template=#", "#template=t#", "#template=t00#",
		"#template=t#{}=aaa", "#template=t#{name1}={name2}=",
		"#template=t2#" //не задан обязательный параметр
	};
	TCSelector s(num, 3);
	int offset = 0;
	switch (s.value())
	{
		case 1: //несуществующая директива
			if (setDirective(header, invalidDir[rand0(sz(invalidDir) - 1)], offset))
			{
				__tc__("directive.incorrect.invalidDir"); __tc_ok__;
			}
			break;
		case 2: //неправильные параметры для def директивы
			if (setDirective(header, invalidDefDir[rand0(sz(invalidDefDir) - 1)], offset))
			{
				__tc__("directive.incorrect.invalidDefDir"); __tc_ok__;
			}
			break;
		case 3: //неправильные параметры для template директивы
			if (setDirective(header, invalidTemplateDir[rand0(sz(invalidTemplateDir) - 1)], offset))
			{
				__tc__("directive.incorrect.invalidTemplateDir"); __tc_ok__;
				intProps["directive.invalidTemplateDir"] = 1;
			}
			break;
		default:
			__unreachable__("Invalid num for template directive");
	}
}

void SmppProtocolTestCases::incorrectDirectives(bool sync, const TestCaseId& num)
{
	__require__(num.size() == 2);
	__decl_tc__;
	PduData::IntProps intProps;
	try
	{
		//тип pdu
		TCSelector s1(num[0], 2);
		const Address* destAlias = fixture->smeReg->getRandomAddress();
		__require__(destAlias);
		switch (s1.value())
		{
			case 1: //submit_sm
				{
					__tc__("directive.submitSm");
					PduSubmitSm* pdu = new PduSubmitSm();
					fixture->transmitter->setupRandomCorrectSubmitSmPdu(
						pdu, *destAlias, rand0(1));
					incorrectDirectives(reinterpret_cast<SmppHeader*>(pdu), intProps, num[1]);
					//отправить и зарегистрировать pdu
					fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync, &intProps);
				}
				break;
			case 2: //data_sm
				{
					__tc__("directive.dataSm");
					PduDataSm* pdu = new PduDataSm();
					fixture->transmitter->setupRandomCorrectDataSmPdu(pdu, *destAlias);
					incorrectDirectives(reinterpret_cast<SmppHeader*>(pdu), intProps, num[1]);
					//отправить и зарегистрировать pdu
					fixture->transmitter->sendDataSmPdu(pdu, NULL, sync, &intProps);
				}
				break;
			default:
				__unreachable__("Invalid num for pdu type");
		}
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppProtocolTestCases::replaceSmIncorrect(PduReplaceSm* pdu, bool sync)
{
	__require__(pdu);
	__decl_tc__;
	try
	{
		__tc__("replaceSm.incorrect.messageId");
		SmppUtil::setupRandomCorrectReplaceSmPdu(pdu, DEFAULT, false);
		PduAddress srcAddr;
		SmppUtil::convert(fixture->smeAddr, &srcAddr);
		pdu->set_source(srcAddr);
		//отправить и зарегистрировать pdu
		fixture->transmitter->sendReplaceSmPdu(pdu, NULL, sync);
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppProtocolTestCases::replaceSmCorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	TCSelector s(num, 8);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(timeCheckAccuracy);
	__tc__("replaceSm.correct");
	for (; s.check(); s++)
	{
		__trace2__("replaceSmCorrect(%d)", s.value());
		try
		{
			PduReplaceSm* pdu = new PduReplaceSm();
			//выбрать случайный messageId из sms ожидающих доставки
			PduData* replacePduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				replacePduData = getNonReplaceEnrotePdu();
			}
			if (!replacePduData)
			{
				replaceSmIncorrect(pdu, sync);
				return;
			}
			fixture->transmitter->setupRandomCorrectReplaceSmPdu(pdu, replacePduData);
			//replaced pdu params
			SmsPduWrapper replacePdu(replacePduData);
			bool udhi = replacePdu.getEsmClass() & ESM_CLASS_UDHI_INDICATOR;
			uint8_t dc = replacePdu.getDataCoding();
			if (fixture->smeInfo.forceDC)
			{
				bool simMsg;
				bool res = SmppUtil::extractDataCoding(dc, dc, simMsg);
				__require__(res);
			}
			switch (s.value())
			{
				case 1: //ничего особенного
					//__tc__("replaceSm.correct");
					break;
				case 2: //доставка уже должна была начаться
					{
						__tc__("replaceSm.correct.waitTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(maxWaitTime), t, time(NULL), __absoluteTime__);
						pdu->set_scheduleDeliveryTime(t);
						//отменить подтверждения доставки и нотификации
						//pdu->set_registredDelivery(0);
					}
					break;
				case 3: //срок валидности больше максимального
					{
						__tc__("replaceSm.correct.validTimeExceeded");
						SmppTime t;
						SmppUtil::time2string(time(NULL) + maxValidPeriod * 10,
							t, time(NULL), __numTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 4: //время доставки без изменений
					if (replacePdu.getWaitTime() > time(NULL))
					{
						__tc__("replaceSm.correct.waitTimeNull");
						SmppTime t;
						SmppUtil::time2string(replacePdu.getWaitTime() + timeCheckAccuracy,
							t, time(NULL), __numTime__);
						pdu->set_scheduleDeliveryTime("");
						pdu->set_validityPeriod(t);
					}
					break;
				case 5: //срок валидности без изменений
					if (replacePdu.getValidTime() > time(NULL) + timeCheckAccuracy)
					{
						__tc__("replaceSm.correct.validTimeNull");
						SmppTime t;
						SmppUtil::time2string(replacePdu.getValidTime() - timeCheckAccuracy,
							t, time(NULL), __numTime__);
						pdu->set_scheduleDeliveryTime(t);
						pdu->set_validityPeriod("");
					}
					break;
				case 6: //пустое тело сообщения
					if (!udhi)
					{
						__tc__("replaceSm.correct.smLengthMarginal");
						pdu->set_shortMessage(NULL, 0);
						//pdu->set_shortMessage("", 0);
					}
					break;
				case 7: //тело сообщения максимальной длины
					{
						__tc__("replaceSm.correct.smLengthMarginal");
						int len = MAX_SMPP_SM_LENGTH;
						auto_ptr<char> tmp = rand_text2(len, dc, udhi, false);
						pdu->set_shortMessage(tmp.get(), len);
					}
					break;
				case 8: //замещение уже ранее замещенного сообщения
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						replacePduData = getReplaceEnrotePdu();
						if (replacePduData)
						{
							__tc__("replaceSm.correct.replaceReplacedEnrote");
							fixture->transmitter->setupRandomCorrectReplaceSmPdu(
								pdu, replacePduData);
						}
					}
					break;
				/*
				//время следующей доставки в этом случае расчитывается не от
				//schedule_delivery_time, а от нового вычисленного nextTryTime
				case 9: //замещение существующего сообщения, но находящегося
					//уже в процессе повторной доставки.
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						replacePduData = getNonReplaceRescheduledEnrotePdu();
						if (replacePduData)
						{
							__tc__("replaceSm.correct.replaceRepeatedDeliveryEnrote");
							fixture->transmitter->setupRandomCorrectReplaceSmPdu(
								pdu, replacePduData);
						}
					}
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			if (!replacePduData)
			{
				replaceSmIncorrect(pdu, sync);
				return;
			}
			fixture->transmitter->sendReplaceSmPdu(pdu, replacePduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProtocolTestCases::replaceSmIncorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	TCSelector s(num, 13 /*17*/);
	__decl_tc__;
	__cfg_int__(maxWaitTime);
	__cfg_int__(timeCheckAccuracy);
	__tc__("replaceSm.correct"); //correct пока не выполнились case
	for (; s.check(); s++)
	{
		__trace2__("replaceSmIncorrect(%d)", s.value());
		try
		{
			PduReplaceSm* pdu = new PduReplaceSm();
			//выбрать случайную sms ожидающую доставки
			PduData* replacePduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				replacePduData = getNonReplaceEnrotePdu();
			}
			if (!replacePduData)
			{
				replaceSmIncorrect(pdu, sync);
				return;
			}
			fixture->transmitter->setupRandomCorrectReplaceSmPdu(pdu, replacePduData);
			SmsPduWrapper replacePdu(replacePduData);
			switch (s.value())
			{
				case 1: //неправильный message_id
					{
						__tc__("replaceSm.incorrect.messageId");
						auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
						pdu->set_messageId(msgId.get());
					}
					break;
				case 2: //неправильный message_id
					{
						__tc__("replaceSm.incorrect.messageId");
						pdu->set_messageId("-1");
					}
					break;
				case 3: //неправильный message_id
					{
						__tc__("replaceSm.incorrect.messageId");
						pdu->set_messageId("368934881474191032320"); //2**65 * 10
					}
					break;
				case 4: //неправильный адрес отправителя
					{
						__tc__("replaceSm.incorrect.sourceAddr");
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_source(smppAddr);
					}
					break;
				case 5: //неправильный формат validity_period
					{
						__tc__("replaceSm.incorrect.validTimeFormat");
						string t = pdu->get_validityPeriod();
						t[rand0(MAX_SMPP_TIME_LENGTH - 1)] = 'a';
						pdu->set_validityPeriod(t.c_str());
					}
					break;
				case 6: //неправильный формат validity_period
					{
						__tc__("replaceSm.incorrect.validTimeFormat");
						string t = pdu->get_validityPeriod();
						t.erase(rand0(MAX_SMPP_TIME_LENGTH - 1), 1);
						pdu->set_validityPeriod(t.c_str());
					}
					break;
				case 7: //неправильный формат schedule_delivery_time
					{
						__tc__("replaceSm.incorrect.waitTimeFormat");
						string t = pdu->get_scheduleDeliveryTime();
						t[rand0(MAX_SMPP_TIME_LENGTH - 1)] = 'a';
						pdu->set_scheduleDeliveryTime(t.c_str());
					}
					break;
				case 8: //неправильный формат schedule_delivery_time
					{
						__tc__("replaceSm.incorrect.waitTimeFormat");
						string t = pdu->get_scheduleDeliveryTime();
						t.erase(rand0(MAX_SMPP_TIME_LENGTH - 1), 1);
						pdu->set_scheduleDeliveryTime(t.c_str());
					}
					break;
				case 9: //срок валидности уже закончился
					{
						__tc__("replaceSm.incorrect.validTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - timeCheckAccuracy, t, time(NULL), __absoluteTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 10: //waitTime > validTime
					{
						__tc__("replaceSm.incorrect.waitTimeInvalid1");
						SmppTime t;
						time_t validTime = time(NULL) + rand1(maxWaitTime);
						time_t waitTime = validTime + timeCheckAccuracy;
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 11: //waitTime > validTime
					{
						__tc__("replaceSm.incorrect.waitTimeInvalid2");
						SmppTime t;
						time_t newWaitTime = replacePdu.getValidTime() + timeCheckAccuracy;
						SmppUtil::time2string(newWaitTime, t, time(NULL), __numTime__);
						pdu->set_scheduleDeliveryTime(t);
						pdu->set_validityPeriod("");
					}
					break;
				case 12: //waitTime > validTime
					{
						__tc__("replaceSm.incorrect.waitTimeInvalid3");
						SmppTime t;
						time_t newValidTime = replacePdu.getWaitTime() - timeCheckAccuracy;
						SmppUtil::time2string(
							newValidTime, t, time(NULL), __absoluteTime__);
						pdu->set_scheduleDeliveryTime("");
						pdu->set_validityPeriod(t);
					}
					break;
				case 13: //замещением существующего сообщения, но находящегося
					//уже в финальном состоянии.
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						PduData* finalPduData = getFinalPdu();
						if (finalPduData)
						{
							__tc__("replaceSm.incorrect.replaceFinal");
							fixture->transmitter->setupRandomCorrectReplaceSmPdu(
								pdu, finalPduData);
						}
					}
					break;
				/*
				case 13: //некорректная длина udh
					if (replacePdu->get_message().get_esmClass() & ESM_CLASS_UDHI_INDICATOR)
					{
						__tc__("replaceSm.incorrect.udhiLength");
						int len = rand1(5);
						char buf[len];
						*buf = (unsigned char) (len + rand1(10));
						pdu->get_message().set_shortMessage(buf, len);
					}
					break;
				case 14: //длина message_id больше допустимой
					{
						__tc__("replaceSm.incorrect.messageIdLength");
						auto_ptr<char> tmp = rand_char(MAX_MSG_ID_LENGTH + 1);
						pdu->set_messageId(tmp.get());
					}
					break;
				case 15: //длина source_addr больше допустимой
					{
						__tc__("replaceSm.incorrect.sourceAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->set_source(addr);
					}
					break;
				case 16: //длина validity_period больше допустимой
					{
						__tc__("replaceSm.incorrect.validTimeLength");
						auto_ptr<char> tmp = rand_char(MAX_SMPP_TIME_LENGTH + 1);
						pdu->set_validityPeriod(tmp.get());
					}
					break;
				case 17: //длина schedule_delivery_time больше допустимой
					{
						__tc__("replaceSm.incorrect.waitTimeLength");
						auto_ptr<char> tmp = rand_char(MAX_SMPP_TIME_LENGTH + 1);
						pdu->set_scheduleDeliveryTime(tmp.get());
					}
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			__require__(replacePduData);
			fixture->transmitter->sendReplaceSmPdu(pdu, replacePduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProtocolTestCases::querySmCorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	__decl_tc__;
	TCSelector s(num, 5);
	__tc__("querySm.correct");
	for (; s.check(); s++)
	{
		__trace2__("querySmCorrect(%d)", s.value());
		try
		{
			PduQuerySm* pdu = new PduQuerySm();
			//выбрать случайный messageId из sms ожидающих доставки
			PduData* pduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				switch (s.value())
				{
					case 1:
						__tc__("querySm.correct.enroute");
						pduData = getPduByState(SMPP_ENROUTE_STATE);
						break;
					case 2:
						__tc__("querySm.correct.delivered");
						pduData = getPduByState(SMPP_DELIVERED_STATE);
						break;
					case 3:
						__tc__("querySm.correct.expired");
						pduData = getPduByState(SMPP_EXPIRED_STATE);
						break;
					case 4:
						__tc__("querySm.correct.undeliverable");
						pduData = getPduByState(SMPP_UNDELIVERABLE_STATE);
						break;
					case 5:
						__tc__("querySm.correct.deleted");
						pduData = getPduByState(SMPP_DELETED_STATE);
						break;
					default:
						__unreachable__("Invalid num state");
				}
			}
			PduAddress srcAddr;
			SmppUtil::convert(fixture->smeAddr, &srcAddr);
			pdu->set_source(srcAddr);
			if (pduData)
			{
				__require__(pduData->strProps.count("smsId"));
				pdu->set_messageId(pduData->strProps["smsId"].c_str());
			}
			else
			{
				__tc__("querySm.incorrect.messageId");
				auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
				pdu->set_messageId(msgId.get());
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendQuerySmPdu(pdu, pduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProtocolTestCases::querySmIncorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	__decl_tc__;
	int numState = 5; int numOther = 3 /*5*/;
	TCSelector s(num, numState * numOther);
	__tc__("querySm.incorrect");
	for (; s.check(); s++)
	{
		__trace2__("querySmIncorrect(%d)", s.value());
		try
		{
			PduQuerySm* pdu = new PduQuerySm();
			//выбрать случайный messageId из sms ожидающих доставки
			PduData* pduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				switch (s.value1(numState))
				{
					case 1:
						pduData = getPduByState(SMPP_ENROUTE_STATE);
						break;
					case 2:
						pduData = getPduByState(SMPP_DELIVERED_STATE);
						break;
					case 3:
						pduData = getPduByState(SMPP_EXPIRED_STATE);
						break;
					case 4:
						pduData = getPduByState(SMPP_UNDELIVERABLE_STATE);
						break;
					case 5:
						pduData = getPduByState(SMPP_DELETED_STATE);
						break;
					default:
						__unreachable__("Invalid num state");
				}
			}
			PduAddress srcAddr;
			SmppUtil::convert(fixture->smeAddr, &srcAddr);
			pdu->set_source(srcAddr);
			switch (s.value2(numState))
			{
				case 1: //неправильный messageId
					__tc__("querySm.incorrect.messageId");
					pdu->set_messageId("-1");
					break;
				case 2: //неправильный messageId
					__tc__("querySm.incorrect.messageId");
					pdu->set_messageId("36893488147419103232"); // 2**65
					break;
				case 3: //не совпадает srcAddr
					if (pduData)
					{
						__tc__("querySm.incorrect.sourceAddr");
						__require__(pduData->strProps.count("smsId"));
						pdu->set_messageId(pduData->strProps["smsId"].c_str());
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_source(smppAddr);
					}
					else
					{
						__tc__("querySm.incorrect.messageId");
						auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
						pdu->set_messageId(msgId.get());
					}
					break;
				/*
				case 4: //длина message_id больше допустимой
					{
						__tc__("querySm.incorrect.messageIdLength");
						auto_ptr<char> tmp = rand_char(MAX_MSG_ID_LENGTH + 1);
						pdu->set_messageId(tmp.get());
					}
					break;
				case 5: //длина source_addr больше допустимой
					{
						__tc__("querySm.incorrect.sourceAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->set_source(addr);
					}
					break;
				*/
				default:
					__unreachable__("Invalid num other");
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendQuerySmPdu(pdu, pduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

struct CancelKey
{
	const Address srcAddr;
	const Address destAddr;
	const string servType;

	CancelKey(const Address& _srcAddr, const Address& _destAddr,
		const string& _servType)
	: srcAddr(_srcAddr), destAddr(_destAddr), servType(_servType) {}
	bool operator< (const CancelKey& key) const
	{
		if (srcAddr < key.srcAddr) return true;
		if (srcAddr == key.srcAddr)
		{
			if (destAddr < key.destAddr) return true;
			if (destAddr == key.destAddr && servType < key.servType) return true;
		}
		return false;
	}
	const string str() const
	{
		ostringstream s;
		s << "srcAddr = " << srcAddr << ", destAddr = " << destAddr <<
			", servType = " << servType;
		return s.str();
	}
};

PduData* SmppProtocolTestCases::getCancelSmGroupParams(bool checkServType,
	Address& srcAddr, Address& destAddr, string& servType)
{
	typedef map<CancelKey, bool> CancelKeyMap;
	CancelKeyMap cancelMap;
	PduData* pduData = NULL;

	__require__(fixture->pduReg);
	__cfg_int__(timeCheckAccuracy);
	PduRegistry::PduMonitorIterator* it1 = fixture->pduReg->getMonitors(0, LONG_MAX);
	time_t checkTime = time(NULL) + timeCheckAccuracy + 3;
	__trace2__("getCancelSmGroupParams(): checkTime = %ld, checkServType = %s",
		checkTime, checkServType ? "true" : "false");
	//список "плохих" адресов для мониторов во всех состояниях
	while (PduMonitor* m = it1->next())
	{
		if (m->getType() != DELIVERY_MONITOR || m->pduData->replacedByPdu)
		{
			continue;
		}
		//чтобы избежать пересечения с порожденными delivery reports
		uint8_t regDelivery = SmppTransmitterTestCases::getRegisteredDelivery(m->pduData);
		DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
		if (monitor->getCheckTime() < checkTime ||
			(regDelivery != NO_SMSC_DELIVERY_RECEIPT &&
				(abs(monitor->getLastTime() - time(NULL)) <= timeCheckAccuracy ||
				abs(monitor->getValidTime() - time(NULL)) <= timeCheckAccuracy)))
		{
			CancelKey key(monitor->srcAddr, monitor->destAddr,
				checkServType ? monitor->serviceType : "");
			__trace2__("getCancelSmGroupParams(): bad monitor = %d, checkTime = %ld, key = %s",
				monitor->getId(), monitor->getCheckTime(), key.str().c_str());
			cancelMap[key] = false;
		}
	}
	delete it1;
	//поиск "хороших" адресов
	PduRegistry::PduMonitorIterator* it2 = fixture->pduReg->getMonitors(checkTime, LONG_MAX);
	while (PduMonitor* m = it2->next())
	{
		if (m->getType() != DELIVERY_MONITOR || m->pduData->replacedByPdu ||
			m->getFlag() != PDU_REQUIRED_FLAG || !m->pduData->strProps.count("smsId"))
		{
			continue;
		}
		DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
		CancelKey key(monitor->srcAddr, monitor->destAddr,
			checkServType ? monitor->serviceType : "");
		if (monitor->serviceType != "" && !cancelMap.count(key))
		{
			__trace2__("getCancelSmGroupParams(): good monitor = %d, checkTime = %ld, key = %s",
				monitor->getId(), monitor->getCheckTime(), key.str().c_str());
			srcAddr = key.srcAddr;
			destAddr = key.destAddr;
			servType = key.servType;
			pduData = monitor->pduData;
			break;
		}
	}
	delete it2;
	return pduData;
}

void SmppProtocolTestCases::cancelSmSingleCorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	__decl_tc__;
	TCSelector s(num, 2);
	__tc__("cancelSm.correct");
	for (; s.check(); s++)
	{
		__trace2__("cancelSmCorrect(%d)", s.value());
		try
		{
			PduCancelSm* pdu = new PduCancelSm();
			PduAddress srcAddr;
			SmppUtil::convert(fixture->smeAddr, &srcAddr);
			pdu->set_source(srcAddr);
			//выбрать случайную sms ожидающую доставки
			PduData* cancelPduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				cancelPduData = getNonReplaceEnrotePdu();
			}
			if (!cancelPduData)
			{
				__tc__("cancelSm.incorrect.messageId");
				auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
				pdu->set_messageId(msgId.get());
				//pdu->set_source();
				//pdu->set_dest();
				//pdu->set_serviceType();
				//отправить и зарегистрировать pdu
				fixture->transmitter->sendCancelSmPdu(pdu, NULL, sync);
				__tc_ok__;
				return;
			}
			__trace2__("cancelPduData = %p", cancelPduData);
			//canceld pdu params
			__require__(cancelPduData->strProps.count("smsId"));
			SmsPduWrapper cancelPdu(cancelPduData);
			__require__(cancelPdu.getSource() == srcAddr);
			switch (s.value())
			{
				case 1: //по message_id и source_addr с нулевыми service_type, dest_addr
					__tc__("cancelSm.correct.messageIdWithoutDestAddr");
					pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
					//pdu->set_source();
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 2: //по message_id, source_addr и dest_addr с нулевым service_type
					__tc__("cancelSm.correct.messageIdWithDestAddr");
					pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
					//pdu->set_source();
					pdu->set_dest(cancelPdu.getDest());
					//pdu->set_serviceType();
					break;
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendCancelSmPdu(pdu, cancelPduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProtocolTestCases::cancelSmGroupCorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	__decl_tc__;
	TCSelector s(num, 2);
	__tc__("cancelSm.correct");
	for (; s.check(); s++)
	{
		__trace2__("cancelSmGroupCorrect(%d)", s.value());
		try
		{
			PduCancelSm* pdu = new PduCancelSm();
			PduData* pduData = NULL;
			Address srcAddr, destAddr;
			string servType;
			switch (s.value())
			{
				case 1: //по source_addr и dest_addr с нулевым service_type
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						pduData = getCancelSmGroupParams(false, srcAddr, destAddr, servType);
						if (pduData)
						{
							__tc__("cancelSm.correct.destAddrWithoutServiceType");
							PduAddress addr;
							//pdu->set_messageId();
							pdu->set_source(*SmppUtil::convert(srcAddr, &addr));
							pdu->set_dest(*SmppUtil::convert(destAddr, &addr));
							//pdu->set_serviceType();
						}
					}
					break;
				case 2: //по source_addr, dest_addr и service_type
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						pduData = getCancelSmGroupParams(true, srcAddr, destAddr, servType);
						if (pduData)
						{
							__tc__("cancelSm.correct.destAddrWithServiceType");
							PduAddress addr;
							//pdu->set_messageId();
							pdu->set_source(*SmppUtil::convert(srcAddr, &addr));
							pdu->set_dest(*SmppUtil::convert(destAddr, &addr));
							pdu->set_serviceType(servType.c_str());
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			if (!pduData)
			{
				__tc__("cancelSm.incorrect.messageId");
				auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
				pdu->set_messageId(msgId.get());
				//pdu->set_source();
				//pdu->set_dest();
				//pdu->set_serviceType();
			}
			fixture->transmitter->sendCancelSmPdu(pdu, pduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProtocolTestCases::cancelSmIncorrect(bool sync, int num)
{
	__require__(fixture->pduReg);
	__decl_tc__;
	TCSelector s(num, 15 /*19*/);
	__tc__("cancelSm.incorrect");
	for (; s.check(); s++)
	{
		__trace2__("cancelSmIncorrect(%d)", s.value());
		try
		{
			static /* const */ PduAddress nullAddr;
			PduCancelSm* pdu = new PduCancelSm();
			PduAddress srcAddr;
			SmppUtil::convert(fixture->smeAddr, &srcAddr);
			pdu->set_source(srcAddr);
			//выбрать случайную sms ожидающую доставки
			PduData* cancelPduData = NULL;
			{
				MutexGuard mguard(fixture->pduReg->getMutex());
				cancelPduData = getNonReplaceEnrotePdu();
			}
			if (!cancelPduData)
			{
				__tc__("cancelSm.incorrect.messageId");
				auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
				pdu->set_messageId(msgId.get());
				//pdu->set_source();
				//pdu->set_dest();
				//pdu->set_serviceType();
				//отправить и зарегистрировать pdu
				fixture->transmitter->sendCancelSmPdu(pdu, NULL, sync);
				__tc_ok__;
				return;
			}
			//canceld pdu params
			__require__(cancelPduData->strProps.count("smsId"));
			SmsPduWrapper cancelPdu(cancelPduData);
			switch (s.value())
			{
				case 1: //неправильный message_id
					{
						__tc__("cancelSm.incorrect.messageId");
						auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
						pdu->set_messageId(msgId.get());
						//pdu->set_source();
						//pdu->set_dest();
						//pdu->set_serviceType();
					}
					break;
				case 2: //неправильный message_id
					__tc__("cancelSm.incorrect.messageId");
					pdu->set_messageId("-1");
					//pdu->set_source();
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 3: //неправильный message_id
					__tc__("cancelSm.incorrect.messageId");
					pdu->set_messageId("36893488147419103232"); //2**65
					//pdu->set_source();
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 4: //адрес отправителя не совпадает
					{
						__tc__("cancelSm.incorrect.sourceAddr");
						pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_source(smppAddr);
						//pdu->set_dest();
						//pdu->set_serviceType();
					}
					break;
				case 5: //адрес получателя не совпадает
					{
						__tc__("cancelSm.incorrect.destAddr");
						pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
						//pdu->set_source();
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_dest(smppAddr);
						//pdu->set_serviceType();
					}
					break;
				case 6: //задан только source_addr
					__tc__("cancelSm.incorrect.sourceAddrOnly");
					//pdu->set_messageId();
					//pdu->set_source();
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 7: //задан только dest_addr
					{
						__tc__("cancelSm.incorrect.destAddrOnly");
						//pdu->set_messageId();
						pdu->set_source(nullAddr);
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_dest(smppAddr);
						//pdu->set_serviceType();
					}
					break;
				case 8: //задан только message_id
					__tc__("cancelSm.incorrect.messageIdOnly");
					pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
					pdu->set_source(nullAddr);
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 9: //все поля пустые
					__tc__("cancelSm.incorrect.emptyFields");
					//pdu->set_messageId();
					pdu->set_source(nullAddr);
					//pdu->set_dest();
					//pdu->set_serviceType();
					break;
				case 10: //задан service_type без dest_addr
					{
						__tc__("cancelSm.incorrect.serviceTypeWithoutDestAddr");
						//pdu->set_messageId();
						//pdu->set_source();
						//pdu->set_dest();
						auto_ptr<char> tmp = rand_char(MAX_ESERVICE_TYPE_LENGTH);
						pdu->set_serviceType(tmp.get());
					}
					break;
				case 11: //заданы правильные message_id и service_type
					{
						__tc__("cancelSm.incorrect.messageIdWithServiceType");
						pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
						//pdu->set_source();
						//pdu->set_dest();
						pdu->set_serviceType(nvl(cancelPdu.getServiceType()));
					}
					break;
				case 12: //заданы правильные message_id, dest_addr и service_type
					{
						__tc__("cancelSm.incorrect.allFields");
						pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
						//pdu->set_source();
						pdu->set_dest(cancelPdu.getDest());
						pdu->set_serviceType(nvl(cancelPdu.getServiceType()));
					}
					break;
				case 13: //сообщение в финальном состоянии
					{
						MutexGuard mguard(fixture->pduReg->getMutex());
						cancelPduData = getFinalPdu();
						if (cancelPduData)
						{
							__tc__("cancelSm.incorrect.cancelFinal");
							pdu->set_messageId(cancelPduData->strProps["smsId"].c_str());
							//pdu->set_source();
							//pdu->set_dest();
							//pdu->set_serviceType();
						}
					}
					break;
				case 14: //группа sms-ок пустая
					{
						__tc__("cancelSm.incorrect.noSms");
						//pdu->set_messageId();
						//pdu->set_source();
						Address addr; PduAddress smppAddr;
						SmsUtil::setupRandomCorrectAddress(&addr);
						SmppUtil::convert(addr, &smppAddr);
						pdu->set_dest(smppAddr);
						//pdu->set_serviceType();
					}
					break;
				case 15: //группа sms-ок пустая
					{
						__tc__("cancelSm.incorrect.noSms");
						//pdu->set_messageId();
						//pdu->set_source();
						pdu->set_dest(cancelPdu.getDest());
						auto_ptr<char> tmp = rand_char(MAX_ESERVICE_TYPE_LENGTH);
						pdu->set_serviceType(tmp.get());
					}
					break;
				/*
				case 16: //длина message_id больше допустимой
					{
						__tc__("cancelSm.incorrect.messageIdLength");
						auto_ptr<char> tmp = rand_char(MAX_MSG_ID_LENGTH + 1);
						pdu->set_messageId(tmp.get());
					}
					break;
				case 17: //длина service_type больше допустимой
					{
						__tc__("cancelSm.incorrect.serviceTypeLength");
						auto_ptr<char> tmp = rand_char(MAX_SERVICE_TYPE_LENGTH + 1);
						pdu->set_serviceType(tmp.get());
					}
					break;
				case 18: //длина source_addr больше допустимой
					{
						__tc__("cancelSm.incorrect.sourceAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->set_source(addr);
					}
					break;
				case 19: //длина dest_addr больше допустимой
					{
						__tc__("cancelSm.incorrect.destAddrLength");
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH + 1);
						PduAddress addr;
						addr.set_value(tmp.get());
						pdu->set_dest(addr);
					}
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendCancelSmPdu(pdu, cancelPduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProtocolTestCases::sendInvalidPdu(bool sync, int num)
{
	__decl_tc__;
	TCSelector s(num, 14);
	for (; s.check(); s++)
	{
		try
		{
			SmppHeader* pdu;
			switch (s.value())
			{
				//реквесты
				case 1:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduQuerySm());
					pdu->set_commandId(QUERY_SM);
					break;
				case 2:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduDeliverySm());
					pdu->set_commandId(DELIVERY_SM);
					break;
				case 3:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduCancelSm());
					pdu->set_commandId(CANCEL_SM);
					break;
				case 4:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduOutBind());
					pdu->set_commandId(OUTBIND);
					break;
				case 5:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduMultiSm());
					pdu->set_commandId(SUBMIT_MULTI);
					break;
				case 6:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduAlertNotification());
					pdu->set_commandId(ALERT_NOTIFICATION);
					break;
				case 7:
					__tc__("sendInvalidPdu.request");
					pdu = reinterpret_cast<SmppHeader*>(new PduDataSm());
					pdu->set_commandId(DATA_SM);
					break;
				//респонсы
				case 8:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduBindTRXResp());
					pdu->set_commandId(BIND_RECIEVER_RESP);
					break;
				case 9:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduBindTRXResp());
					pdu->set_commandId(BIND_TRANSMITTER_RESP);
					break;
				case 10:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduBindTRXResp());
					pdu->set_commandId(BIND_TRANCIEVER_RESP);
					break;
				case 11:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduQuerySmResp());
					pdu->set_commandId(QUERY_SM_RESP);
					break;
				case 12:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduSubmitSmResp());
					pdu->set_commandId(SUBMIT_SM_RESP);
					break;
				case 13:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduCancelSmResp());
					pdu->set_commandId(CANCEL_SM_RESP);
					break;
				case 14:
					__tc__("sendInvalidPdu.response");
					pdu = reinterpret_cast<SmppHeader*>(new PduDataSmResp());
					pdu->set_commandId(DATA_SM_RESP);
					break;
				/*
				case 15:
					__tc__("sendInvalidPdu.response");
					{
						PduMultiSmResp* p = new PduMultiSmResp();
						p->set_sme(NULL);
						pdu = reinterpret_cast<SmppHeader*>(p);
					}
					pdu->set_commandId(SUBMIT_MULTI_RESP);
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			pdu->set_commandStatus(rand0(1));
			//отправить и зарегистрировать pdu
			fixture->transmitter->sendInvalidPdu(pdu, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
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
	SmppHeader* header, bool sync, bool sendDelay)
{
	__require__(header);
	__decl_tc__;
	int delay = 0;
	time_t sendTime = time(NULL);
	if (sendDelay)
	{
		__tc__("smsResp.deliverySm.delay"); __tc_ok__;
		delay = getRandomRespDelay();
		sendTime += delay / 1000;
	}
	__trace2__("sendDeliverySmRespOk(): delay = %d", delay);
	try
	{
		__tc__("smsResp.deliverySm.sendOk");
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(header->get_sequenceNumber());
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
	return make_pair(Status::DELIVERYTIMEDOUT,
		time(NULL) + fixture->smeInfo.timeout);
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDeliverySmRespRetry(
	SmppHeader* header, bool sync, int num)
{
	__trace2__("sendDeliverySmRespRetry(): sme timeout = %d", fixture->smeInfo.timeout);
	TCSelector s(num, 2);
	__decl_tc__;
	time_t sendTime = time(NULL);
	try
	{
		uint32_t commandStatus = ESME_ROK;
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(header->get_sequenceNumber());
		respPdu.get_header().set_commandStatus(ESME_ROK);
		switch (s.value())
		{
			case 1: //временная ошибка на стороне sme, запрос на повторную доставку
				__tc__("smsResp.deliverySm.sendRetry.tempAppError");
				commandStatus = ESME_RX_T_APPN;
				respPdu.get_header().set_commandStatus(commandStatus);
				fixture->transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			case 2: //переполнение очереди стороне sme
				__tc__("smsResp.deliverySm.sendRetry.msgQueueFull");
				commandStatus = ESME_RMSGQFUL;
				respPdu.get_header().set_commandStatus(commandStatus);
				fixture->transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			/*
			case 3: //отправить респонс с неправильным sequence_number
				__tc__("smsResp.deliverySm.sendRetry.invalidSequenceNumber");
				commandStatus = DELIVERY_STATUS_NO_RESPONSE;
				sendTime += fixture->smeInfo.timeout;
				respPdu.get_header().set_sequenceNumber(INT_MAX);
				respPdu.get_header().set_commandStatus(ESME_ROK);
				fixture->transmitter->sendDeliverySmResp(respPdu, sync);
				break;
			case 4: //не отправлять респонс
				__tc__("smsResp.deliverySm.sendRetry.notSend");
				commandStatus = DELIVERY_STATUS_NO_RESPONSE;
				sendTime += fixture->smeInfo.timeout;
				break;
			case 5: //отправить респонс после sme timeout
				{
					__tc__("smsResp.deliverySm.sendRetry.sendAfterSmeTimeout");
					commandStatus = DELIVERY_STATUS_NO_RESPONSE;
					sendTime += fixture->smeInfo.timeout;
					respPdu.get_header().set_commandStatus(ESME_ROK);
					int timeout = 1000 * (fixture->smeInfo.timeout + 1);
					__trace2__("sendAfterSmeTimeout(): timeout = %d", timeout);
					fixture->transmitter->sendDeliverySmResp(respPdu, sync, timeout);
				}
				break;
			*/
			default:
				__unreachable__("Invalid num");
		}
		__tc_ok__;
		return make_pair(commandStatus, sendTime);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
	return make_pair(Status::DELIVERYTIMEDOUT,
		time(NULL) + fixture->smeInfo.timeout);
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDeliverySmRespError(
	SmppHeader* header, bool sync, bool sendDelay, int num)
{
	__decl_tc__;
	int delay = 0;
	time_t sendTime = time(NULL);
	if (sendDelay)
	{
		__tc__("smsResp.deliverySm.delay"); __tc_ok__;
		delay = getRandomRespDelay();
		sendTime += delay / 1000;
	}
	__trace2__("sendDeliverySmRespError(): delay = %d", delay);
	TCSelector s(num, 3);
	try
	{
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(header->get_sequenceNumber());
		switch (s.value())
		{
			case 1: //отправить респонс с кодом ошибки 0x1-0x10f
				__tc__("smsResp.deliverySm.sendError.standardError");
				respPdu.get_header().set_commandStatus(rand1(0x10f));
				break;
			case 2: //отправить респонс с кодом ошибки:
				//0x110-0x3ff - Reserved for SMPP extension
				//0x400-0x4ff - Reserved for SMSC vendor specific
				__tc__("smsResp.deliverySm.sendError.reservedError");
				respPdu.get_header().set_commandStatus(rand2(0x110, 0x4ff));
				break;
			case 3: //отправить респонс с кодом ошибки >0x500 - Reserved
				__tc__("smsResp.deliverySm.sendError.outRangeError");
				respPdu.get_header().set_commandStatus(rand2(0x500, INT_MAX));
				break;
			/*
			case 4: //неустранимая ошибка на стороне sme, отказ от всех последующих сообщений
				__tc__("smsResp.deliverySm.sendError.permanentAppError");
				respPdu.get_header().set_commandStatus(ESME_RX_P_APPN);
				break;
			*/
			default:
				__unreachable__("Invalid num");
		}
		//рекурсивный вызов, чтобы предотвратить закрытие smpp сессии
		if (respPdu.get_header().get_commandStatus() == ESME_RX_P_APPN)
		{
			return sendDeliverySmRespError(header, sync, sendDelay, num);
		}
		fixture->transmitter->sendDeliverySmResp(respPdu, sync, delay);
		__tc_ok__;
		return make_pair(respPdu.get_header().get_commandStatus(), sendTime);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
	return make_pair(Status::DELIVERYTIMEDOUT,
		time(NULL) + fixture->smeInfo.timeout);
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDataSmRespOk(
	SmppHeader* header, bool sync, bool sendDelay)
{
	__decl_tc__;
	int delay = 0;
	time_t sendTime = time(NULL);
	if (sendDelay)
	{
		__tc__("smsResp.dataSm.delay"); __tc_ok__;
		delay = getRandomRespDelay();
		sendTime += delay / 1000;
	}
	__trace2__("sendDataSmRespOk(): delay = %d", delay);
	try
	{
		__tc__("smsResp.dataSm.sendOk");
		PduDataSmResp respPdu;
		respPdu.get_header().set_sequenceNumber(header->get_sequenceNumber());
		respPdu.get_header().set_commandStatus(ESME_ROK); //No Error
		auto_ptr<char> msgId = rand_char(rand0(MAX_MSG_ID_LENGTH));
		respPdu.set_messageId(msgId.get());
		SmppUtil::setupRandomCorrectOptionalParams(respPdu.get_optional(), BINARY, false);
		fixture->transmitter->sendDataSmResp(respPdu, sync, delay);
		__tc_ok__;
		return make_pair(ESME_ROK, sendTime);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
	return make_pair(Status::DELIVERYTIMEDOUT,
		time(NULL) + fixture->smeInfo.timeout);
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDataSmRespRetry(
	SmppHeader* header, bool sync, int num)
{
	__trace2__("sendDataSmRespRetry(): sme timeout = %d", fixture->smeInfo.timeout);
	TCSelector s(num, 2);
	__decl_tc__;
	time_t sendTime = time(NULL);
	try
	{
		uint32_t commandStatus = ESME_ROK;
		PduDataSmResp respPdu;
		respPdu.get_header().set_sequenceNumber(header->get_sequenceNumber());
		respPdu.get_header().set_commandStatus(ESME_ROK);
		auto_ptr<char> msgId = rand_char(rand0(MAX_MSG_ID_LENGTH));
		respPdu.set_messageId(msgId.get());
		SmppUtil::setupRandomCorrectOptionalParams(respPdu.get_optional(), BINARY, false);
		switch (s.value())
		{
			case 1: //временная ошибка на стороне sme, запрос на повторную доставку
				__tc__("smsResp.dataSm.sendRetry.tempAppError");
				commandStatus = ESME_RX_T_APPN;
				respPdu.get_header().set_commandStatus(commandStatus);
				fixture->transmitter->sendDataSmResp(respPdu, sync);
				break;
			case 2: //переполнение очереди стороне sme
				__tc__("smsResp.dataSm.sendRetry.msgQueueFull");
				commandStatus = ESME_RMSGQFUL;
				respPdu.get_header().set_commandStatus(commandStatus);
				fixture->transmitter->sendDataSmResp(respPdu, sync);
				break;
			/*
			case 3: //отправить респонс с неправильным sequence_number
				__tc__("smsResp.dataSm.sendRetry.invalidSequenceNumber");
				commandStatus = DELIVERY_STATUS_NO_RESPONSE;
				sendTime += fixture->smeInfo.timeout;
				respPdu.get_header().set_sequenceNumber(INT_MAX);
				respPdu.get_header().set_commandStatus(ESME_ROK);
				fixture->transmitter->sendDataSmResp(respPdu, sync);
				break;
			case 4: //не отправлять респонс
				__tc__("smsResp.dataSm.sendRetry.notSend");
				commandStatus = DELIVERY_STATUS_NO_RESPONSE;
				sendTime += fixture->smeInfo.timeout;
				break;
			case 5: //отправить респонс после sme timeout
				{
					__tc__("smsResp.dataSm.sendRetry.sendAfterSmeTimeout");
					commandStatus = DELIVERY_STATUS_NO_RESPONSE;
					sendTime += fixture->smeInfo.timeout;
					respPdu.get_header().set_commandStatus(ESME_ROK);
					int timeout = 1000 * (fixture->smeInfo.timeout + 1);
					__trace2__("sendAfterSmeTimeout(): timeout = %d", timeout);
					fixture->transmitter->sendDataSmResp(respPdu, sync, timeout);
				}
				break;
			*/
			default:
				__unreachable__("Invalid num");
		}
		__tc_ok__;
		return make_pair(commandStatus, sendTime);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
	return make_pair(Status::DELIVERYTIMEDOUT,
		time(NULL) + fixture->smeInfo.timeout);
}

pair<uint32_t, time_t> SmppProtocolTestCases::sendDataSmRespError(
	SmppHeader* header, bool sync, bool sendDelay, int num)
{
	__decl_tc__;
	int delay = 0;
	time_t sendTime = time(NULL);
	if (sendDelay)
	{
		__tc__("smsResp.dataSm.delay"); __tc_ok__;
		delay = getRandomRespDelay();
		sendTime += delay / 1000;
	}
	__trace2__("sendDataSmRespError(): delay = %d", delay);
	TCSelector s(num, 3);
	try
	{
		PduDataSmResp respPdu;
		respPdu.get_header().set_sequenceNumber(header->get_sequenceNumber());
		auto_ptr<char> msgId = rand_char(rand0(MAX_MSG_ID_LENGTH));
		respPdu.set_messageId(msgId.get());
		SmppUtil::setupRandomCorrectOptionalParams(respPdu.get_optional(), BINARY, false);
		switch (s.value())
		{
			case 1: //отправить респонс с кодом ошибки 0x1-0x10f
				__tc__("smsResp.dataSm.sendError.standardError");
				respPdu.get_header().set_commandStatus(rand1(0x10f));
				break;
			case 2: //отправить респонс с кодом ошибки:
				//0x110-0x3ff - Reserved for SMPP extension
				//0x400-0x4ff - Reserved for SMSC vendor specific
				__tc__("smsResp.dataSm.sendError.reservedError");
				respPdu.get_header().set_commandStatus(rand2(0x110, 0x4ff));
				break;
			case 3: //отправить респонс с кодом ошибки >0x500 - Reserved
				__tc__("smsResp.dataSm.sendError.outRangeError");
				respPdu.get_header().set_commandStatus(rand2(0x500, INT_MAX));
				break;
			/*
			case 4: //неустранимая ошибка на стороне sme, отказ от всех последующих сообщений
				__tc__("smsResp.dataSm.sendError.permanentAppError");
				respPdu.get_header().set_commandStatus(ESME_RX_P_APPN);
				break;
			*/
			default:
				__unreachable__("Invalid num");
		}
		//рекурсивный вызов, чтобы предотвратить закрытие smpp сессии
		if (respPdu.get_header().get_commandStatus() == ESME_RX_P_APPN)
		{
			return sendDataSmRespError(header, sync, sendDelay, num);
		}
		fixture->transmitter->sendDataSmResp(respPdu, sync, delay);
		__tc_ok__;
		return make_pair(respPdu.get_header().get_commandStatus(), sendTime);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
	return make_pair(Status::DELIVERYTIMEDOUT,
		time(NULL) + fixture->smeInfo.timeout);
}

}
}
}

