#include "SmppTransmitterTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::test::conf::TestConfig;
using namespace smsc::sms; //constants
using namespace smsc::profiler; //constants, Profile
using namespace smsc::smpp::SmppCommandSet; //constants
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

//предварительная регистрация pdu, требуется внешняя синхронизация
PduData* SmppTransmitterTestCases::registerSubmitSm(PduSubmitSm* pdu,
	PduData* existentPduData, time_t submitTime, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps, bool normalSms)
{
	__require__(pdu);
	__require__(fixture->pduReg);
	bool destReachble = fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest(), true);
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
	int reportOptions = profile.reportoptions;
	//pdu data
	PduData* pduData = new PduData(reinterpret_cast<SmppHeader*>(pdu),
		submitTime, msgRef, intProps, strProps, objProps);
	pduData->ref();
	//delivery monitor
	PduFlag deliveryFlag = destReachble && normalSms ?
		PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG;
	DeliveryMonitor* deliveryMonitor =
		new DeliveryMonitor(pdu->get_message().get_serviceType(),
			waitTime, validTime, pduData, deliveryFlag);
	if (existentPduData)
	{
		//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
		//destination address and service_type. Сообщение должно быть в 
		//ENROTE state (никак не проверяю).
		__require__(existentPduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* existentPdu =
			reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
		if (!strcmp(pdu->get_message().get_serviceType(),
				existentPdu->get_message().get_serviceType()) &&
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
	//sme ack monitor
	PduFlag smeAckFlag = destReachble && !normalSms ?
		PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG;
	SmeAckMonitor* smeAckMonitor = new SmeAckMonitor(waitTime, pduData, smeAckFlag);
	fixture->pduReg->registerMonitor(smeAckMonitor);
	//delivery receipt monitor
	time_t startTime;
	PduFlag rcptFlag;
	uint8_t regDelivery = reportOptions == ProfileReportOptions::ReportNone ?
		pdu->get_message().get_registredDelivery() & SMSC_DELIVERY_RECEIPT_BITS :
		FINAL_SMSC_DELIVERY_RECEIPT;
	switch (regDelivery)
	{
		case FINAL_SMSC_DELIVERY_RECEIPT:
			startTime = destReachble ? waitTime : validTime;
			rcptFlag = PDU_REQUIRED_FLAG;
			break;
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			startTime = validTime;
			rcptFlag = destReachble && !normalSms ?
				PDU_NOT_EXPECTED_FLAG : PDU_REQUIRED_FLAG;
			break;
		case NO_SMSC_DELIVERY_RECEIPT: //default
		case SMSC_DELIVERY_RECEIPT_RESERVED: //equal to default
			startTime = validTime;
			rcptFlag = PDU_NOT_EXPECTED_FLAG;
			break;
		default:
			__unreachable__("Invalid regDelivery");
	}
	DeliveryReceiptMonitor* rcptMonitor = new DeliveryReceiptMonitor(
		startTime, pduData, rcptFlag);
	rcptMonitor->regDelivery = regDelivery;
	fixture->pduReg->registerMonitor(rcptMonitor);
	//response monitor регистрируется когда станет известен seqNum
	return pduData;
}

//обновить smsId для deliver receipt монитора в PduRegistry и проверить pdu
//требуется внешняя синхронизация
void SmppTransmitterTestCases::processSubmitSmSync(PduData* pduData,
	PduSubmitSmResp* respPdu, time_t respTime)
{
	__require__(pduData && respPdu && pduData->pdu->get_sequenceNumber() ==
		respPdu->get_header().get_sequenceNumber());
	__dumpPdu__("processSubmitSmRespSync", fixture->systemId, respPdu);
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
	PduData::StrProps* strProps, PduData::ObjProps* objProps, bool normalSms)
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
						intProps, strProps, objProps, normalSms); //all times, msgRef
				}
				//__dumpSubmitSmPdu__("submitSmSyncBefore", fixture->systemId, pdu);
				PduSubmitSmResp* respPdu =
					fixture->session->getSyncTransmitter()->submit(*pdu);
				time_t respTime = time(NULL);
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					__dumpSubmitSmPdu__("submitSmSyncAfter", fixture->systemId, pdu);
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
				//__dumpSubmitSmPdu__("submitSmAsyncBefore", fixture->systemId, pdu);
				time_t submitTime = time(NULL);
				PduSubmitSmResp* respPdu =
					fixture->session->getAsyncTransmitter()->submit(*pdu);
				__require__(!respPdu);
				time_t responseTime = time(NULL);
				__dumpSubmitSmPdu__("submitSmAsyncAfter", fixture->systemId, pdu);
				PduData* pduData = registerSubmitSm(pdu, existentPduData,
					submitTime, intProps, strProps, objProps, normalSms); //all times, msgRef, sequenceNumber
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
		PduData* pduData = registerSubmitSm(resPdu, replacePduData, submitTime, NULL, NULL, NULL, true);
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
	__require__(pduData && respPdu && pduData->pdu->get_sequenceNumber() ==
		respPdu->get_header().get_sequenceNumber());
	__dumpPdu__("processReplaceSmRespSync", fixture->systemId, respPdu);
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
				__dumpReplaceSmPdu__("replaceSmSyncBefore", fixture->systemId, pdu);
				PduReplaceSmResp* respPdu =
					fixture->session->getSyncTransmitter()->replace(*pdu);
				__dumpReplaceSmPdu__("replaceSmSyncAfter", fixture->systemId, pdu);
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					processReplaceSmSync(pduData, respPdu, time(NULL));
				}
			}
			else
			{
				__tc__("replaceSm.async");
				MutexGuard mguard(fixture->pduReg->getMutex());
				__dumpReplaceSmPdu__("replaceSmAsyncBefore", fixture->systemId, pdu);
				time_t submitTime = time(NULL);
				PduReplaceSmResp* respPdu =
					fixture->session->getAsyncTransmitter()->replace(*pdu);
				__dumpReplaceSmPdu__("replaceSmAsyncAfter", fixture->systemId, pdu);
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

void SmppTransmitterTestCases::sendDeliverySmResp(PduDeliverySmResp& pdu, bool sync)
{
	__decl_tc__;
	try
	{
		if (sync)
		{
			__tc__("sendDeliverySmResp.sync");
			//__dumpPdu__("sendDeliverySmRespSyncBefore", fixture->systemId, pdu);
			fixture->session->getSyncTransmitter()->sendDeliverySmResp(pdu);
			__dumpPdu__("sendDeliverySmRespSyncAfter", fixture->systemId, &pdu);
		}
		else
		{
			__tc__("sendDeliverySmResp.async");
			//__dumpPdu__("sendDeliverySmRespAsyncBefore", fixture->systemId, pdu);
			fixture->session->getAsyncTransmitter()->sendDeliverySmResp(pdu);
			__dumpPdu__("sendDeliverySmRespAsyncAfter", fixture->systemId, &pdu);
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

}
}
}

