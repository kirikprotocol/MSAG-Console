#include "NormalSmsHandler.hpp"
#include "SmppTransmitterTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/core/PduUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "profiler/profiler.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::sms::Address;
using smsc::util::Logger;
using smsc::test::conf::TestConfig;
using namespace smsc::profiler;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::test::smpp;
using namespace smsc::test::core;
using namespace smsc::test::util;

NormalSmsHandler::NormalSmsHandler(SmppFixture* _fixture)
: fixture(_fixture), chkList(_fixture->chkList) {}

Category& NormalSmsHandler::getLog()
{
	static Category& log = Logger::getCategory("NormalSmsHandler");
	return log;
}

vector<int> NormalSmsHandler::checkRoute(PduSubmitSm& pdu1, PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, destAlias1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu1.get_message().get_dest(), &destAlias1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//правильность destAddr
	const RouteHolder* routeHolder = NULL;
	const Address destAddr = fixture->aliasReg->findAddressByAlias(destAlias1);
	if (destAddr != destAddr2)
	{
		res.push_back(1);
	}
	else
	{
		//правильность маршрута
		routeHolder = fixture->routeReg->lookup(origAddr1, destAddr2);
		if (!routeHolder)
		{
			res.push_back(2);
		}
		else if (fixture->smeInfo.systemId != routeHolder->route.smeSystemId)
		{
			res.push_back(3);
		}
	}
	//правильность origAddr
	if (routeHolder)
	{
		if (fixture->smeInfo.wantAlias)
		{
			const Address origAlias = fixture->aliasReg->findAliasByAddress(origAddr1);
			if (origAlias != origAlias2)
			{
				res.push_back(4);
			}
		}
		else if (origAddr1 != origAlias2)
		{
			res.push_back(5);
		}
	}
	return res;
}

void NormalSmsHandler::compareMsgText(PduSubmitSm& origPdu, PduDeliverySm& pdu)
{
	__require__(fixture->profileReg);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	Address destAddr;
	SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
	time_t profileUpdateTime;
	int codePage = fixture->profileReg->getProfile(destAddr, profileUpdateTime).codepage;
	if (time(NULL) <= profileUpdateTime + timeCheckAccuracy)
	{
		//профайл может быть неконсистентным
		return;
	}
	//pdu
	uint8_t dc = pdu.get_message().get_dataCoding();
	const char* sm = pdu.get_message().get_shortMessage();
	uint8_t smLen = pdu.get_message().get_smLength();
	const char* mp = pdu.get_optional().has_messagePayload() ?
		pdu.get_optional().get_messagePayload() : NULL;
	int mpLen = pdu.get_optional().has_messagePayload() ?
		pdu.get_optional().size_messagePayload() : 0;
	//origPdu
	uint8_t origDc = origPdu.get_message().get_dataCoding();
	const char* origSm = origPdu.get_message().get_shortMessage();
	uint8_t origSmLen = origPdu.get_message().get_smLength();
	const char* origMp = origPdu.get_optional().has_messagePayload() ?
		origPdu.get_optional().get_messagePayload() : NULL;
	int origMpLen = origPdu.get_optional().has_messagePayload() ?
		origPdu.get_optional().size_messagePayload() : 0;
	//игнорирую опциональный language_indicator
	switch(codePage)
	{
		case ProfileCharsetOptions::Default:
			__tc__("processDeliverySm.normalSms.checkDataCoding");
			if (dc != DATA_CODING_SMSC_DEFAULT)
			{
				__tc_fail__(1);
			}
			__tc_ok_cond__;
			if (dc == DATA_CODING_SMSC_DEFAULT)
			{
				if (origDc == DATA_CODING_SMSC_DEFAULT)
				{
					__tc__("processDeliverySm.normalSms.checkTextEqualDataCoding");
				}
				else
				{
					__tc__("processDeliverySm.normalSms.checkTextDiffDataCoding");
				}
				__tc_fail2__(compare(origDc, origSm, origSmLen, dc, sm, smLen), 0);
				__tc_fail2__(compare(origDc, origMp, origMpLen, dc, mp, mpLen), 10);
				__tc_ok_cond__;
			}
			break;
		case ProfileCharsetOptions::Ucs2:
			if (dc == origDc)
			{
				__tc__("processDeliverySm.normalSms.checkTextEqualDataCoding");
			}
			else
			{
				__tc__("processDeliverySm.normalSms.checkTextDiffDataCoding");
			}
			__tc_fail2__(compare(origDc, origSm, origSmLen, dc, sm, smLen), 0);
			__tc_fail2__(compare(origDc, origMp, origMpLen, dc, mp, mpLen), 10);
			__tc_ok_cond__;
			break;
		default:
			__unreachable__("Invalid profile");
	}
}

void NormalSmsHandler::registerIntermediateNotificationMonitor(
	const DeliveryMonitor* monitor, PduRegistry* pduReg, uint32_t deliveryStatus,
	time_t recvTime, time_t respTime)
{
	__require__(monitor && pduReg);
	__decl_tc__;
	uint8_t regDelivery =
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData);
	time_t startTime;
	PduFlag flag;
	switch (regDelivery)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			startTime = monitor->getValidTime();
			flag = PDU_NOT_EXPECTED_FLAG;
			break;
		case FINAL_SMSC_DELIVERY_RECEIPT:
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			switch (monitor->getFlag())
			{
				case PDU_REQUIRED_FLAG: //delivery rescheduled
					if (deliveryStatus == 0xffffffff) //респонс не отослан
					{
						startTime = recvTime + fixture->smeInfo.timeout - 1;
					}
					else
					{
						startTime = respTime;
					}
					flag = PDU_REQUIRED_FLAG;
					break;
				case PDU_MISSING_ON_TIME_FLAG:
				case PDU_NOT_EXPECTED_FLAG:
					__unreachable__("Invalid monitor state");
					//break;
				case PDU_RECEIVED_FLAG:
				case PDU_ERROR_FLAG:
					__tc__("processDeliverySm.deliveryReport.intermediateNotification.noRescheduling");
					__tc_ok__;
					startTime = monitor->getValidTime();
					flag = PDU_NOT_EXPECTED_FLAG;
					break;
				case PDU_EXPIRED_FLAG:
					if (deliveryStatus == 0xffffffff) //респонс не отослан
					{
						startTime = recvTime + fixture->smeInfo.timeout - 1;
						flag = startTime < monitor->getValidTime() ?
							PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG;
					}
					else
					{
						startTime = respTime;
						flag = PDU_REQUIRED_FLAG;
					}
					break;
				default:
					__unreachable__("Invalid monitor flag");
			}
			break;
		default:
			__unreachable__("Invalid registered delivery flag");
	}
	IntermediateNotificationMonitor* m =
		new IntermediateNotificationMonitor(startTime, monitor->pduData, flag);
	m->deliveryFlag = monitor->getFlag();
	m->deliveryStatus = deliveryStatus;
	pduReg->registerMonitor(m);
}

void NormalSmsHandler::registerDeliveryReceiptMonitor(const DeliveryMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime, time_t respTime)
{
	__require__(monitor && pduReg);
	__decl_tc__;
	uint8_t regDelivery =
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData);
	time_t startTime = 0;
	PduFlag flag;
	switch (regDelivery)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			break;
		case FINAL_SMSC_DELIVERY_RECEIPT:
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			switch (monitor->getFlag())
			{
				case PDU_REQUIRED_FLAG:
					//не регистрировать монитор, продолжение повторных доставок
					break;
				case PDU_MISSING_ON_TIME_FLAG:
				case PDU_NOT_EXPECTED_FLAG:
					__unreachable__("Invalid monitor state");
					//break;
				case PDU_RECEIVED_FLAG:
					startTime = respTime;
					if (regDelivery == FAILURE_SMSC_DELIVERY_RECEIPT)
					{
						__tc__("processDeliverySm.deliveryReport.deliveryReceipt.failureDeliveryReceipt");
						__tc_ok__;
						flag = PDU_NOT_EXPECTED_FLAG;
					}
					else
					{
						__require__(regDelivery == FINAL_SMSC_DELIVERY_RECEIPT);
						flag = PDU_REQUIRED_FLAG;
					}
					break;
				case PDU_ERROR_FLAG:
					startTime = respTime;
					flag = PDU_REQUIRED_FLAG;
					break;
				case PDU_EXPIRED_FLAG:
					__tc__("processDeliverySm.deliveryReport.deliveryReceipt.expiredDeliveryReceipt");
					__tc_ok__;
					if (deliveryStatus == 0xffffffff) //респонс не отослан
					{
						startTime =
							max(recvTime + (time_t) (fixture->smeInfo.timeout - 1),
								monitor->getValidTime());
					}
					else
					{
						startTime = monitor->getValidTime();
					}
					flag = PDU_REQUIRED_FLAG;
					break;
				default:
					__unreachable__("Invalid monitor flag");
			}
			break;
		default:
			__unreachable__("Invalid registered delivery flag");
	}
	if (startTime)
	{
		DeliveryReceiptMonitor* m =
			new DeliveryReceiptMonitor(startTime, monitor->pduData, flag);
		m->deliveryFlag = monitor->getFlag();
		m->deliveryStatus = deliveryStatus;
		pduReg->registerMonitor(m);
	}
}

void NormalSmsHandler::registerDeliveryReportMonitors(const DeliveryMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime, time_t respTime)
{
	//intermediate notification monitor: первая попытка доставки с rescheduling
	if (!monitor->getLastAttempt())
	{
		registerIntermediateNotificationMonitor(monitor, pduReg, deliveryStatus,
			recvTime, respTime);
	}
	registerDeliveryReceiptMonitor(monitor, pduReg, deliveryStatus,
		recvTime, respTime);
}

#define __compare__(failureCode, field) \
	if (pdu.field != origPdu->field) { \
		ostringstream s1, s2; \
		s1 << (pdu.field); \
		s2 << (origPdu->field); \
		__trace2__("%s: %s != %s", #field, s1.str().c_str(), s2.str().c_str()); \
		__tc_fail__(failureCode); \
	}

#define __compareCStr__(failureCode, field) \
	if ((pdu.field && !origPdu->field) || \
		(!pdu.field && origPdu->field) || \
		(pdu.field && origPdu->field && strcmp(pdu.field, origPdu->field))) { \
		__trace2__("%s: %s != %s", #field, pdu.field, origPdu->field); \
		__tc_fail__(failureCode); \
	}

void NormalSmsHandler::processPdu(PduDeliverySm& pdu, const Address origAddr,
	time_t recvTime)
{
	__trace__("processNormalSms()");
	__decl_tc__;
	try
	{
		__tc__("processDeliverySm.normalSms");
		//в полученной pdu нет user_message_reference
		if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(1);
			throw TCException();
		}
		//перкрыть pduReg класса
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(origAddr);
		__require__(pduReg);
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		DeliveryMonitor* monitor = pduReg->getDeliveryMonitor(
			pdu.get_optional().get_userMessageReference());
		//для user_message_reference из полученной pdu
		//нет соответствующего оригинального pdu
		if (!monitor)
		{
			__tc_fail__(2);
			__trace2__("getDeliveryMonitor(): pduReg = %p, userMessageReference = %d, monitor = NULL",
				pduReg, pdu.get_optional().get_userMessageReference());
			throw TCException();
		}
		//в редких случаях delivery_sm приходит раньше submit_sm_resp
		//никак это не отслеживаю
		if (!monitor->pduData->valid)
		{
			__tc_fail__(-1);
			//throw TCException();
		}
		__tc_ok_cond__;
		__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* origPdu =
			reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
		//проверить правильность маршрута
		__tc__("processDeliverySm.normalSms.checkRoute");
		__tc_fail2__(checkRoute(*origPdu, pdu), 0);
		__tc_ok_cond__;
		//сравнить поля полученной и оригинальной pdu
		__tc__("processDeliverySm.normalSms.checkMandatoryFields");
		//поля хедера проверяются в processDeliverySm()
		//message
		__compareCStr__(1, get_message().get_serviceType());
		//правильность адресов проверяется в fixture->routeChecker->checkRouteForNormalSms()
		//__compareAddr__(get_message().get_source());
		//__compareAddr__(get_message().get_dest());
		__compare__(2, get_message().get_esmClass() & 0xfc); //без 2-ух младших битов
		if ((pdu.get_message().get_esmClass() & ESM_CLASS_MESSAGE_TYPE_BITS) !=
			ESM_CLASS_NORMAL_MESSAGE)
		{
			__tc_fail__(3);
		}
		__compare__(4, get_message().get_protocolId());
		//в действительности, priority задается маршрутом и
		//влияет на порядок доставки сообщений
		__compare__(5, get_message().get_priorityFlag());
		__compare__(6, get_message().get_registredDelivery());
		__tc_ok_cond__;
		//сравнить текст
		if (fixture->profileReg)
		{
			compareMsgText(*origPdu, pdu);
		}
		//optional
		__tc__("processDeliverySm.normalSms.checkOptionalFields");
		//отключить message_payload, который проверяется в compareMsgText()
		__tc_fail2__(SmppUtil::compareOptional(
			pdu.get_optional(), origPdu->get_optional(), OPT_MSG_PAYLOAD), 0);
		__tc_ok_cond__;
		//проверка механизма повторной доставки
		__tc__("processDeliverySm.normalSms.scheduleChecks");
		__tc_fail2__(monitor->checkSchedule(recvTime), 0);
		__tc_ok_cond__;
		//отправить респонс
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendDeliverySmResp(pdu);
		RespPduFlag respFlag = isAccepted(deliveryResp.first);
		//обновить статус delivery монитора
		__tc__("processDeliverySm.normalSms.checkAllowed");
		pduReg->removeMonitor(monitor);
		__tc_fail2__(monitor->update(recvTime, respFlag), 0);
		pduReg->registerMonitor(monitor);
		__tc_ok_cond__;
		//зарегистрировать delivery report мониторы
		registerDeliveryReportMonitors(monitor, pduReg, deliveryResp.first,
			recvTime, deliveryResp.second);
	}
	catch (TCException&)
	{
		//отправить респонс
		fixture->respSender->sendDeliverySmResp(pdu);
	}
	catch (...)
	{
		__tc_fail__(100);
		error();
	}
}

}
}
}

