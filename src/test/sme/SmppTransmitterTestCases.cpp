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
using smsc::sme::SmppInvalidBindState;
using smsc::test::conf::TestConfig;
using namespace smsc::sms; //constants
using namespace smsc::profiler; //constants, Profile
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants
using namespace smsc::smpp::DataCoding; //constants
using namespace smsc::test::smpp; //constants, SmppUtil
using namespace smsc::test::core; //constants
using namespace smsc::test::sms; //constants
using namespace smsc::test::util; //constants

Category& SmppTransmitterTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppTransmitterTestCases");
	return log;
}

void SmppTransmitterTestCases::setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu,
	const Address& destAlias, bool useShortMessage, uint64_t mask)
{
	__require__(pdu);
	SmppUtil::setupRandomCorrectSubmitSmPdu(pdu, useShortMessage,
		mask & ~OPT_USSD_SERVICE_OP);
	 //Default message Type (i.e. normal message)
	pdu->get_message().set_esmClass(
		pdu->get_message().get_esmClass() & 0xc3);
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
	__require__(pduData);
	__require__(pduData->intProps.count("registredDelivery"));
	__require__(pduData->intProps.count("reportOptions"));
	uint8_t registredDelivery = (uint8_t) pduData->intProps["registredDelivery"];
	int reportOptions = pduData->intProps["reportOptions"];
	switch (reportOptions)
	{
		case ProfileReportOptions::ReportNone:
			return (registredDelivery & SMSC_DELIVERY_RECEIPT_BITS);
		case ProfileReportOptions::ReportFull:
			return FINAL_SMSC_DELIVERY_RECEIPT;
		default:
			__unreachable__("Invalid report options");
	}
}

void SmppTransmitterTestCases::cancelMonitor(PduMonitor* monitor,
	time_t cancelTime, bool forceRemoveMonitor)
{
	__require__(monitor);
	__cfg_int__(timeCheckAccuracy);
	switch (monitor->getFlag())
	{
		case PDU_REQUIRED_FLAG:
		case PDU_COND_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			fixture->pduReg->removeMonitor(monitor);
			if (abs(monitor->getCheckTime() - cancelTime) <= timeCheckAccuracy)
			{
				__require__(!forceRemoveMonitor);
				monitor->setCondRequired();
			}
			else
			{
				monitor->setNotExpected();
			}
			if (forceRemoveMonitor)
			{
				delete monitor;
			}
			else
			{
				fixture->pduReg->registerMonitor(monitor);
			}
			break;
		case PDU_NOT_EXPECTED_FLAG:
			if (forceRemoveMonitor)
			{
				fixture->pduReg->removeMonitor(monitor);
				delete monitor;
			}
			break;
		default:
			__unreachable__("Invalid monitor flag");
	}
}

uint16_t SmppTransmitterTestCases::cancelPduMonitors(PduData* pduData,
	time_t cancelTime, bool forceRemoveMonitors, State state)
{
	__require__(pduData && pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	__require__(pdu->get_optional().has_userMessageReference());
	uint16_t msgRef = pdu->get_optional().get_userMessageReference();
	//delivery monitor
	DeliveryMonitor* deliveryMonitor = fixture->pduReg->getDeliveryMonitor(msgRef);
	__require__(deliveryMonitor);
	deliveryMonitor->state = state;
	cancelMonitor(deliveryMonitor, cancelTime, forceRemoveMonitors);
	//delivery receipt monitor
	DeliveryReceiptMonitor* rcptMonitor =
		fixture->pduReg->getDeliveryReceiptMonitor(msgRef);
	if (rcptMonitor)
	{
		//���� ���� delivery receipt monitor, �� pdu ��� ����������������
		__require__(deliveryMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
		rcptMonitor->state = state;
		cancelMonitor(rcptMonitor, cancelTime, forceRemoveMonitors);
	}
	//intermediate notification monitor
	IntermediateNotificationMonitor* notifMonitor =
		fixture->pduReg->getIntermediateNotificationMonitor(msgRef);
	if (notifMonitor)
	{
		notifMonitor->state = state;
		cancelMonitor(notifMonitor, cancelTime, forceRemoveMonitors);
	}
	return msgRef;
}

void SmppTransmitterTestCases::registerTransmitterReportMonitors(uint16_t msgRef,
	time_t waitTime, time_t validTime, PduData* pduData)
{
	__decl_tc__;
	if (fixture->smeType == SME_TRANSMITTER)
	{
		return;
	}
	uint8_t regDelivery = getRegisteredDelivery(pduData);
	if (regDelivery == FINAL_SMSC_DELIVERY_RECEIPT ||
		regDelivery == FAILURE_SMSC_DELIVERY_RECEIPT)
	{
		if (pduData->intProps.count("ussdServiceOp"))
		{
			__tc__("deliverySm.reports.deliveryReceipt.ussdServiceOp");
			__tc_ok__;
			__tc__("deliverySm.reports.intermediateNotification.ussdServiceOp");
			__tc_ok__;
		}
		else
		{
			//intermediate notification
			__tc__("deliverySm.reports.intermediateNotification.transmitter");
			__tc_ok__;
			IntermediateNotificationMonitor* notifMonitor =
				new IntermediateNotificationMonitor(msgRef, waitTime, pduData, PDU_REQUIRED_FLAG);
			notifMonitor->state = ENROUTE;
			notifMonitor->deliveryStatus = DELIVERY_STATUS_DEST_TRANSMITTER;
			fixture->pduReg->registerMonitor(notifMonitor);
			//delivery receipt
			__tc__("deliverySm.reports.deliveryReceipt.transmitter");
			__tc_ok__;
			DeliveryReceiptMonitor* rcptMonitor =
				new DeliveryReceiptMonitor(msgRef, validTime, pduData, PDU_REQUIRED_FLAG);
			rcptMonitor->state = EXPIRED;
			rcptMonitor->deliveryStatus = DELIVERY_STATUS_DEST_TRANSMITTER;
			fixture->pduReg->registerMonitor(rcptMonitor);
		}
	}
}

void SmppTransmitterTestCases::registerNotBoundReportMonitors(uint16_t msgRef,
	time_t waitTime, time_t validTime, PduData* pduData)
{
	__decl_tc__;
	if (fixture->smeType == SME_TRANSMITTER)
	{
		return;
	}
	uint8_t regDelivery = getRegisteredDelivery(pduData);
	if (regDelivery == FINAL_SMSC_DELIVERY_RECEIPT ||
		regDelivery == FAILURE_SMSC_DELIVERY_RECEIPT)
	{
		if (pduData->intProps.count("ussdServiceOp"))
		{
			__tc__("deliverySm.reports.deliveryReceipt.ussdServiceOp");
			__tc_ok__;
			__tc__("deliverySm.reports.intermediateNotification.ussdServiceOp");
			__tc_ok__;
		}
		else
		{
			//intermediate notification
			__tc__("deliverySm.reports.intermediateNotification.notBound");
			__tc_ok__;
			IntermediateNotificationMonitor* notifMonitor =
				new IntermediateNotificationMonitor(msgRef, waitTime, pduData, PDU_REQUIRED_FLAG);
			notifMonitor->state = ENROUTE;
			notifMonitor->deliveryStatus = DELIVERY_STATUS_DEST_NOT_BOUND;
			fixture->pduReg->registerMonitor(notifMonitor);
			//delivery receipt
			__tc__("deliverySm.reports.deliveryReceipt.notBound");
			__tc_ok__;
			DeliveryReceiptMonitor* rcptMonitor =
				new DeliveryReceiptMonitor(msgRef, validTime, pduData, PDU_REQUIRED_FLAG);
			rcptMonitor->state = EXPIRED;
			rcptMonitor->deliveryStatus = DELIVERY_STATUS_DEST_NOT_BOUND;
			fixture->pduReg->registerMonitor(rcptMonitor);
		}
	}
}

void SmppTransmitterTestCases::registerNormalSmeMonitors(PduSubmitSm* pdu,
	PduData* existentPduData, uint16_t msgRef, time_t waitTime, time_t validTime,
	PduData* pduData)
{
	bool deliveryFlag = false;
	bool transmitterReportsFlag = false;
	bool notBoundReportsFlag = false;
	SmeType destType = fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest());
	switch (destType)
	{
		case SME_NO_ROUTE:
			//����� ������� � ����� ������
			return;
		case SME_TRANSMITTER:
			transmitterReportsFlag = true;
			break;
		case SME_NOT_BOUND:
			notBoundReportsFlag = true;
			break;
		case SME_RECEIVER:
		case SME_TRANSCEIVER:
			deliveryFlag = true;
			break;
		default:
			__unreachable__("Invalid sme type");
	}
	//delivery monitor
	if (deliveryFlag)
	{
		Address srcAddr, destAddr;
		SmppUtil::convert(pdu->get_message().get_source(), &srcAddr);
		SmppUtil::convert(pdu->get_message().get_dest(), &destAddr);
		DeliveryMonitor* deliveryMonitor = new DeliveryMonitor(srcAddr, destAddr,
			nvl(pdu->get_message().get_serviceType()), msgRef, waitTime, validTime,
			pduData, PDU_REQUIRED_FLAG);
		if (existentPduData && existentPduData->pdu->get_commandId() == SUBMIT_SM)
		{
			//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
			//destination address and service_type. ��������� ������ ���� � 
			//ENROTE state.
			//data_sm � ���� �� source address, destination address �
			//service_type ���������� �� ������!!!
			PduSubmitSm* existentPdu =
				reinterpret_cast<PduSubmitSm*>(existentPduData->pdu);
			if (!strcmp(nvl(pdu->get_message().get_serviceType()),
					nvl(existentPdu->get_message().get_serviceType())) &&
				pdu->get_message().get_source() ==
					existentPdu->get_message().get_source() &&
				pdu->get_message().get_dest() ==
					existentPdu->get_message().get_dest())
			{
				if (pdu->get_message().get_replaceIfPresentFlag() == 1)
				{
					existentPduData->ref();
					deliveryMonitor->pduData->replacePdu = existentPduData;
					existentPduData->replacedByPdu = pduData;
					uint16_t cancelMsgRef =
						cancelPduMonitors(existentPduData, pduData->sendTime, false, ENROUTE);
					__trace2__("replaced pdu:\n\tuserMessageReference = %d", (int) cancelMsgRef);
				}
				else
				{
					deliveryMonitor->pduData->intProps["hasSmppDuplicates"] = 1;
					existentPduData->intProps["hasSmppDuplicates"] = 1;
				}
			}
		}
		fixture->pduReg->registerMonitor(deliveryMonitor);
		//������ �������� ����� ������� � NormalSmsHandler::registerDeliveryReportMonitors()
		return;
	}
	//�������
	if (transmitterReportsFlag)
	{
		registerTransmitterReportMonitors(msgRef, waitTime, validTime, pduData);
	}
	if (notBoundReportsFlag)
	{
		registerNotBoundReportMonitors(msgRef, waitTime, validTime, pduData);
	}
}

void SmppTransmitterTestCases::registerExtSmeMonitors(PduSubmitSm* pdu,
	uint16_t msgRef, time_t waitTime, time_t validTime, PduData* pduData)
{
	__decl_tc__;
	if (fixture->smeType == SME_TRANSMITTER)
	{
		return;
	}
	//�����������, ��� ext sme ������ �������� � �� ���� ���� �������
	//ext sme ������ ���������� sme ack �, �� ��������, final delivery receipt
	__require__(fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest()) == SME_TRANSCEIVER);
	//sme ack monitor
	SmeAckMonitor* smeAckMonitor =
		new SmeAckMonitor(msgRef, waitTime, pduData, PDU_REQUIRED_FLAG);
	fixture->pduReg->registerMonitor(smeAckMonitor);
	//delivery receipt monitor
	uint8_t regDelivery = getRegisteredDelivery(pduData);
	if (regDelivery == FINAL_SMSC_DELIVERY_RECEIPT)
	{
		if (pduData->intProps.count("ussdServiceOp"))
		{
			__tc__("deliverySm.reports.deliveryReceipt.ussdServiceOp");
			__tc_ok__;
		}
		else
		{
			DeliveryReceiptMonitor* rcptMonitor =
				new DeliveryReceiptMonitor(msgRef, waitTime, pduData, PDU_REQUIRED_FLAG);
			rcptMonitor->state = DELIVERED;
			rcptMonitor->deliveryStatus = ESME_ROK;
			fixture->pduReg->registerMonitor(rcptMonitor);
		}
	}
}

void SmppTransmitterTestCases::registerNullSmeMonitors(PduSubmitSm* pdu,
	uint16_t msgRef, time_t waitTime, time_t validTime, uint32_t deliveryStatus,
	PduData* pduData)
{
	__decl_tc__;
	if (fixture->smeType == SME_TRANSMITTER)
	{
		return;
	}
	//�����������, ��� null sme ������ �������� � �� ���� ���� �������
	//null sme �� ���������� sme ack, � �� deliver_sm ����� ���������� �������
	__require__(fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest()) == SME_TRANSCEIVER);
	//delivery receipt monitor
	uint8_t regDelivery = getRegisteredDelivery(pduData);
	if (regDelivery == FINAL_SMSC_DELIVERY_RECEIPT ||
		(regDelivery == FAILURE_SMSC_DELIVERY_RECEIPT && deliveryStatus != ESME_ROK))
	{
		if (pduData->intProps.count("ussdServiceOp"))
		{
			__tc__("deliverySm.reports.deliveryReceipt.ussdServiceOp");
			__tc_ok__;
		}
		else
		{
			DeliveryReceiptMonitor* rcptMonitor = 
				new DeliveryReceiptMonitor(msgRef, waitTime, pduData, PDU_REQUIRED_FLAG);
			rcptMonitor->state = deliveryStatus == ESME_ROK ? DELIVERED : UNDELIVERABLE;
			rcptMonitor->deliveryStatus = deliveryStatus;
			fixture->pduReg->registerMonitor(rcptMonitor);
		}
	}
}

//��������������� ����������� pdu, ��������� ������� �������������
PduData* SmppTransmitterTestCases::prepareSubmitSm(PduSubmitSm* pdu,
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
	time_t t; //������� ����� ����� ������ ���, ��� � profileReg, ��������� profileUpdateTime
	Profile profile = fixture->profileReg->getProfile(fixture->smeAddr, t);
	__require__(t <= submitTime); //� ��������� �� �������
	//pdu data
	pdu->get_header().set_commandId(SUBMIT_SM); //������������� � ������ �������� submit_sm
	PduData* pduData = new PduData(reinterpret_cast<SmppHeader*>(pdu),
		submitTime, intProps, strProps, objProps);
	pduData->intProps["registredDelivery"] = pdu->get_message().get_registredDelivery();
	pduData->intProps["reportOptions"] = profile.reportoptions;
	if (pdu->get_optional().has_ussdServiceOp())
	{
		pduData->intProps["ussdServiceOp"] = pdu->get_optional().get_ussdServiceOp();
	}
	pduData->checkRes = fixture->pduChecker->checkSubmitSm(pduData);
	pduData->ref();
	//��������� ������� ������ (����� �� ������� � ����� ������)
	if (pduData->checkRes.size())
	{
		return pduData;
	}
	//response monitor �������������� ����� ������ �������� seqNum
	switch (pduType)
	{
		case PDU_NORMAL:
			//����������� DeliveryMonitor � ��� sme, �� ������� ���� �������,
			//�� ��� ����� sme DeliveryReceiptMonitor � IntermediateNotificationMonitor
			registerNormalSmeMonitors(pdu, existentPduData, msgRef, waitTime,
				validTime, pduData);
			break;
		case PDU_EXT_SME:
			//����������� SmeAckMonitor � DeliveryReceiptMonitor
			registerExtSmeMonitors(pdu, msgRef, waitTime, validTime, pduData);
			break;
		case PDU_NULL_OK:
			//����������� ������ DeliveryReceiptMonitor
			registerNullSmeMonitors(pdu, msgRef, waitTime, validTime, ESME_ROK, pduData);
			break;
		case PDU_NULL_ERR:
			//����������� ������ DeliveryReceiptMonitor
			registerNullSmeMonitors(pdu, msgRef, waitTime, validTime, ESME_RX_P_APPN, pduData);
			break;
		default:
			__unreachable__("Invalid pdu type");
	}
	return pduData;
}

//�������� smsId ��� deliver receipt �������� � PduRegistry � ��������� pdu
//��������� ������� �������������
void SmppTransmitterTestCases::processSubmitSmSync(PduData* pduData,
	PduSubmitSmResp* respPdu, time_t respTime)
{
	__require__(pduData);
	__dumpPdu__("processSubmitSmRespSync", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(respPdu));
	__decl_tc__;
	__tc__("submitSm.resp.sync");
	if (!respPdu)
	{
		__tc_fail__(1);
	}
	else
	{
		//�������, �� �� �������������� ������� �������
		ResponseMonitor monitor(pduData->pdu->get_sequenceNumber(),
			pduData->sendTime, pduData, PDU_REQUIRED_FLAG);
		fixture->pduChecker->processSubmitSmResp(&monitor, *respPdu, respTime);
		delete respPdu; //disposePdu
	}
	__tc_ok_cond__;
	pduData->unref();
}

//���������������� ������� �������, ��������� ������� �������������
void SmppTransmitterTestCases::processSubmitSmAsync(PduData* pduData)
{
	__require__(fixture->pduReg);
	__require__(pduData);
	//������� � ���������������� ������� �������
	ResponseMonitor* monitor =
		new ResponseMonitor(pduData->pdu->get_sequenceNumber(),
			pduData->sendTime, pduData, PDU_REQUIRED_FLAG);
	fixture->pduReg->registerMonitor(monitor);
	pduData->unref();
}

//��������� � ���������������� pdu
void SmppTransmitterTestCases::sendSubmitSmPdu(PduSubmitSm* pdu,
	PduData* existentPduData, bool sync, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps, PduType pduType)
{
	if (fixture->smeType == SME_RECEIVER)
	{
		__decl_tc__;
		try
		{
			__tc__("submitSm.receiver");
			if (sync)
			{
				fixture->session->getSyncTransmitter()->submit(*pdu);
			}
			else
			{
				fixture->session->getAsyncTransmitter()->submit(*pdu);
			}
			__tc_fail__(1);
		}
		catch (SmppInvalidBindState&)
		{
			__tc_ok__;
			return;
		}
		throw Exception("Missing invalid bind exception");
	}
	__decl_tc12__;
	try
	{
		switch (fixture->smeType)
		{
			case SME_TRANSMITTER:
				__tc1__("submitSm.transmitter");
				break;
			case SME_TRANSCEIVER:
				__tc1__("submitSm.transceiver");
				break;
			default:
				__unreachable__("Invalid sme type");
		}
		if (fixture->pduReg)
		{
			if (sync)
			{
				__tc2__("submitSm.sync");
				PduData* pduData;
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					pdu->get_header().set_sequenceNumber(0); //�� ��������
					pduData = prepareSubmitSm(pdu, existentPduData, time(NULL),
						intProps, strProps, objProps, pduType); //all times, msgRef
				}
				//__dumpPdu__("submitSmSyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				PduSubmitSmResp* respPdu =
					fixture->session->getSyncTransmitter()->submit(*pdu);
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					__dumpPdu__("submitSmSyncAfter", fixture->smeInfo.systemId,
						reinterpret_cast<SmppHeader*>(pdu));
					processSubmitSmSync(pduData, respPdu, time(NULL));
				}
			}
			else
			{
				__tc2__("submitSm.async");
				MutexGuard mguard(fixture->pduReg->getMutex());
				//__dumpPdu__("submitSmAsyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				time_t submitTime = time(NULL);
				PduSubmitSmResp* respPdu =
					fixture->session->getAsyncTransmitter()->submit(*pdu);
				__require__(!respPdu);
				time_t responseTime = time(NULL);
				__dumpPdu__("submitSmAsyncAfter", fixture->smeInfo.systemId,
					reinterpret_cast<SmppHeader*>(pdu));
				PduData* pduData = prepareSubmitSm(pdu, existentPduData,
					submitTime, intProps, strProps, objProps, pduType); //all times, msgRef, sequenceNumber
				processSubmitSmAsync(pduData);
			}
			//pdu life time ������������ PduRegistry
			//disposePdu(pdu);
		}
		else
		{
			if (sync)
			{
				__tc2__("submitSm.sync");
				PduSubmitSmResp* respPdu =
					fixture->session->getSyncTransmitter()->submit(*pdu);
				if (respPdu)
				{
					delete respPdu; //disposePdu
				}
			}
			else
			{
				__tc2__("submitSm.async");
				fixture->session->getAsyncTransmitter()->submit(*pdu);
			}
			delete pdu; //disposePdu
		}
		__tc12_ok_cond__;
	}
	catch (...)
	{
		__tc12_fail__(100);
		//error();
		throw;
	}
}

void SmppTransmitterTestCases::setupRandomCorrectReplaceSmPdu(PduReplaceSm* pdu,
	PduData* replacePduData)
{
	__require__(replacePduData && replacePduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* origPdu = reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
	uint8_t dataCoding = origPdu->get_message().get_dataCoding();
	bool udhi = origPdu->get_message().get_esmClass() & ESM_CLASS_UDHI_INDICATOR;
	SmppUtil::setupRandomCorrectReplaceSmPdu(pdu, dataCoding, udhi);
	//source
	PduAddress srcAddr;
	SmppUtil::convert(fixture->smeAddr, &srcAddr);
	pdu->set_source(srcAddr);
	//messageId
	__require__(replacePduData->strProps.count("smsId"));
	pdu->set_messageId(replacePduData->strProps["smsId"].c_str());
}

void SmppTransmitterTestCases::registerReplaceMonitors(PduSubmitSm* resPdu,
	PduData* replacePduData, PduData* pduData)
{
	uint16_t msgRef = resPdu->get_optional().get_userMessageReference();
	time_t waitTime = max(pduData->sendTime, SmppUtil::getWaitTime(
			resPdu->get_message().get_scheduleDeliveryTime(), pduData->sendTime));
	time_t validTime = SmppUtil::getValidTime(
		resPdu->get_message().get_validityPeriod(), pduData->sendTime);
	//��������
	bool deliveryFlag = false;
	bool transmitterReportsFlag = false;
	bool notBoundReportsFlag = false;
	SmeType destType = fixture->routeChecker->isDestReachable(
		resPdu->get_message().get_source(), resPdu->get_message().get_dest());
	switch (destType)
	{
		case SME_TRANSMITTER: //��������� sms, ��� ������� ��� �� ���� ������� ��������
			transmitterReportsFlag = true;
			break;
		case SME_NOT_BOUND:
			notBoundReportsFlag = true;
			break;
		case SME_RECEIVER:
		case SME_TRANSCEIVER:
			deliveryFlag = true;
			break;
		default: //SME_NO_ROUTE
			__unreachable__("Invalid sme type");
	}
	uint16_t cancelMsgRef =
		cancelPduMonitors(replacePduData, pduData->sendTime, true, ENROUTE);
	__trace2__("replaced pdu:\n\tuserMessageReference = %d", (int) cancelMsgRef);
	//delivery monitor
	if (deliveryFlag)
	{
		Address srcAddr, destAddr;
		SmppUtil::convert(resPdu->get_message().get_source(), &srcAddr);
		SmppUtil::convert(resPdu->get_message().get_dest(), &destAddr);
		__require__(resPdu->get_optional().has_userMessageReference());
		DeliveryMonitor* deliveryMonitor = new DeliveryMonitor(srcAddr, destAddr,
			nvl(resPdu->get_message().get_serviceType()), msgRef, waitTime,
			validTime, pduData, PDU_REQUIRED_FLAG);
		replacePduData->ref();
		pduData->replacePdu = replacePduData;
		replacePduData->replacedByPdu = pduData;
		fixture->pduReg->registerMonitor(deliveryMonitor);
		//�������� ����� ������� � NormalSmsHandler::registerDeliveryReportMonitors()
		return;
	}
	//�������
	if (transmitterReportsFlag)
	{
		registerTransmitterReportMonitors(msgRef, waitTime, validTime, pduData);
	}
	if (notBoundReportsFlag)
	{
		registerNotBoundReportMonitors(msgRef, waitTime, validTime, pduData);
	}
}

//��������������� ����������� pdu, ��������� ������� �������������
PduData* SmppTransmitterTestCases::prepareReplaceSm(PduReplaceSm* pdu,
	PduData* replacePduData, time_t submitTime, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps)
{
	//�������� pdu, ������� ���������� ����� ���������
	PduFlag replacePduFlag = PDU_NOT_EXPECTED_FLAG;
	PduSubmitSm* resPdu;
	if (replacePduData)
	{
		__require__(replacePduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* replacePdu =
			reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
		resPdu = reinterpret_cast<PduSubmitSm*>(
			SmppUtil::copyPdu(replacePduData->pdu));
		__require__(resPdu);
		if (pdu->get_scheduleDeliveryTime())
		{
			resPdu->get_message().set_scheduleDeliveryTime(
				pdu->get_scheduleDeliveryTime());
		}
		else
		{
			SmppTime t;
			time_t waitTime = SmppUtil::getWaitTime(
				replacePdu->get_message().get_scheduleDeliveryTime(), replacePduData->sendTime);
			resPdu->get_message().set_scheduleDeliveryTime(
				SmppUtil::time2string(waitTime, t, submitTime, __absoluteTime__));
		}
		if (pdu->get_validityPeriod())
		{
			resPdu->get_message().set_validityPeriod(pdu->get_validityPeriod());
		}
		else
		{
			SmppTime t;
			time_t validTime = SmppUtil::getValidTime(
				replacePdu->get_message().get_validityPeriod(), replacePduData->sendTime);
			resPdu->get_message().set_validityPeriod(
				SmppUtil::time2string(validTime, t, submitTime, __absoluteTime__));
		}
		DeliveryMonitor* monitor = fixture->pduReg->getDeliveryMonitor(
			replacePdu->get_optional().get_userMessageReference());
		if (monitor)
		{
			replacePduFlag = monitor->getFlag();
		}
	}
	else
	{
		resPdu = new PduSubmitSm();
		resPdu->get_message().set_scheduleDeliveryTime(
			pdu->get_scheduleDeliveryTime() ? pdu->get_scheduleDeliveryTime() : "");
		resPdu->get_message().set_validityPeriod(
			pdu->get_validityPeriod() ? pdu->get_validityPeriod() : "");
	}
	resPdu->get_message().set_source(pdu->get_source());
	resPdu->get_message().set_registredDelivery(pdu->get_registredDelivery());
	resPdu->get_message().set_smDefaultMsgId(pdu->get_smDefaultMsgId());
	resPdu->get_message().set_shortMessage(pdu->get_shortMessage(), pdu->size_shortMessage());
	//report options
	Address srcAddr;
	SmppUtil::convert(pdu->get_source(), &srcAddr);
	time_t t; //������� ����� ����� ������ ���, ��� � profileReg, ��������� profileUpdateTime
	Profile profile = fixture->profileReg->getProfile(srcAddr, t);
	__require__(t <= submitTime); //� ��������� �� �������
	resPdu->get_header().set_commandId(SUBMIT_SM);
	PduData* pduData = new PduData(reinterpret_cast<SmppHeader*>(resPdu),
		submitTime, intProps, strProps, objProps);
	pduData->intProps["registredDelivery"] = pdu->get_registredDelivery();
	pduData->intProps["reportOptions"] = profile.reportoptions;
	if (resPdu->get_optional().has_ussdServiceOp())
	{
		pduData->intProps["ussdServiceOp"] = resPdu->get_optional().get_ussdServiceOp();
	}
	pduData->strProps["smsId"] = nvl(pdu->get_messageId());
	//��������� ������� ������ (����� �� ������� � ����� ������)
	pduData->checkRes = fixture->pduChecker->checkReplaceSm(pduData,
		replacePduData, replacePduFlag);
	pduData->ref();
	if (pduData->checkRes.size())
	{
		return pduData;
	}
	registerReplaceMonitors(resPdu, replacePduData, pduData);
	//������� ������� � ������ �������� �������������� �� ����
	return pduData;
}

//�������� sequenceNumber � PduRegistry � ��������� pdu
//��������� ������� �������������
void SmppTransmitterTestCases::processReplaceSmSync(PduData* pduData,
	PduReplaceSm* pdu, PduReplaceSmResp* respPdu, time_t respTime)
{
	__require__(pduData && pdu);
	__dumpPdu__("processReplaceSmRespSync", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(respPdu));
	__decl_tc__;
	//�������� sequenceNumber � �������� pdu
	pduData->pdu->set_sequenceNumber(pdu->get_header().get_sequenceNumber());
	__tc__("replaceSm.resp.sync");
	if (!respPdu)
	{
		__tc_fail__(1);
	}
	else
	{
		//�������, �� �� �������������� ������� �������
		ResponseMonitor monitor(pduData->pdu->get_sequenceNumber(),
			pduData->sendTime, pduData, PDU_REQUIRED_FLAG);
		fixture->pduChecker->processReplaceSmResp(&monitor, *respPdu, respTime);
		delete respPdu; //disposePdu
	}
	__tc_ok_cond__;
	pduData->unref();
}

//�������� sequenceNumber � PduRegistry, ��������� ������� �������������
void SmppTransmitterTestCases::processReplaceSmAsync(PduData* pduData,
	PduReplaceSm* pdu)
{
	__require__(fixture->pduReg);
	__require__(pduData && pdu);
	//�������� sequenceNumber � �������� pdu
	pduData->pdu->set_sequenceNumber(pdu->get_header().get_sequenceNumber());
	//������� � ���������������� ������� �������
	ResponseMonitor* monitor =
		new ResponseMonitor(pduData->pdu->get_sequenceNumber(),
			pduData->sendTime, pduData, PDU_REQUIRED_FLAG);
	fixture->pduReg->registerMonitor(monitor);
	pduData->unref();
}

//��������� � ���������������� pdu
void SmppTransmitterTestCases::sendReplaceSmPdu(PduReplaceSm* pdu,
	PduData* replacePduData, bool sync, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps)
{
	if (fixture->smeType == SME_RECEIVER)
	{
		__decl_tc__;
		try
		{
			__tc__("replaceSm.receiver");
			if (sync)
			{
				fixture->session->getSyncTransmitter()->replace(*pdu);
			}
			else
			{
				fixture->session->getAsyncTransmitter()->replace(*pdu);
			}
			__tc_fail__(1);
		}
		catch (SmppInvalidBindState&)
		{
			__tc_ok__;
			return;
		}
		throw Exception("Missing invalid bind exception");
	}
	__decl_tc12__;
	try
	{
		switch (fixture->smeType)
		{
			case SME_TRANSMITTER:
				__tc1__("replaceSm.transmitter");
				break;
			case SME_TRANSCEIVER:
				__tc1__("replaceSm.transceiver");
				break;
			default:
				__unreachable__("Invalid pdu type");
		}
		if (fixture->pduReg)
		{
			if (sync)
			{
				__tc2__("replaceSm.sync");
				PduData* pduData;
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					pdu->get_header().set_sequenceNumber(0); //�� ��������
					pduData = prepareReplaceSm(pdu, replacePduData, time(NULL),
						intProps, strProps, objProps);
				}
				//__dumpPdu__("replaceSmSyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				PduReplaceSmResp* respPdu =
					fixture->session->getSyncTransmitter()->replace(*pdu);
				__dumpPdu__("replaceSmSyncAfter", fixture->smeInfo.systemId,
					reinterpret_cast<SmppHeader*>(pdu));
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					processReplaceSmSync(pduData, pdu, respPdu, time(NULL));
				}
			}
			else
			{
				__tc2__("replaceSm.async");
				MutexGuard mguard(fixture->pduReg->getMutex());
				//__dumpPdu__("replaceSmAsyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				time_t submitTime = time(NULL);
				PduReplaceSmResp* respPdu =
					fixture->session->getAsyncTransmitter()->replace(*pdu);
				__dumpPdu__("replaceSmAsyncAfter", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				PduData* pduData = prepareReplaceSm(pdu, replacePduData,
					submitTime, intProps, strProps, objProps);
				processReplaceSmAsync(pduData, pdu);
			}
			//pdu life time ������������ PduRegistry
			//disposePdu(pdu);
		}
		else
		{
			if (sync)
			{
				__tc2__("replaceSm.sync");
				PduReplaceSmResp* respPdu =
					fixture->session->getSyncTransmitter()->replace(*pdu);
				if (respPdu)
				{
					delete respPdu; //disposePdu
				}
			}
			else
			{
				__tc2__("replaceSm.async");
				fixture->session->getAsyncTransmitter()->replace(*pdu);
			}
			delete pdu; //disposePdu
		}
		__tc12_ok__;
	}
	catch (...)
	{
		__tc12_fail__(100);
		//error();
		throw;
	}
}

PduData* SmppTransmitterTestCases::prepareQuerySm(PduQuerySm* pdu,
	PduData* origPduData, time_t queryTime, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps)
{
	PduData* pduData = new PduData(reinterpret_cast<SmppHeader*>(pdu),
		queryTime, intProps, strProps, objProps);
	pduData->checkRes = fixture->pduChecker->checkQuerySm(pduData, origPduData);
	pduData->ref();
	return pduData;
}

void SmppTransmitterTestCases::processQuerySmSync(PduData* pduData,
	PduQuerySmResp* respPdu, time_t respTime)
{
	__require__(pduData);
	__dumpPdu__("processQuerySmRespSync", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(respPdu));
	__decl_tc__;
	__tc__("querySm.resp.sync");
	if (!respPdu)
	{
		__tc_fail__(1);
	}
	else
	{
		//�������, �� �� �������������� ������� �������
		ResponseMonitor monitor(pduData->pdu->get_sequenceNumber(),
			pduData->sendTime, pduData, PDU_REQUIRED_FLAG);
		fixture->pduChecker->processQuerySmResp(&monitor, *respPdu, respTime);
		delete respPdu; //disposePdu
	}
	__tc_ok_cond__;
	pduData->unref();
}

//�������� sequenceNumber � PduRegistry, ��������� ������� �������������
void SmppTransmitterTestCases::processQuerySmAsync(PduData* pduData)
{
	__require__(fixture->pduReg);
	__require__(pduData);
	//������� � ���������������� ������� �������
	ResponseMonitor* monitor =
		new ResponseMonitor(pduData->pdu->get_sequenceNumber(),
			pduData->sendTime, pduData, PDU_REQUIRED_FLAG);
	fixture->pduReg->registerMonitor(monitor);
	pduData->unref();
}

void SmppTransmitterTestCases::sendQuerySmPdu(PduQuerySm* pdu,
	PduData* origPduData, bool sync, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps)
{
	if (fixture->smeType == SME_RECEIVER)
	{
		__decl_tc__;
		try
		{
			__tc__("querySm.receiver");
			if (sync)
			{
				fixture->session->getSyncTransmitter()->query(*pdu);
			}
			else
			{
				fixture->session->getAsyncTransmitter()->query(*pdu);
			}
			__tc_fail__(1);
		}
		catch (SmppInvalidBindState&)
		{
			__tc_ok__;
			return;
		}
		throw Exception("Missing invalid bind exception");
	}
	__decl_tc12__;
	try
	{
		switch (fixture->smeType)
		{
			case SME_TRANSMITTER:
				__tc1__("querySm.transmitter");
				break;
			case SME_TRANSCEIVER:
				__tc1__("querySm.transceiver");
				break;
			default:
				__unreachable__("Invalid sme type");
		}
		if (fixture->pduReg)
		{
			if (sync)
			{
				__tc2__("querySm.sync");
				PduData* pduData;
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					pdu->get_header().set_sequenceNumber(0); //�� ��������
					pduData = prepareQuerySm(pdu, origPduData, time(NULL),
						intProps, strProps, objProps);
				}
				//__dumpPdu__("querySmSyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				PduQuerySmResp* respPdu =
					fixture->session->getSyncTransmitter()->query(*pdu);
				__dumpPdu__("querySmSyncAfter", fixture->smeInfo.systemId,
					reinterpret_cast<SmppHeader*>(pdu));
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					processQuerySmSync(pduData, respPdu, time(NULL));
				}
			}
			else
			{
				__tc2__("querySm.async");
				MutexGuard mguard(fixture->pduReg->getMutex());
				//__dumpPdu__("querySmAsyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				time_t queryTime = time(NULL);
				PduQuerySmResp* respPdu =
					fixture->session->getAsyncTransmitter()->query(*pdu);
				__dumpPdu__("querySmAsyncAfter", fixture->smeInfo.systemId,
					reinterpret_cast<SmppHeader*>(pdu));
				PduData* pduData = prepareQuerySm(pdu, pduData,
					queryTime, intProps, strProps, objProps);
				processQuerySmAsync(pduData);
			}
			//pdu life time ������������ PduRegistry
			//disposePdu(pdu);
		}
		else
		{
			if (sync)
			{
				__tc2__("querySm.sync");
				PduQuerySmResp* respPdu =
					fixture->session->getSyncTransmitter()->query(*pdu);
				if (respPdu)
				{
					delete respPdu; //disposePdu
				}
			}
			else
			{
				__tc2__("querySm.async");
				fixture->session->getAsyncTransmitter()->query(*pdu);
			}
			delete pdu; //disposePdu
		}
		__tc12_ok__;
	}
	catch (...)
	{
		__tc12_fail__(100);
		//error();
		throw;
	}
}

void SmppTransmitterTestCases::processCancelledMonitors(PduCancelSm* pdu,
	PduData* cancelPduData, time_t cancelTime, PduData* pduData)
{
	__require__(fixture->pduReg);
	//��������� cancel ��� sms-��
	if (pdu->get_messageId())
	{
		__require__(!pdu->get_serviceType());
		__require__(cancelPduData);
		uint16_t cancelMsgRef =
			cancelPduMonitors(cancelPduData, cancelTime, false, DELETED);
		__trace2__("cancelled pdu:\n\tuserMessageReference = %d", (int) cancelMsgRef);
	}
	else
	{
		Address srcAddr, destAddr;
		SmppUtil::convert(pdu->get_source(), &srcAddr);
		SmppUtil::convert(pdu->get_dest(), &destAddr);
		PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(0, LONG_MAX);
		//����������� ������, �� ����� �� ������ cancel, ����� �� ��������� ��������
		vector<PduData*> cancelPduDataList;
		while (PduMonitor* m = it->next())
		{
			if (m->getType() == DELIVERY_MONITOR
				/*&& m->getFlag() != PDU_NOT_EXPECTED_FLAG*/)
			//����� ���� delivery ��������, ������� ��� expired, �� vliadity_period
			//��� �� �������� � report �������� required
			{
				DeliveryMonitor* monitor = dynamic_cast<DeliveryMonitor*>(m);
				if (monitor->srcAddr == srcAddr && monitor->destAddr == destAddr &&
					(!pdu->get_serviceType() || monitor->serviceType == pdu->get_serviceType()))
				{
					cancelPduDataList.push_back(monitor->pduData);
				}
			}
		}
		delete it;
		__trace__("cancelled pdus:\n");
		for (int i = 0; i < cancelPduDataList.size(); i++)
		{
			uint16_t cancelMsgRef =
				cancelPduMonitors(cancelPduDataList[i], cancelTime, false, DELETED);
			__trace2__("\tuserMessageReference = %d\n", (int) cancelMsgRef);
		}
	}
}

//��������������� ����������� pdu, ��������� ������� �������������
PduData* SmppTransmitterTestCases::prepareCancelSm(PduCancelSm* pdu,
	PduData* cancelPduData, time_t cancelTime, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps)
{
	//cancelPduFlag
	PduFlag cancelPduFlag = PDU_NOT_EXPECTED_FLAG;
	if (cancelPduData)
	{
		__require__(cancelPduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* cancelPdu = reinterpret_cast<PduSubmitSm*>(cancelPduData->pdu);
		__require__(cancelPdu->get_optional().has_userMessageReference());
		DeliveryMonitor* monitor = fixture->pduReg->getDeliveryMonitor(
			cancelPdu->get_optional().get_userMessageReference());
		if (monitor)
		{
			cancelPduFlag = monitor->getFlag();
		}
	}
	PduData* pduData = new PduData(reinterpret_cast<SmppHeader*>(pdu),
		cancelTime, intProps, strProps, objProps);
	//��������� ������� ������ (����� �� ������� � ����� ������)
	pduData->checkRes = fixture->pduChecker->checkCancelSm(pduData,
		cancelPduData, cancelPduFlag);
	pduData->ref();
	if (!pduData->checkRes.size())
	{
		processCancelledMonitors(pdu, cancelPduData, cancelTime, pduData);
	}
	//������� ������� � ������ �������� �������������� �� ����
	return pduData;
}

//�������� sequenceNumber � PduRegistry � ��������� pdu
//��������� ������� �������������
void SmppTransmitterTestCases::processCancelSmSync(PduData* pduData,
	PduCancelSmResp* respPdu, time_t respTime)
{
	__require__(pduData);
	__dumpPdu__("processCancelSmRespSync", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(respPdu));
	__decl_tc__;
	__tc__("cancelSm.resp.sync");
	if (!respPdu)
	{
		__tc_fail__(1);
	}
	else
	{
		//�������, �� �� �������������� ������� �������
		ResponseMonitor monitor(pduData->pdu->get_sequenceNumber(),
			pduData->sendTime, pduData, PDU_REQUIRED_FLAG);
		fixture->pduChecker->processCancelSmResp(&monitor, *respPdu, respTime);
		delete respPdu; //disposePdu
	}
	__tc_ok_cond__;
	pduData->unref();
}

//�������� sequenceNumber � PduRegistry, ��������� ������� �������������
void SmppTransmitterTestCases::processCancelSmAsync(PduData* pduData)
{
	__require__(fixture->pduReg);
	__require__(pduData);
	//������� � ���������������� ������� �������
	ResponseMonitor* monitor =
		new ResponseMonitor(pduData->pdu->get_sequenceNumber(),
			pduData->sendTime, pduData, PDU_REQUIRED_FLAG);
	fixture->pduReg->registerMonitor(monitor);
	pduData->unref();
}

//��������� � ���������������� pdu
void SmppTransmitterTestCases::sendCancelSmPdu(PduCancelSm* pdu,
	PduData* cancelPduData, bool sync, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps)
{
	if (fixture->smeType == SME_RECEIVER)
	{
		__decl_tc__;
		try
		{
			__tc__("cancelSm.receiver");
			if (sync)
			{
				fixture->session->getSyncTransmitter()->cancel(*pdu);
			}
			else
			{
				fixture->session->getAsyncTransmitter()->cancel(*pdu);
			}
			__tc_fail__(1);
		}
		catch (SmppInvalidBindState&)
		{
			__tc_ok__;
			return;
		}
		throw Exception("Missing invalid bind exception");
	}
	__decl_tc12__;
	try
	{
		switch (fixture->smeType)
		{
			case SME_TRANSMITTER:
				__tc1__("cancelSm.transmitter");
				break;
			case SME_TRANSCEIVER:
				__tc1__("cancelSm.transceiver");
				break;
			default:
				__unreachable__("Invalid sme type");
		}
		if (fixture->pduReg)
		{
			if (sync)
			{
				__tc2__("cancelSm.sync");
				PduData* pduData;
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					pdu->get_header().set_commandId(CANCEL_SM); //��� �� ���������
					pdu->get_header().set_sequenceNumber(0); //�� ��������
					pduData = prepareCancelSm(pdu, cancelPduData, time(NULL),
						intProps, strProps, objProps);
				}
				//__dumpPdu__("cancelSmSyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				PduCancelSmResp* respPdu =
					fixture->session->getSyncTransmitter()->cancel(*pdu);
				__dumpPdu__("cancelSmSyncAfter", fixture->smeInfo.systemId,
					reinterpret_cast<SmppHeader*>(pdu));
				{
					MutexGuard mguard(fixture->pduReg->getMutex());
					processCancelSmSync(pduData, respPdu, time(NULL));
				}
			}
			else
			{
				__tc2__("cancelSm.async");
				MutexGuard mguard(fixture->pduReg->getMutex());
				//__dumpPdu__("cancelSmAsyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				time_t submitTime = time(NULL);
				PduCancelSmResp* respPdu =
					fixture->session->getAsyncTransmitter()->cancel(*pdu);
				__dumpPdu__("cancelSmAsyncAfter", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				PduData* pduData = prepareCancelSm(pdu, cancelPduData,
					submitTime, intProps, strProps, objProps);
				processCancelSmAsync(pduData);
			}
			//pdu life time ������������ PduRegistry
			//disposePdu(pdu);
		}
		else
		{
			if (sync)
			{
				__tc2__("cancelSm.sync");
				PduCancelSmResp* respPdu =
					fixture->session->getSyncTransmitter()->cancel(*pdu);
				if (respPdu)
				{
					delete respPdu; //disposePdu
				}
			}
			else
			{
				__tc2__("cancelSm.async");
				fixture->session->getAsyncTransmitter()->cancel(*pdu);
			}
			delete pdu; //disposePdu
		}
		__tc12_ok__;
	}
	catch (...)
	{
		__tc12_fail__(100);
		//error();
		throw;
	}
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
			__tc__("deliverySm.resp.sync");
			//__dumpPdu__("sendDeliverySmRespSyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
			fixture->session->getSyncTransmitter()->sendDeliverySmResp(pdu);
			__dumpPdu__("sendDeliverySmRespSyncAfter", fixture->smeInfo.systemId,
				reinterpret_cast<SmppHeader*>(&pdu));
		}
		else
		{
			__tc__("deliverySm.resp.async");
			//__dumpPdu__("sendDeliverySmRespAsyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
			fixture->session->getAsyncTransmitter()->sendDeliverySmResp(pdu);
			__dumpPdu__("sendDeliverySmRespAsyncAfter", fixture->smeInfo.systemId,
				reinterpret_cast<SmppHeader*>(&pdu));
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

//��������� ������� �������������
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

//�������� sequenceNumber � PduRegistry, ��������� ������� �������������
void SmppTransmitterTestCases::processGenericNackAsync(PduData* pduData)
{
	__require__(fixture->pduReg);
	__require__(pduData);
	//������� � ���������������� ������� �������
	GenericNackMonitor* monitor =
		new GenericNackMonitor(pduData->pdu->get_sequenceNumber(),
			pduData->sendTime, pduData, PDU_REQUIRED_FLAG);
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
				//__dumpPdu__("sendInvalidPduSyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
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
				__dumpPdu__("sendInvalidPduSyncAfter", fixture->smeInfo.systemId,
					reinterpret_cast<SmppHeader*>(pdu));
			}
			else
			{
				__tc__("sendInvalidPdu.async");
				MutexGuard mguard(fixture->pduReg->getMutex());
				//__dumpPdu__("sendInvalidPduAsyncBefore", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(pdu));
				time_t submitTime = time(NULL);
				SmppHeader* respPdu =
					fixture->session->getAsyncTransmitter()->sendPdu(pdu);
				__dumpPdu__("sendInvalidPduAsyncAfter", fixture->smeInfo.systemId,
					reinterpret_cast<SmppHeader*>(pdu));
				PduData* pduData = new PduData(pdu, submitTime, 0);
				pduData->ref();
				processGenericNackAsync(pduData);
			}
			//pdu life time ������������ PduRegistry
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

