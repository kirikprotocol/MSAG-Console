#include "SmppTransmitterTestCases.hpp"
#include "SmppPduChecker.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "util/Exception.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::util::Exception;
using smsc::test::conf::TestConfig;
using namespace smsc::sms; //constants
using namespace smsc::profiler; //constants, Profile
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants
using namespace smsc::test::smpp; //constants, SmppUtil
using namespace smsc::test::core; //constants
using namespace smsc::test::sms; //constants
using namespace smsc::test::util; //constants

Category& SmppTransmitterTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppTransmitterTestCases");
	return log;
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
    /*
	//m.get_registredDelivery();
	__cfg_int__(maxWaitTime);
	__cfg_int__(maxValidPeriod);
	__cfg_int__(maxDeliveryPeriod);
	__cfg_int__(timeCheckAccuracy);
	__cfg_int__(sequentialPduInterval);
	Address srcAddr;
	SmppUtil::convert(m.get_source(), &srcAddr);
	time_t t;
	Profile profile = fixture->profileReg->getProfile(srcAddr, t);
	__require__(t <= time(NULL)); //с точностью до секунды
	if (hasDeliveryReceipt(m, profile) || hasIntermediateNotification(m, profile))
	{
		time_t waitTime = time(NULL) + rand2(sequentialPduInterval, maxWaitTime);
		time_t validTime = SmppUtil::adjustValidTime(waitTime,
			waitTime + rand2(sequentialPduInterval, maxDeliveryPeriod));
		SmppTime t;
		m.set_scheduleDeliveryTime(
			SmppUtil::time2string(waitTime, t, time(NULL), __numTime__));
		m.set_validityPeriod(
			SmppUtil::time2string(validTime, t, time(NULL), __numTime__));
	}
    */
}

void SmppTransmitterTestCases::setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu,
	const Address& destAlias, uint64_t mask)
{
	__require__(pdu);
	SmppUtil::setupRandomCorrectSubmitSmPdu(pdu, mask);
	 //Default message Type (i.e. normal message)
	pdu->get_message().set_esmClass(
		pdu->get_message().get_esmClass() & 0xc3);
	//если ожидаются подтверждения доставки или промежуточные нотификации,
	//установить отложенную доставку
	checkRegisteredDelivery(pdu->get_message());
	//source
	PduAddress srcAddr;
	SmppUtil::convert(fixture->smeAddr, &srcAddr);
	pdu->get_message().set_source(srcAddr);
	PduAddress addr;
	SmppUtil::convert(destAlias, &addr);
	pdu->get_message().set_dest(addr);
	//msgRef
	if (fixture->pduReg)
	{
		pdu->get_optional().set_userMessageReference(fixture->pduReg->nextMsgRef());
	}
}

uint8_t SmppTransmitterTestCases::getRegisteredDelivery(PduData* pduData)
{
	__require__(pduData && pduData->pdu);
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	switch (pduData->reportOptions)
	{
		case ProfileReportOptions::ReportNone:
			return (pdu->get_message().get_registredDelivery() &
				SMSC_DELIVERY_RECEIPT_BITS);
		case ProfileReportOptions::ReportFull:
			return FINAL_SMSC_DELIVERY_RECEIPT;
		default:
			__unreachable__("Invalid report options");
	}
}

void SmppTransmitterTestCases::registerNormalSmeMonitors(PduSubmitSm* pdu,
	PduData* existentPduData, time_t waitTime, time_t validTime, PduData* pduData)
{
	bool destReachble = fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest(), true);
	//delivery monitor
	PduFlag deliveryFlag = destReachble ? PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG;
	DeliveryMonitor* deliveryMonitor =
		new DeliveryMonitor(nvl(pdu->get_message().get_serviceType()),
			waitTime, validTime, pduData, deliveryFlag);
	if (existentPduData)
	{
		//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
		//destination address and service_type. Сообщение должно быть в 
		//ENROTE state (никак не проверяю).
		__require__(existentPduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* existentPdu =
			reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
		if (!strcmp(nvl(pdu->get_message().get_serviceType()),
				nvl(existentPdu->get_message().get_serviceType())) &&
			pdu->get_message().get_source() ==
				existentPdu->get_message().get_source() &&
			pdu->get_message().get_dest() ==
				existentPdu->get_message().get_dest())
		{
			if (pdu->get_message().get_replaceIfPresentFlag() == 0)
			{
				deliveryMonitor->pduData->intProps["hasSmppDuplicates"] = 1;
				existentPduData->intProps["hasSmppDuplicates"] = 1;
			}
			else if (pdu->get_message().get_replaceIfPresentFlag() == 1)
			{
				deliveryMonitor->pduData->replacePdu = existentPduData;
				existentPduData->replacedByPdu = pduData;
				existentPduData->valid = false;
			}
		}
	}
	fixture->pduReg->registerMonitor(deliveryMonitor);
	//delivery report monitors
	if (destReachble)
	{
		//мониторы на репорты будут созданы в NormalSmsHandler::registerDeliveryReportMonitors()
		return;
	}
	//маршрут существует, но нет sme
	//должен быть delivery receipt и intermaediate notification
	__require__(fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest(), false));
	uint8_t regDelivery = getRegisteredDelivery(pduData);
	PduFlag flag;
	switch (regDelivery)
	{
		case FINAL_SMSC_DELIVERY_RECEIPT:
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			flag = PDU_REQUIRED_FLAG;
			break;
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			flag = PDU_NOT_EXPECTED_FLAG;
			break;
		default:
			__unreachable__("Invalid registered delivery flag");
	}
	if (flag == PDU_REQUIRED_FLAG)
	{
		//intermediate notification
		IntermediateNotificationMonitor* m1 =
			new IntermediateNotificationMonitor(waitTime, pduData, PDU_REQUIRED_FLAG);
		m1->deliveryFlag = PDU_NOT_EXPECTED_FLAG;
		m1->deliveryStatus = ESME_RINVDSTADR;
		fixture->pduReg->registerMonitor(m1);
		//delivery receipt
		DeliveryReceiptMonitor* m2 =
			new DeliveryReceiptMonitor(validTime, pduData, PDU_REQUIRED_FLAG);
		m2->deliveryFlag = PDU_EXPIRED_FLAG;
		m2->deliveryStatus = ESME_RINVDSTADR;
		fixture->pduReg->registerMonitor(m2);
	}
}

void SmppTransmitterTestCases::registerExtSmeMonitors(PduSubmitSm* pdu,
	time_t waitTime, time_t validTime, PduData* pduData)
{
	//предполагаю, что ext sme всегда запущено и на него есть маршрут
	//ext sme всегда отправляет sme ack и, по ситуации, final delivery receipt
	__require__(fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest(), true));
	//sme ack monitor
	SmeAckMonitor* smeAckMonitor = new SmeAckMonitor(waitTime, pduData, PDU_REQUIRED_FLAG);
	fixture->pduReg->registerMonitor(smeAckMonitor);
	//delivery receipt monitor
	uint8_t regDelivery = getRegisteredDelivery(pduData);
	PduFlag flag;
	switch (regDelivery)
	{
		case FINAL_SMSC_DELIVERY_RECEIPT:
			flag = PDU_REQUIRED_FLAG;
			break;
		case FAILURE_SMSC_DELIVERY_RECEIPT:
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			flag = PDU_NOT_EXPECTED_FLAG;
			break;
		default:
			__unreachable__("Invalid regDelivery");
	}
	if (flag == PDU_REQUIRED_FLAG)
	{
		DeliveryReceiptMonitor* rcptMonitor =
			new DeliveryReceiptMonitor(waitTime, pduData, flag);
		rcptMonitor->deliveryFlag = PDU_RECEIVED_FLAG;
		rcptMonitor->deliveryStatus = ESME_ROK;
		fixture->pduReg->registerMonitor(rcptMonitor);
	}
}

void SmppTransmitterTestCases::registerNullSmeMonitors(PduSubmitSm* pdu,
	time_t waitTime, time_t validTime, uint32_t deliveryStatus, PduData* pduData)
{
	//предполагаю, что null sme всегда запущено и на него есть маршрут
	//null sme не отправляет sme ack, а на deliver_sm сразу отправляет респонс
	__require__(fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest(), true));
	//delivery receipt monitor
	uint8_t regDelivery = getRegisteredDelivery(pduData);
	PduFlag flag;
	switch (regDelivery)
	{
		case FINAL_SMSC_DELIVERY_RECEIPT:
			flag = PDU_REQUIRED_FLAG;
			break;
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			flag = deliveryStatus == ESME_ROK ? PDU_NOT_EXPECTED_FLAG : PDU_REQUIRED_FLAG;
			break;
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			flag = PDU_NOT_EXPECTED_FLAG;
			break;
		default:
			__unreachable__("Invalid regDelivery");
	}
	if (flag == PDU_REQUIRED_FLAG)
	{
		DeliveryReceiptMonitor* rcptMonitor = 
			new DeliveryReceiptMonitor(waitTime, pduData, flag);
		rcptMonitor->deliveryFlag = deliveryStatus == ESME_ROK ?
			PDU_RECEIVED_FLAG : PDU_ERROR_FLAG;
		rcptMonitor->deliveryStatus = deliveryStatus;
		fixture->pduReg->registerMonitor(rcptMonitor);
	}
}

//предварительная регистрация pdu, требуется внешняя синхронизация
PduData* SmppTransmitterTestCases::registerSubmitSm(PduSubmitSm* pdu,
	PduData* existentPduData, time_t submitTime, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps, PduType pduType)
{
	__require__(pdu);
	__require__(fixture->pduReg);
	//waitTime, validTime, msgRef
	time_t waitTime = max(submitTime, SmppUtil::getWaitTime(
			pdu->get_message().get_scheduleDeliveryTime(), submitTime));
	time_t validTime = SmppUtil::getValidTime(
		pdu->get_message().get_validityPeriod(), submitTime);
	__require__(pdu->get_optional().has_userMessageReference());
	uint16_t msgRef = pdu->get_optional().get_userMessageReference();
	//report options
	Address srcAddr;
	SmppUtil::convert(pdu->get_message().get_source(), &srcAddr);
	time_t t; //профиль нужно брать именно тот, что в profileReg, игнорируя profileUpdateTime
	Profile profile = fixture->profileReg->getProfile(srcAddr, t);
	__require__(t <= time(NULL)); //с точностью до секунды
	//pdu data
	PduData* pduData = new PduData(reinterpret_cast<SmppHeader*>(pdu),
		submitTime, msgRef, profile.reportoptions, intProps, strProps, objProps);
	pduData->ref();
	//если маршрута нет, то нужен только монитор на респонс (commandStatus = ESME_RINVDSTADR)
	bool routeExists = fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest(), false);
	if (!routeExists)
	{
		return pduData;
	}
	switch (pduType)
	{
		case PDU_NORMAL:
			//регистрация DeliveryMonitor и для sme, на которые есть маршрут,
			//но нет самих sme DeliveryReceiptMonitor и IntermediateNotificationMonitor
			registerNormalSmeMonitors(pdu, existentPduData, waitTime,
				validTime, pduData);
			break;
		case PDU_EXT_SME:
			//регистрация SmeAckMonitor и DeliveryReceiptMonitor
			registerExtSmeMonitors(pdu, waitTime, validTime, pduData);
			break;
		case PDU_NULL_OK:
			//регистрация только DeliveryReceiptMonitor
			registerNullSmeMonitors(pdu, waitTime, validTime, ESME_ROK, pduData);
			break;
		case PDU_NULL_ERR:
			//регистрация только DeliveryReceiptMonitor
			registerNullSmeMonitors(pdu, waitTime, validTime, ESME_RX_P_APPN, pduData);
			break;
		default:
			__unreachable__("Invalid pdu type");
	}
	//response monitor регистрируется когда станет известен seqNum
	return pduData;
}

//обновить smsId для deliver receipt монитора в PduRegistry и проверить pdu
//требуется внешняя синхронизация
void SmppTransmitterTestCases::processSubmitSmSync(PduData* pduData,
	PduSubmitSmResp* respPdu, time_t respTime)
{
	__require__(pduData);
	__dumpPdu__("processSubmitSmRespSync", fixture->smeInfo.systemId, respPdu);
	__decl_tc__;
	__tc__("processSubmitSmResp.sync");
	if (!respPdu)
	{
		__tc_fail__(1);
	}
	else
	{
		//создать, но не регистрировать респонс монитор
		ResponseMonitor monitor(pduData->pdu->get_sequenceNumber(),
			pduData, PDU_REQUIRED_FLAG);
		fixture->pduChecker->processSubmitSmResp(&monitor, *respPdu, respTime);
		delete respPdu; //disposePdu
	}
	__tc_ok_cond__;
	pduData->unref();
}

//зарегистрировать респонс монитор, требуется внешняя синхронизация
void SmppTransmitterTestCases::processSubmitSmAsync(PduData* pduData)
{
	__require__(fixture->pduReg);
	__require__(pduData);
	//создать и зарегистрировать респонс монитор
	ResponseMonitor* monitor = new ResponseMonitor(
		pduData->pdu->get_sequenceNumber(), pduData, PDU_REQUIRED_FLAG);
	fixture->pduReg->registerMonitor(monitor);
	pduData->unref();
}

//отправить и зарегистрировать pdu
void SmppTransmitterTestCases::sendSubmitSmPdu(PduSubmitSm* pdu,
	PduData* existentPduData, bool sync, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps, PduType pduType)
{
	__decl_tc__;
	try
	{
		if (fixture->pduReg)
		{
			if (sync)
			{
				__tc__("submitSm.sync");
				PduData* pduData;
				time_t submitTime;
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					pdu->get_header().set_sequenceNumber(0); //не известен
					submitTime = time(NULL);
					pduData = registerSubmitSm(pdu, existentPduData, submitTime,
						intProps, strProps, objProps, pduType); //all times, msgRef
				}
				//__dumpSubmitSmPdu__("submitSmSyncBefore", fixture->smeInfo.systemId, pdu);
				PduSubmitSmResp* respPdu =
					fixture->session->getSyncTransmitter()->submit(*pdu);
				time_t respTime = time(NULL);
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					__dumpSubmitSmPdu__("submitSmSyncAfter", fixture->smeInfo.systemId, pdu);
					if (respPdu)
					{
						processSubmitSmSync(pduData, respPdu, respTime);
					}
					else
					{
						__tc_fail__(1);
					}
				}
			}
			else
			{
				__tc__("submitSm.async");
				MutexGuard mguard(fixture->pduReg->getMutex());
				//__dumpSubmitSmPdu__("submitSmAsyncBefore", fixture->smeInfo.systemId, pdu);
				time_t submitTime = time(NULL);
				PduSubmitSmResp* respPdu =
					fixture->session->getAsyncTransmitter()->submit(*pdu);
				__require__(!respPdu);
				time_t responseTime = time(NULL);
				__dumpSubmitSmPdu__("submitSmAsyncAfter", fixture->smeInfo.systemId, pdu);
				PduData* pduData = registerSubmitSm(pdu, existentPduData,
					submitTime, intProps, strProps, objProps, pduType); //all times, msgRef, sequenceNumber
				processSubmitSmAsync(pduData);
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
					fixture->session->getSyncTransmitter()->submit(*pdu);
				if (respPdu)
				{
					delete respPdu; //disposePdu
				}
			}
			else
			{
				__tc__("submitSm.async");
				fixture->session->getAsyncTransmitter()->submit(*pdu);
			}
			delete pdu; //disposePdu
		}
		__tc_ok_cond__;
	}
	catch (...)
	{
		__tc_fail__(100);
		//error();
		throw;
	}
}

void SmppTransmitterTestCases::setupRandomCorrectReplaceSmPdu(PduReplaceSm* pdu,
	PduData* replacePduData)
{
	uint8_t dataCoding = DATA_CODING_SMSC_DEFAULT;
	if (replacePduData)
	{
		PduSubmitSm* origPdu = reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
		__require__(origPdu);
		dataCoding = origPdu->get_message().get_dataCoding();
	}
	SmppUtil::setupRandomCorrectReplaceSmPdu(pdu, dataCoding);
	//если ожидаются подтверждения доставки или промежуточные нотификации,
	//установить отложенную доставку
	checkRegisteredDelivery(*pdu);
	//source
	PduAddress srcAddr;
	SmppUtil::convert(fixture->smeAddr, &srcAddr);
	pdu->set_source(srcAddr);
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
}

//предварительная регистрация pdu, требуется внешняя синхронизация
PduData* SmppTransmitterTestCases::registerReplaceSm(PduReplaceSm* pdu,
	PduData* replacePduData, time_t submitTime)
{
	if (replacePduData)
	{
		__require__(replacePduData->pdu &&
			replacePduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* replacePdu = reinterpret_cast<PduSubmitSm*>(
			replacePduData->pdu);
		PduSubmitSm* resPdu = new PduSubmitSm(*replacePdu);
		if (pdu->get_scheduleDeliveryTime())
		{
			resPdu->get_message().set_scheduleDeliveryTime(
				pdu->get_scheduleDeliveryTime());
		}
		if (pdu->get_validityPeriod())
		{
			resPdu->get_message().set_validityPeriod(
				pdu->get_validityPeriod());
		}
		resPdu->get_message().set_registredDelivery(pdu->get_registredDelivery());
		resPdu->get_message().set_smDefaultMsgId(pdu->get_smDefaultMsgId());
		resPdu->get_message().set_shortMessage(pdu->get_shortMessage(), pdu->size_shortMessage());
		resPdu->get_message().set_replaceIfPresentFlag(1); //для правильной работы registerSubmitSm()
		PduData* pduData = registerSubmitSm(resPdu, replacePduData, submitTime, NULL, NULL, NULL, PDU_NORMAL);
		pduData->smsId = pdu->get_messageId();
		__require__(fixture->pduReg);
		DeliveryReceiptMonitor* rcptMonitor =
			fixture->pduReg->getDeliveryReceiptMonitor(
				resPdu->get_optional().get_userMessageReference());
		__require__(rcptMonitor);
		return pduData;
	}
	else
	{
		__require__(fixture->pduReg);
		PduData* pduData = new PduData(reinterpret_cast<SmppHeader*>(pdu), 0, 0);
		//pduData->ref();
		pdu->get_header().set_sequenceNumber(0); //не известен
		//респонс монитор и прочие мониторы регистрировать не надо
		return pduData;
	}
}

//обновить sequenceNumber в PduRegistry и проверить pdu
//требуется внешняя синхронизация
void SmppTransmitterTestCases::processReplaceSmSync(PduData* pduData,
	PduReplaceSmResp* respPdu, time_t respTime)
{
	__require__(pduData);
	__dumpPdu__("processReplaceSmRespSync", fixture->smeInfo.systemId, respPdu);
	__decl_tc__;
	__tc__("processReplaceSmRespSync.sync");
	if (!respPdu)
	{
		__tc_fail__(1);
	}
	else
	{
		//создать, но не регистрировать респонс монитор
		ResponseMonitor monitor(pduData->pdu->get_sequenceNumber(),
			pduData, PDU_REQUIRED_FLAG);
		fixture->pduChecker->processReplaceSmResp(&monitor, *respPdu, respTime);
		delete respPdu; //disposePdu
	}
	__tc_ok_cond__;
	pduData->unref();
}

//обновить sequenceNumber в PduRegistry, требуется внешняя синхронизация
void SmppTransmitterTestCases::processReplaceSmAsync(PduData* pduData)
{
	__require__(fixture->pduReg);
	__require__(pduData);
	//создать и зарегистрировать респонс монитор
	ResponseMonitor* monitor = new ResponseMonitor(
		pduData->pdu->get_sequenceNumber(), pduData, PDU_REQUIRED_FLAG);
	fixture->pduReg->registerMonitor(monitor);
	pduData->unref();
}

//отправить и зарегистрировать pdu
void SmppTransmitterTestCases::sendReplaceSmPdu(PduReplaceSm* pdu,
	PduData* replacePduData, bool sync)
{
	__decl_tc__;
	try
	{
		if (fixture->pduReg)
		{
			if (sync)
			{
				__tc__("replaceSm.sync");
				PduData* pduData;
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					pdu->get_header().set_sequenceNumber(0); //не известен
					pduData = registerReplaceSm(pdu, replacePduData, time(NULL));
				}
				__dumpReplaceSmPdu__("replaceSmSyncBefore", fixture->smeInfo.systemId, pdu);
				PduReplaceSmResp* respPdu =
					fixture->session->getSyncTransmitter()->replace(*pdu);
				__dumpReplaceSmPdu__("replaceSmSyncAfter", fixture->smeInfo.systemId, pdu);
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					processReplaceSmSync(pduData, respPdu, time(NULL));
				}
			}
			else
			{
				__tc__("replaceSm.async");
				MutexGuard mguard(fixture->pduReg->getMutex());
				__dumpReplaceSmPdu__("replaceSmAsyncBefore", fixture->smeInfo.systemId, pdu);
				time_t submitTime = time(NULL);
				PduReplaceSmResp* respPdu =
					fixture->session->getAsyncTransmitter()->replace(*pdu);
				__dumpReplaceSmPdu__("replaceSmAsyncAfter", fixture->smeInfo.systemId, pdu);
				PduData* pduData = registerReplaceSm(pdu, replacePduData, submitTime);
				processReplaceSmAsync(pduData);
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
					fixture->session->getSyncTransmitter()->replace(*pdu);
				if (respPdu)
				{
					delete respPdu; //disposePdu
				}
			}
			else
			{
				__tc__("replaceSm.async");
				fixture->session->getAsyncTransmitter()->replace(*pdu);
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

void SmppTransmitterTestCases::sendQuerySmPdu(PduQuerySm* pdu, bool sync)
{
	__unreachable__("sendQuerySmPdu()");
}

void SmppTransmitterTestCases::sendCancelSmPdu(PduCancelSm* pdu, bool sync)
{
	__unreachable__("sendCancelSmPdu()");
}

void SmppTransmitterTestCases::sendDeliverySmResp(PduDeliverySmResp& pdu,
	bool sync, int delay)
{
	__decl_tc__;
	//pdu.set_messageId("0");
	if (delay)
	{
		__require__(fixture->pduSender);
		DeliverySmRespTask* task = new DeliverySmRespTask(this, pdu, sync);
		fixture->pduSender->schedulePdu(task, delay);
		return;
	}
	try
	{
		if (sync)
		{
			__tc__("sendDeliverySmResp.sync");
			//__dumpPdu__("sendDeliverySmRespSyncBefore", fixture->smeInfo.systemId, pdu);
			fixture->session->getSyncTransmitter()->sendDeliverySmResp(pdu);
			__dumpPdu__("sendDeliverySmRespSyncAfter", fixture->smeInfo.systemId, &pdu);
		}
		else
		{
			__tc__("sendDeliverySmResp.async");
			//__dumpPdu__("sendDeliverySmRespAsyncBefore", fixture->smeInfo.systemId, pdu);
			fixture->session->getAsyncTransmitter()->sendDeliverySmResp(pdu);
			__dumpPdu__("sendDeliverySmRespAsyncAfter", fixture->smeInfo.systemId, &pdu);
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

//требуется внешняя синхронизация
void SmppTransmitterTestCases::processGenericNackSync(time_t submitTime,
	time_t respTime)
{
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	__tc__("processGenericNack.sync"); __tc_ok__;
	__tc__("processGenericNack.checkTime");
	time_t respDelay = respTime - submitTime;
	if (respDelay < 0)
	{
		__tc_fail__(1);
	}
	else if (respDelay > timeCheckAccuracy)
	{
		__tc_fail__(2);
	}
	__tc_ok_cond__;
}

//обновить sequenceNumber в PduRegistry, требуется внешняя синхронизация
void SmppTransmitterTestCases::processGenericNackAsync(PduData* pduData)
{
	__require__(fixture->pduReg);
	__require__(pduData);
	//создать и зарегистрировать респонс монитор
	GenericNackMonitor* monitor = new GenericNackMonitor(
		pduData->pdu->get_sequenceNumber(), pduData, PDU_REQUIRED_FLAG);
	fixture->pduReg->registerMonitor(monitor);
	pduData->unref();
}

void SmppTransmitterTestCases::sendInvalidPdu(SmppHeader* pdu, bool sync)
{
	__decl_tc__;
	try
	{
		pdu->set_sequenceNumber(fixture->session->getNextSeq());
		if (fixture->pduReg)
		{
			if (sync)
			{
				__tc__("sendInvalidPdu.sync");
				//__dumpPdu__("sendInvalidPduSyncBefore", fixture->smeInfo.systemId, pdu);
				time_t submitTime = time(NULL);
				try
				{
					fixture->session->getSyncTransmitter()->sendPdu(pdu);
					__tc_fail__(1);
				}
				catch (Exception&)
				{
					//ok
				}
				processGenericNackSync(submitTime, time(NULL));
				__dumpPdu__("sendInvalidPduSyncAfter", fixture->smeInfo.systemId, pdu);
			}
			else
			{
				__tc__("sendInvalidPdu.async");
				MutexGuard mguard(fixture->pduReg->getMutex());
				//__dumpPdu__("sendInvalidPduAsyncBefore", fixture->smeInfo.systemId, pdu);
				time_t submitTime = time(NULL);
				SmppHeader* respPdu =
					fixture->session->getAsyncTransmitter()->sendPdu(pdu);
				__dumpPdu__("sendInvalidPduAsyncAfter", fixture->smeInfo.systemId, pdu);
				PduData* pduData = new PduData(pdu, submitTime, 0);
				pduData->ref();
				processGenericNackAsync(pduData);
			}
			//pdu life time определяется PduRegistry
			//disposePdu(pdu);
		}
		else
		{
			if (sync)
			{
				__tc__("sendInvalidPdu.sync");
				try
				{
					fixture->session->getSyncTransmitter()->sendPdu(pdu);
					__tc_fail__(1);
				}
				catch (Exception&)
				{
					//ok
				}
			}
			else
			{
				__tc__("sendInvalidPdu.async");
				fixture->session->getAsyncTransmitter()->sendPdu(pdu);
			}
			delete pdu; //disposePdu
		}
		__tc_ok_cond__;
	}
	catch (...)
	{
		__tc_fail__(100);
		//error();
		throw;
	}
}

}
}
}

