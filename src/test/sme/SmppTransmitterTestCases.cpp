#include "SmppTransmitterTestCases.hpp"
#include "test/TestConfig.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::sme::SmppTransmitter;
using namespace smsc::sms; //constants
using namespace smsc::test; //config constants
using namespace smsc::test::smpp; //constants, SmppUtil
using namespace smsc::test::sms; //constants
using namespace smsc::test::util; //constants
using namespace smsc::smpp;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::profiler;

SmppTransmitterTestCases::SmppTransmitterTestCases(SmppSession* sess,
	const SmeSystemId& id, const Address& addr, const SmeRegistry* _smeReg,
	ProfileRegistry* _profileReg, RouteChecker* _routeChecker,
	SmppPduChecker* _pduChecker, CheckList* _chkList)
	: session(sess), systemId(id), smeAddr(addr), smeReg(_smeReg),
	profileReg(_profileReg), routeChecker(_routeChecker),
	pduChecker(_pduChecker), chkList(_chkList)
{
	__require__(session);
	__require__(smeReg);
	//__require__(profileReg);
	//__require__(routeChecker);
	//__require__(pduChecker);
	//__require__(chkList);
	pduReg = smeReg->getPduRegistry(smeAddr); //может быть NULL
	//__trace2__("For smeAddr = %s pduReg = %p ", str(smeAddr).c_str(), pduReg);
}

Category& SmppTransmitterTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppTransmitterTestCases");
	return log;
}

void SmppTransmitterTestCases::submitSmAssert(int num)
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
			__dumpSubmitSmPdu__("SmppTransmitterTestCases::submitSmAssert", systemId, &pdu);
			__tc_fail__(s.value());
		}
		catch (...)
		{
			__tc_ok__;
		}
	}
}

PduData* SmppTransmitterTestCases::getNonReplaceEnrotePdu()
{
	__require__(pduReg);
	__cfg_int__(sequentialPduInterval);
	PduRegistry::PduDataIterator* it = pduReg->getPduByWaitTime(
		time(NULL) + sequentialPduInterval, LONG_MAX);
	//ищу корректную незамещающую и незамещенную ранее pdu
	PduData* pduData = NULL;
	while (PduData* data = it->next())
	{
		if (!data->replacedByPdu && !data->replacePdu &&
			data->smsId.length() && data->deliveryFlag == PDU_REQUIRED_FLAG &&
			!data->intProps.size() && !data->strProps.size())
		{
			pduData = data;
			break;
		}
	}
	delete it;
	return pduData;
}

PduData* SmppTransmitterTestCases::getReplaceEnrotePdu()
{
	__require__(pduReg);
	__cfg_int__(sequentialPduInterval);
	PduRegistry::PduDataIterator* it = pduReg->getPduByWaitTime(
		time(NULL) + sequentialPduInterval, LONG_MAX);
	//ищу замещающую и незамещенную ранее pdu
	PduData* pduData = NULL;
	while (PduData* data = it->next())
	{
		if (!data->replacedByPdu && data->replacePdu &&
			data->smsId.length() && data->deliveryFlag == PDU_REQUIRED_FLAG &&
			!data->intProps.size() && !data->strProps.size())
		{
			pduData = data;
			break;
		}
	}
	delete it;
	return pduData;
}

PduData* SmppTransmitterTestCases::getNonReplaceRescheduledEnrotePdu()
{
	__require__(pduReg);
	__cfg_int__(sequentialPduInterval);
	PduRegistry::PduDataIterator* it = pduReg->getPduByWaitTime(0, time(NULL));
	//ищу незамещенную ранее pdu
	PduData* pduData = NULL;
	while (PduData* data = it->next())
	{
		if (!data->replacedByPdu && data->smsId.length() &&
			data->deliveryFlag == PDU_REQUIRED_FLAG &&
			!data->intProps.size() && !data->strProps.size() &&
			data->deliveryFlag.getNextTime(time(NULL)) >=
			time(NULL) + sequentialPduInterval)
		{
			pduData = data;
			break;
		}
	}
	delete it;
	return pduData;
}

template <class Message>
bool SmppTransmitterTestCases::hasDeliveryReceipt(Message& m, Profile& profile)
{
	uint8_t regDelivery = m.get_registredDelivery();
	return (profile.reportoptions != ProfileReportOptions::ReportNone) ||
		(regDelivery & SMSC_DELIVERY_RECEIPT_BITS) == FINAL_SMSC_DELIVERY_RECEIPT ||
		(regDelivery & SMSC_DELIVERY_RECEIPT_BITS) == FAILURE_SMSC_DELIVERY_RECEIPT;
}

template <class Message>
bool SmppTransmitterTestCases::hasIntermediateNotification(Message& m, Profile& profile)
{
	return false;
}

template <class Message>
void SmppTransmitterTestCases::checkRegisteredDelivery(Message& m)
{
	//m.get_registredDelivery();
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(maxDeliveryPeriod);
	__cfg_int__(timeCheckAccuracy);
	__cfg_int__(sequentialPduInterval);
	Address srcAddr;
	SmppUtil::convert(m.get_source(), &srcAddr);
	time_t t;
	Profile profile = profileReg->getProfile(srcAddr, t);
	__require__(t <= time(NULL)); //с точностью до секунды
	if (hasDeliveryReceipt(m, profile) || hasIntermediateNotification(m, profile))
	{
		SmppTime t;
		time_t waitTime = time(NULL) + rand2(sequentialPduInterval, maxWaitTime);
		time_t validTime = waitTime + rand2(sequentialPduInterval, maxDeliveryPeriod);
		m.set_scheduleDeliveryTime(
			SmppUtil::time2string(waitTime, t, time(NULL), __numTime__));
		m.set_validityPeriod(
			SmppUtil::time2string(validTime, t, time(NULL), __numTime__));
	}
}

void SmppTransmitterTestCases::setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu,
	const Address* destAlias)
{
	SmppUtil::setupRandomCorrectSubmitSmPdu(pdu);
	 //Default message Type (i.e. normal message)
	pdu->get_message().set_esmClass(
		pdu->get_message().get_esmClass() & 0xc3);
	//если ожидаются подтверждения доставки или промежуточные нотификации,
	//установить отложенную доставку
	checkRegisteredDelivery(pdu->get_message());
	//source
	PduAddress srcAddr;
	SmppUtil::convert(smeAddr, &srcAddr);
	pdu->get_message().set_source(srcAddr);
	PduAddress addr;
	if (destAlias)
	{
		SmppUtil::convert(*destAlias, &addr);
	}
	else //случайный dest алиас без проверки наличия маршрутов
	{
		const Address* tmp = smeReg->getRandomAddress();
		__require__(tmp);
		SmppUtil::convert(*tmp, &addr);
	}
	pdu->get_message().set_dest(addr);
	//msgRef
	if (pduReg)
	{
		pdu->get_optional().set_userMessageReference(pduReg->nextMsgRef());
	}
}

//предварительная регистрация pdu, требуется внешняя синхронизация
PduData* SmppTransmitterTestCases::registerSubmitSm(PduSubmitSm* pdu,
	PduData* existentPduData, time_t submitTime, PduData::IntProps* intProps,
	PduData::StrProps* strProps, bool normalSms)
{
	__require__(pduReg);
	__cfg_int__(maxValidPeriod);
	time_t waitTime = max(submitTime, SmppUtil::getWaitTime(
			pdu->get_message().get_scheduleDeliveryTime(), submitTime));
	time_t validTime = normalSms ? SmppUtil::getValidTime(
		pdu->get_message().get_validityPeriod(), submitTime) : waitTime + maxValidPeriod;
	PduData* pduData = new PduData(pdu->get_optional().get_userMessageReference(),
		submitTime, waitTime, validTime, reinterpret_cast<SmppHeader*>(pdu));
	//для флагов самые простые проверки, остальное делается в
	//checkSubmitSmResp, checkSubmitTime, checkWaitTime, checkValidTime
	//pduData->responseFlag = PDU_REQUIRED_FLAG;
	pduData->deliveryFlag = routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest(), false) ?
		PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG;
	//pdu->get_message().get_registredDelivery();
	Address srcAddr;
	SmppUtil::convert(pdu->get_message().get_source(), &srcAddr);
	time_t t; //профиль нужно брать именно тот, что в profileReg, игнорируя profileUpdateTime
	Profile profile = profileReg->getProfile(srcAddr, t);
	__require__(t <= time(NULL)); //с точностью до секунды
	pduData->reportOptions = profile.reportoptions;
	pduData->deliveryReceiptFlag =
		hasDeliveryReceipt(pdu->get_message(), profile) ?
		PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG;
	pduData->intermediateNotificationFlag =
		hasIntermediateNotification(pdu->get_message(), profile) ?
		PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG;
	//опциональные проперти
	if (intProps)
	{
		pduData->intProps = *intProps;
	}
	if (strProps)
	{
		pduData->strProps = *strProps;
	}
	//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
	//destination address and service_type. Сообщение должно быть в 
	//ENROTE state.
	PduSubmitSm* existentPdu = existentPduData && existentPduData->pdu ?
		reinterpret_cast<PduSubmitSm*>(existentPduData->pdu) : NULL;
	if (existentPdu && !strcmp(pdu->get_message().get_serviceType(),
		existentPdu->get_message().get_serviceType()) &&
		pdu->get_message().get_source() == existentPdu->get_message().get_source() &&
		pdu->get_message().get_dest() == existentPdu->get_message().get_dest() &&
		existentPduData->deliveryFlag == PDU_REQUIRED_FLAG)
	{
		if (pdu->get_message().get_replaceIfPresentFlag() == 0)
		{
			pduData->intProps["hasSmppDuplicates"] = 1;
			existentPduData->intProps["hasSmppDuplicates"] = 1;
		}
		else if (pdu->get_message().get_replaceIfPresentFlag() == 1)
		{
			pduData->replacePdu = existentPduData;
			existentPduData->replacedByPdu = pduData;
		}
	}
	pduReg->registerPdu(pduData);
	return pduData;
}

//обновить smsId, sequenceNumber в PduRegistry и проверить pdu
//требуется внешняя синхронизация
void SmppTransmitterTestCases::processSubmitSmSync(PduData* pduData,
	PduSubmitSmResp* respPdu, time_t respTime)
{
	__require__(pduReg);
	__require__(pduData);
	__dumpPdu__("processSubmitSmRespSync", systemId, respPdu);
	__decl_tc__;
	__tc__("processSubmitSmResp.sync");
	if (!respPdu)
	{
		__tc_fail__(1);
		pduData->responseFlag = PDU_MISSING_ON_TIME_FLAG;
		//обновить sequenceNumber
		pduReg->updatePdu(pduData);
	}
	else
	{
		pduChecker->processSubmitSmResp(pduData, *respPdu, respTime);
		delete respPdu; //disposePdu
	}
	__tc_ok_cond__;
	//pduReg->updatePdu(pduData);  //вся обработка в processReplaceSmResp()
}

//обновить sequenceNumber в PduRegistry, требуется внешняя синхронизация
void SmppTransmitterTestCases::processSubmitSmAsync(PduData* pduData,
	PduSubmitSmResp* respPdu)
{
	__require__(pduReg);
	__require__(pduData);
	__decl_tc__;
	__tc__("processSubmitSmResp.async");
	if (respPdu)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
}

//отправить и зарегистрировать pdu
void SmppTransmitterTestCases::sendSubmitSmPdu(PduSubmitSm* pdu,
	PduData* existentPduData, bool sync, PduData::IntProps* intProps,
	PduData::StrProps* strProps, bool normalSms)
{
	__decl_tc__;
	try
	{
		if (pduReg)
		{
			if (sync)
			{
				__tc__("submitSm.sync");
				PduData* pduData;
				{
					MutexGuard mguard(pduReg->getMutex());
					pdu->get_header().set_sequenceNumber(0); //не известен
					pduData = registerSubmitSm(pdu, existentPduData, time(NULL),
						intProps, strProps, normalSms); //all times, msgRef
				}
				//__dumpSubmitSmPdu__("submitSmSyncBefore", systemId, pdu);
				PduSubmitSmResp* respPdu = session->getSyncTransmitter()->submit(*pdu);
				__dumpSubmitSmPdu__("submitSmSyncAfter", systemId, pdu);
				{
					MutexGuard mguard(pduReg->getMutex());
					processSubmitSmSync(pduData, respPdu, time(NULL)); //smsId, sequenceNumber
				}
			}
			else
			{
				__tc__("submitSm.async");
				MutexGuard mguard(pduReg->getMutex());
				//__dumpSubmitSmPdu__("submitSmAsyncBefore", systemId, pdu);
				time_t submitTime = time(NULL);
				PduSubmitSmResp* respPdu = session->getAsyncTransmitter()->submit(*pdu);
				__dumpSubmitSmPdu__("submitSmAsyncAfter", systemId, pdu);
				PduData* pduData = registerSubmitSm(pdu, existentPduData,
					submitTime, intProps, strProps, normalSms); //all times, msgRef, sequenceNumber
				processSubmitSmAsync(pduData, respPdu);
			}
			//pdu life time определяется PduRegistry
			//disposePdu(pdu);
		}
		else
		{
			if (sync)
			{
				__tc__("submitSm.sync");
				PduSubmitSmResp* respPdu =
					session->getSyncTransmitter()->submit(*pdu);
				if (respPdu)
				{
					delete respPdu; //disposePdu
				}
			}
			else
			{
				__tc__("submitSm.async");
				session->getAsyncTransmitter()->submit(*pdu);
			}
			delete pdu; //disposePdu
		}
		__tc_ok__;
	}
	catch (...)
	{
		__tc_fail__(100);
		//error();
		throw;
	}
}

void SmppTransmitterTestCases::submitSmCorrect(bool sync, int num)
{
	TCSelector s(num, 15);
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
			setupRandomCorrectSubmitSmPdu(pdu, NULL);
			PduData* existentPduData = NULL;
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
				case 9: //msgRef одинаковые (эквивалентно msgRef отсутствуют)
					//Согласно GSM 03.40 пункт 9.2.3.25 если совпадают
					//TP-MR, TP-DA, OA, то при ETSI_REJECT_IF_PRESENT будет ошибка.
					//Для SMPP все должно работать независимо от msgRef.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
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
				case 10: //отправка дублированного сообщения без замещения уже существующего
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
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
						}
					}
					break;
				case 11: //отправка дублированного сообщения с попыткой замещения
					//уже существующего, но service_type не совпадает.
					//Для несовпадающих source address и destination address
					//тест кейсов не делаю, т.к. возникнут проблемы с маршрутами.
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
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
				case 12: //отправка дублированного сообщения с замещением уже существующего
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
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
				case 13: //отправка дублированного сообщения с замещением уже
					//ранее замещенного
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
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
				case 14: //отправка дублированного сообщения с замещением уже
					//существующего, но находящегося уже в финальном состоянии.
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						PduData* finalPduData = pduReg->getLastRemovedPdu();
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
						}
					}
					break;
				case 15: //отправка дублированного сообщения с замещением уже
					//существующего, но находящегося уже в процессе повторной доставки.
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
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
			sendSubmitSmPdu(pdu, existentPduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppTransmitterTestCases::submitSmIncorrect(bool sync, int num)
{
	TCSelector s(num, 10);
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
			setupRandomCorrectSubmitSmPdu(pdu, NULL);
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
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			sendSubmitSmPdu(pdu, NULL, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

PduData* SmppTransmitterTestCases::setupRandomCorrectReplaceSmPdu(PduReplaceSm* pdu)
{
	SmppUtil::setupRandomCorrectReplaceSmPdu(pdu);
	//если ожидаются подтверждения доставки или промежуточные нотификации,
	//установить отложенную доставку
	checkRegisteredDelivery(*pdu);
	//source
	PduAddress srcAddr;
	SmppUtil::convert(smeAddr, &srcAddr);
	pdu->set_source(srcAddr);
	//выбрать случайный messageId из sms ожидающих доставки
	PduData* replacePduData = NULL;
	if (pduReg)
	{
		MutexGuard mguard(pduReg->getMutex());
		replacePduData = getNonReplaceEnrotePdu();
	}
	//задать заведомо несуществующий messageId
	if (replacePduData)
	{
		pdu->set_messageId(replacePduData->smsId.c_str());
	}
	else
	{
		auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
		pdu->set_messageId(msgId.get());
	}
	return replacePduData;
}

//предварительная регистрация pdu, требуется внешняя синхронизация
PduData* SmppTransmitterTestCases::registerReplaceSm(PduReplaceSm* pdu,
	PduData* replacePduData, time_t submitTime)
{
	if (replacePduData)
	{
		__require__(replacePduData->pdu &&
			replacePduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* replacePdu = reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
		PduSubmitSm* resPdu = new PduSubmitSm(*replacePdu);
		resPdu->get_message().set_scheduleDeliveryTime(pdu->get_scheduleDeliveryTime());
		resPdu->get_message().set_validityPeriod(pdu->get_validityPeriod());
		resPdu->get_message().set_registredDelivery(pdu->get_registredDelivery());
		resPdu->get_message().set_smDefaultMsgId(pdu->get_smDefaultMsgId());
		resPdu->get_message().set_shortMessage(pdu->get_shortMessage(), pdu->size_shortMessage());
		resPdu->get_message().set_replaceIfPresentFlag(1); //для правильной работы registerSubmitSm()
		PduData* pduData = registerSubmitSm(resPdu, replacePduData, submitTime, NULL, NULL, true);
		pduData->smsId = replacePduData->smsId;
		return pduData;
	}
	else
	{
		__require__(pduReg);
		PduData* pduData = new PduData(0, 0, 0, 0, reinterpret_cast<SmppHeader*>(pdu));
		pdu->get_header().set_sequenceNumber(0); //не известен
		//pduData->responseFlag = PDU_REQUIRED_FLAG;
		pduData->deliveryFlag = PDU_NOT_EXPECTED_FLAG;
		pduData->deliveryReceiptFlag = PDU_NOT_EXPECTED_FLAG;
		pduData->intermediateNotificationFlag = PDU_NOT_EXPECTED_FLAG;
		//pduReg->registerPdu(pduData);
		return pduData;
	}
}

//обновить sequenceNumber в PduRegistry и проверить pdu
//требуется внешняя синхронизация
void SmppTransmitterTestCases::processReplaceSmSync(PduData* pduData,
	PduReplaceSmResp* respPdu, time_t respTime)
{
	__require__(pduData);
	__dumpPdu__("processReplaceSmRespSync", systemId, respPdu);
	__decl_tc__;
	__tc__("processReplaceSmResp.sync");
	if (!respPdu)
	{
		__tc_fail__(1);
		pduData->responseFlag = PDU_MISSING_ON_TIME_FLAG;
	}
	else
	{
		pduChecker->processReplaceSmResp(pduData, *respPdu, respTime);
		delete respPdu; //disposePdu
	}
	__tc_ok_cond__;
	//pduReg->updatePdu(pduData); //вся обработка в processReplaceSmResp()
}

//обновить sequenceNumber в PduRegistry, требуется внешняя синхронизация
void SmppTransmitterTestCases::processReplaceSmAsync(PduData* pduData,
	PduReplaceSmResp* respPdu)
{
	__require__(pduData);
	__decl_tc__;
	__tc__("processReplaceSmResp.async");
	if (respPdu)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
}

//отправить и зарегистрировать pdu
void SmppTransmitterTestCases::sendReplaceSmPdu(PduReplaceSm* pdu,
	PduData* replacePduData, bool sync)
{
	__decl_tc__;
	try
	{
		if (pduReg)
		{
			if (sync)
			{
				__tc__("replaceSm.sync");
				PduData* pduData;
				{
					MutexGuard mguard(pduReg->getMutex());
					pdu->get_header().set_sequenceNumber(0); //не известен
					pduData = registerReplaceSm(pdu, replacePduData, time(NULL));
				}
				__dumpReplaceSmPdu__("replaceSmSyncBefore", systemId, pdu);
				PduReplaceSmResp* respPdu = session->getSyncTransmitter()->replace(*pdu);
				__dumpReplaceSmPdu__("replaceSmSyncAfter", systemId, pdu);
				{
					MutexGuard mguard(pduReg->getMutex());
					processReplaceSmSync(pduData, respPdu, time(NULL));
				}
			}
			else
			{
				__tc__("replaceSm.async");
				MutexGuard mguard(pduReg->getMutex());
				__dumpReplaceSmPdu__("replaceSmAsyncBefore", systemId, pdu);
				time_t submitTime = time(NULL);
				PduReplaceSmResp* respPdu = session->getAsyncTransmitter()->replace(*pdu);
				__dumpReplaceSmPdu__("replaceSmAsyncAfter", systemId, pdu);
				PduData* pduData = registerReplaceSm(pdu, replacePduData, submitTime);
				processReplaceSmAsync(pduData, respPdu);
			}
			//pdu life time определяется PduRegistry
			//disposePdu(pdu);
		}
		else
		{
			if (sync)
			{
				__tc__("replaceSm.sync");
				PduReplaceSmResp* respPdu =
					session->getSyncTransmitter()->replace(*pdu);
				if (respPdu)
				{
					delete respPdu; //disposePdu
				}
			}
			else
			{
				__tc__("replaceSm.async");
				session->getAsyncTransmitter()->replace(*pdu);
			}
			delete pdu; //disposePdu
		}
		__tc_ok__;
	}
	catch (...)
	{
		__tc_fail__(100);
		//error();
		throw;
	}
}

void SmppTransmitterTestCases::replaceSm(bool sync, int num)
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
			PduData* replacePduData = setupRandomCorrectReplaceSmPdu(pdu);
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
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						replacePduData = getReplaceEnrotePdu();
						if (replacePduData)
						{
							__tc__("replaceSm.replaceReplacedEnrote");
							pdu->set_messageId(replacePduData->smsId.c_str());
						}
					}
					break;
				case 10: //замещением существующего сообщения, но находящегося
					//уже в финальном состоянии.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						PduData* finalPduData = pduReg->getLastRemovedPdu();
						if (finalPduData)
						{
							__tc__("replaceSm.replaceFinal");
							pdu->set_messageId(finalPduData->smsId.c_str());
						}
					}
					break;
				case 11: //замещение существующего сообщения, но находящегося
					//уже в процессе повторной доставки.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						replacePduData = getNonReplaceRescheduledEnrotePdu();
						if (replacePduData)
						{
							__tc__("replaceSm.replaceRepeatedDeliveryEnrote");
							pdu->set_messageId(replacePduData->smsId.c_str());
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//отправить и зарегистрировать pdu
			sendReplaceSmPdu(pdu, replacePduData, sync);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmppTransmitterTestCases::sendDeliverySmResp(PduDeliverySmResp& pdu, bool sync)
{
	__decl_tc__;
	try
	{
		if (sync)
		{
			__tc__("sendDeliverySmResp.sync");
			//__dumpPdu__("sendDeliverySmRespSyncBefore", systemId, pdu);
			session->getSyncTransmitter()->sendDeliverySmResp(pdu);
			__dumpPdu__("sendDeliverySmRespSyncAfter", systemId, &pdu);
		}
		else
		{
			__tc__("sendDeliverySmResp.async");
			//__dumpPdu__("sendDeliverySmRespAsyncBefore", systemId, pdu);
			session->getAsyncTransmitter()->sendDeliverySmResp(pdu);
			__dumpPdu__("sendDeliverySmRespAsyncAfter", systemId, &pdu);
		}
		__tc_ok__;
	}
	catch (...)
	{
		__tc_fail__(100);
		//error();
		throw;
	}
}

uint32_t SmppTransmitterTestCases::sendDeliverySmRespOk(PduDeliverySm& pdu, bool sync)
{
	__trace__("sendDeliverySmRespOk()");
	__decl_tc__;
	try
	{
		__tc__("sendDeliverySmResp.sendOk");
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		respPdu.get_header().set_commandStatus(ESME_ROK); //No Error
		sendDeliverySmResp(respPdu, sync);
		__tc_ok__;
		return ESME_ROK;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
	return 0xffffffff;
}

uint32_t SmppTransmitterTestCases::sendDeliverySmRespRetry(PduDeliverySm& pdu,
	bool sync, int num)
{
	__trace__("sendDeliverySmRespRetry()");
	TCSelector s(num, 4);
	__decl_tc__;
	try
	{
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		switch (s.value())
		{
			case 1: //не отправлять респонс
				__tc__("sendDeliverySmResp.sendRetry.notSend");
				respPdu.get_header().set_commandStatus(0xffffffff);
				break;
			case 2: //временная ошибка на стороне sme, запрос на повторную доставку
				__tc__("sendDeliverySmResp.sendRetry.tempAppError");
				respPdu.get_header().set_commandStatus(ESME_RX_T_APPN);
				sendDeliverySmResp(respPdu, sync);
				break;
			case 3: //переполнение очереди стороне sme
				__tc__("sendDeliverySmResp.sendRetry.msgQueueFull");
				respPdu.get_header().set_commandStatus(ESME_RMSGQFUL);
				sendDeliverySmResp(respPdu, sync);
				break;
			case 4: //отправить респонс с неправильным sequence_number
				__tc__("sendDeliverySmResp.sendRetry.invalidSequenceNumber");
				respPdu.get_header().set_sequenceNumber(INT_MAX);
				respPdu.get_header().set_commandStatus(0xffffffff);
				//respPdu.get_header().set_commandStatus(ESME_ROK); //No Error
				sendDeliverySmResp(respPdu, sync);
				break;
			default:
				__unreachable__("Invalid num");
		}
		__tc_ok__;
		return respPdu.get_header().get_commandStatus();
	}
	catch(...)
	{
		__tc_fail__(s.value());
		error();
	}
	return 0xffffffff;
}

uint32_t SmppTransmitterTestCases::sendDeliverySmRespError(PduDeliverySm& pdu,
	bool sync, int num)
{
	__trace__("sendDeliverySmRespError()");
	TCSelector s(num, 3);
	__decl_tc__;
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
		sendDeliverySmResp(respPdu, sync);
		__tc_ok__;
		return respPdu.get_header().get_commandStatus();
	}
	catch(...)
	{
		__tc_fail__(s.value());
		error();
	}
	return 0xffffffff;
}

void SmppTransmitterTestCases::sendUpdateProfilePdu(PduSubmitSm* pdu,
	const string& text, bool sync, uint8_t dataCoding, PduData::IntProps& intProps)
{
	__require__(pdu);
	__decl_tc__;
	try
	{
		switch (dataCoding)
		{
			case DATA_CODING_SMSC_DEFAULT:
				__tc__("updateProfileCorrect.cmdTextDefault");
				break;
			case DATA_CODING_UCS2:
				__tc__("updateProfileCorrect.cmdTextUcs2");
				break;
			default:
				__unreachable__("Invalid data coding");
		}
		string encText = encode(text.c_str(), dataCoding);
		pdu->get_message().set_shortMessage(encText.c_str(), encText.length());
		pdu->get_message().set_dataCoding(dataCoding);
		//отправить и зарегистрировать pdu
		sendSubmitSmPdu(pdu, NULL, sync, &intProps, NULL, false);
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		//error();
		throw;
	}
}

void SmppTransmitterTestCases::updateProfileCorrect(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 8);
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			__cfg_addr__(profilerAlias);
			setupRandomCorrectSubmitSmPdu(pdu, &profilerAlias);
			string text;
			int cmdType;
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //report none
					__tc__("updateProfile.reportOptions.reportNoneMixedCase");
					text = "RePoRT NoNe";
					intProps["reportOptions"] = ProfileReportOptions::ReportNone;
					cmdType = UPDATE_REPORT_OPTIONS;
					break;
				case 2: //report none
					__tc__("updateProfile.reportOptions.reportNoneSpaces");
					text = "  rEpOrt  nOnE  ";
					intProps["reportOptions"] = ProfileReportOptions::ReportNone;
					cmdType = UPDATE_REPORT_OPTIONS;
					break;
				case 3: //report full
					__tc__("updateProfile.reportOptions.reportFullMixedCase");
					text = "RePoRT FuLL";
					intProps["reportOptions"] = ProfileReportOptions::ReportFull;
					cmdType = UPDATE_REPORT_OPTIONS;
					break;
				case 4: //report full
					__tc__("updateProfile.reportOptions.reportFullSpaces");
					text = "  rEpOrt  fUll  ";
					intProps["reportOptions"] = ProfileReportOptions::ReportFull;
					cmdType = UPDATE_REPORT_OPTIONS;
					break;
				case 5: //ucs2 codepage
					__tc__("updateProfile.dataCoding.ucs2CodepageMixedCase");
					text = "uCS2";
					intProps["codePage"] = ProfileCharsetOptions::Ucs2;
					cmdType = UPDATE_CODE_PAGE;
					break;
				case 6: //usc2 codepage
					__tc__("updateProfile.dataCoding.ucs2CodepageSpaces");
					text = "  Ucs2  ";
					intProps["codePage"] = ProfileCharsetOptions::Ucs2;
					cmdType = UPDATE_CODE_PAGE;
					break;
				case 7: //default codepage
					__tc__("updateProfile.dataCoding.defaultCodepageMixedCase");
					text = "DeFauLT";
					intProps["codePage"] = ProfileCharsetOptions::Default;
					cmdType = UPDATE_CODE_PAGE;
					break;
				case 8: //default codepage
					__tc__("updateProfile.dataCoding.defaultCodepageSpaces");
					text = "  dEfAUlt  ";
					intProps["codePage"] = ProfileCharsetOptions::Default;
					cmdType = UPDATE_CODE_PAGE;
					break;
				default:
					__unreachable__("Invalid num");
			}
			//задать кодировку, отправить и зарегистрировать pdu
			if (profileReg)
			{
				time_t t;
				Profile profile = profileReg->getProfile(smeAddr, t);
				if (intProps.count("reportOptions"))
				{
					profile.reportoptions = intProps.find("reportOptions")->second;
				}
				if (intProps.count("codePage"))
				{
					profile.codepage = intProps.find("codePage")->second;
				}
				//установить немедленную доставку и обновить profileReg
				pdu->get_message().set_scheduleDeliveryTime("");
				profileReg->putProfile(smeAddr, profile);
			}
			sendUpdateProfilePdu(pdu, text, sync, dataCoding, intProps);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppTransmitterTestCases::updateProfileIncorrect(bool sync, uint8_t dataCoding)
{
	__decl_tc__;
	__tc__("updateProfile.incorrectCmdText");
	try
	{
		PduSubmitSm* pdu = new PduSubmitSm();
		__cfg_addr__(profilerAlias);
		setupRandomCorrectSubmitSmPdu(pdu, &profilerAlias);
		PduData::IntProps intProps;
		intProps["incorrectCmdText"] = 1;
		//задать кодировку, отправить и зарегистрировать pdu
		sendUpdateProfilePdu(pdu, "Cmd Text", sync, dataCoding, intProps);
		__tc_ok__;

	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

}
}
}

