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
	const Address& destAlias, uint64_t mask)
{
	__require__(pdu);
	SmppUtil::setupRandomCorrectSubmitSmPdu(pdu, mask);
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

void SmppTransmitterTestCases::updateReplacePduMonitors(PduData* pduData,
	time_t submitTime)
{
	__require__(pduData && pduData->pdu);
	__require__(pduData->pdu->get_commandId() == SUBMIT_SM);
	__cfg_int__(timeCheckAccuracy);
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	__require__(pdu->get_optional().has_userMessageReference());
	uint16_t msgRef = pdu->get_optional().get_userMessageReference();
	//delivery monitor
	DeliveryMonitor* deliveryMonitor = fixture->pduReg->getDeliveryMonitor(msgRef);
	__require__(deliveryMonitor);
	switch (deliveryMonitor->getFlag())
	{
		case PDU_REQUIRED_FLAG: //ENROUTE pdu
			__require__(deliveryMonitor->getCheckTime() > submitTime + timeCheckAccuracy);
			fixture->pduReg->removeMonitor(deliveryMonitor);
			deliveryMonitor->setNotExpected();
			//�������� ��� ���� ������ �� ���������� pdu
			fixture->pduReg->registerMonitor(deliveryMonitor);
			break;
		case PDU_NOT_EXPECTED_FLAG: //final pdu
			__require__(deliveryMonitor->state == DELIVERED);
			break;
		default:
			__unreachable__("Invalid delivery monitor flag");
	}
	//delivery receipt monitor
	DeliveryReceiptMonitor* rcptMonitor =
		fixture->pduReg->getDeliveryReceiptMonitor(msgRef);
	if (rcptMonitor)
	{
		//���� ���� delivery receipt monitor, �� pdu ��� ����������������
		__require__(deliveryMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
		switch (rcptMonitor->getFlag())
		{
			case PDU_REQUIRED_FLAG: //�������� pdu, ��� ������� ������ ��������� �������� � ������ ������ �������� �������������
				fixture->pduReg->removeMonitor(rcptMonitor);
				if (rcptMonitor->getCheckTime() >= submitTime &&
					rcptMonitor->getCheckTime() <= submitTime + timeCheckAccuracy)
				{
					rcptMonitor->setCondRequired();
					fixture->pduReg->registerMonitor(rcptMonitor);
				}
				else
				{
					__require__(rcptMonitor->getCheckTime() > submitTime + timeCheckAccuracy);
					rcptMonitor->setNotExpected();
					//������� ���� ������ ��� �� �����
					//fixture->pduReg->registerMonitor(rcptMonitor);
				}
				break;
			case PDU_NOT_EXPECTED_FLAG:
				break;
			default:
				__unreachable__("Invalid delivery receipt monitor flag");
		}
	}
	//intermediate notification monitor
	IntermediateNotificationMonitor* notifMonitor =
		fixture->pduReg->getIntermediateNotificationMonitor(msgRef);
	if (notifMonitor)
	{
		switch (notifMonitor->getFlag())
		{
			case PDU_REQUIRED_FLAG: //��������, ������� �� ��� ������� � ������� ��� �� �����
				fixture->pduReg->removeMonitor(notifMonitor);
				if (notifMonitor->getCheckTime() >= submitTime &&
					notifMonitor->getCheckTime() <= submitTime + timeCheckAccuracy)
				{
					notifMonitor->setCondRequired();
					fixture->pduReg->registerMonitor(notifMonitor);
				}
				else
				{
					__require__(notifMonitor->getCheckTime() > submitTime + timeCheckAccuracy);
					notifMonitor->setNotExpected();
					//������� ���� ������ ��� �� �����
					//fixture->pduReg->registerMonitor(notifMonitor);
				}
				break;
			case PDU_NOT_EXPECTED_FLAG:
				break;
			default:
				__unreachable__("Invalid intermediate notification monitor flag");
		}
	}
}

void SmppTransmitterTestCases::registerNormalSmeMonitors(PduSubmitSm* pdu,
	PduData* existentPduData, const Profile& profile, uint16_t msgRef,
	time_t waitTime, time_t validTime, PduData* pduData)
{
	bool deliveryFlag = false;
	bool errorReportsFlag = false;
	bool expiredReportFlag = false;
	SmeType destType = fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest());
	switch (destType)
	{
		case SME_NO_ROUTE:
			//����� ������� � ����� ������
			return;
		case SME_TRANSMITTER:
			errorReportsFlag = true;
			break;
		case SME_NOT_BOUND:
			expiredReportFlag = true;
			break;
		case SME_RECEIVER:
		case SME_TRANSCEIVER:
			deliveryFlag = true;
			//�������� ����� ������� � NormalSmsHandler::registerDeliveryReportMonitors()
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
					updateReplacePduMonitors(existentPduData, pduData->sendTime);
				}
				else
				{
					deliveryMonitor->pduData->intProps["hasSmppDuplicates"] = 1;
					existentPduData->intProps["hasSmppDuplicates"] = 1;
				}
			}
		}
		fixture->pduReg->registerMonitor(deliveryMonitor);
	}
	//�������
	uint8_t regDelivery = getRegisteredDelivery(pduData);
	if (regDelivery == FINAL_SMSC_DELIVERY_RECEIPT ||
		regDelivery == FAILURE_SMSC_DELIVERY_RECEIPT)
	{
		if (errorReportsFlag)
		{
			//delivery receipt
			DeliveryReceiptMonitor* rcptMonitor =
				new DeliveryReceiptMonitor(msgRef, waitTime, pduData, PDU_REQUIRED_FLAG);
			rcptMonitor->state = UNDELIVERABLE;
			rcptMonitor->deliveryStatus = ESME_RX_P_APPN;
			fixture->pduReg->registerMonitor(rcptMonitor);
		}
		if (expiredReportFlag)
		{
			//intermediate notification
			IntermediateNotificationMonitor* notifMonitor =
				new IntermediateNotificationMonitor(msgRef, waitTime, pduData, PDU_REQUIRED_FLAG);
			notifMonitor->state = ENROUTE;
			notifMonitor->deliveryStatus = ESME_RSYSERR;
			fixture->pduReg->registerMonitor(notifMonitor);
			//delivery receipt
			DeliveryReceiptMonitor* rcptMonitor =
				new DeliveryReceiptMonitor(msgRef, validTime, pduData, PDU_REQUIRED_FLAG);
			rcptMonitor->state = EXPIRED;
			rcptMonitor->deliveryStatus = ESME_RSYSERR;
			fixture->pduReg->registerMonitor(rcptMonitor);
		}
	}
}

void SmppTransmitterTestCases::registerExtSmeMonitors(PduSubmitSm* pdu,
	uint16_t msgRef, time_t waitTime, time_t validTime, PduData* pduData)
{
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
		DeliveryReceiptMonitor* rcptMonitor =
			new DeliveryReceiptMonitor(msgRef, waitTime, pduData, PDU_REQUIRED_FLAG);
		rcptMonitor->state = DELIVERED;
		rcptMonitor->deliveryStatus = ESME_ROK;
		fixture->pduReg->registerMonitor(rcptMonitor);
	}
}

void SmppTransmitterTestCases::registerNullSmeMonitors(PduSubmitSm* pdu,
	uint16_t msgRef, time_t waitTime, time_t validTime, uint32_t deliveryStatus,
	PduData* pduData)
{
	//�����������, ��� null sme ������ �������� � �� ���� ���� �������
	//null sme �� ���������� sme ack, � �� deliver_sm ����� ���������� �������
	__require__(fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest()) == SME_TRANSCEIVER);
	//delivery receipt monitor
	uint8_t regDelivery = getRegisteredDelivery(pduData);
	if (regDelivery == FINAL_SMSC_DELIVERY_RECEIPT ||
		(regDelivery == FAILURE_SMSC_DELIVERY_RECEIPT && deliveryStatus != ESME_ROK))
	{
		DeliveryReceiptMonitor* rcptMonitor = 
			new DeliveryReceiptMonitor(msgRef, waitTime, pduData, PDU_REQUIRED_FLAG);
		rcptMonitor->state = deliveryStatus == ESME_ROK ? DELIVERED : UNDELIVERABLE;
		rcptMonitor->deliveryStatus = deliveryStatus;
		fixture->pduReg->registerMonitor(rcptMonitor);
	}
}

//��������������� ����������� pdu, ��������� ������� �������������
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
	time_t t; //������� ����� ����� ������ ���, ��� � profileReg, ��������� profileUpdateTime
	Profile profile = fixture->profileReg->getProfile(srcAddr, t);
	__require__(t <= submitTime); //� ��������� �� �������
	//pdu data
	pdu->get_header().set_commandId(SUBMIT_SM); //������������� � ������ �������� submit_sm
	PduData* pduData = new PduData(reinterpret_cast<SmppHeader*>(pdu),
		submitTime, intProps, strProps, objProps);
	pduData->intProps["registredDelivery"] = pdu->get_message().get_registredDelivery();
	pduData->intProps["reportOptions"] = profile.reportoptions;
	pduData->ref();
	//��������� ������� ������ (����� �� ������� � ����� ������)
	set<int> res = fixture->pduChecker->checkSubmitSm(pduData);
	if (res.size())
	{
		return pduData;
	}
	//response monitor �������������� ����� ������ �������� seqNum
	switch (pduType)
	{
		case PDU_NORMAL:
			//����������� DeliveryMonitor � ��� sme, �� ������� ���� �������,
			//�� ��� ����� sme DeliveryReceiptMonitor � IntermediateNotificationMonitor
			registerNormalSmeMonitors(pdu, existentPduData, profile, msgRef,
				waitTime, validTime, pduData);
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
	__dumpPdu__("processSubmitSmRespSync", fixture->smeInfo.systemId, respPdu);
	__decl_tc__;
	__tc__("processSubmitSmResp.sync");
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
					pdu->get_header().set_sequenceNumber(0); //�� ��������
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
			//pdu life time ������������ PduRegistry
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
	uint8_t dataCoding = DEFAULT;
	if (replacePduData)
	{
		PduSubmitSm* origPdu = reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
		__require__(origPdu);
		dataCoding = origPdu->get_message().get_dataCoding();
	}
	SmppUtil::setupRandomCorrectReplaceSmPdu(pdu, dataCoding);
	//source
	PduAddress srcAddr;
	SmppUtil::convert(fixture->smeAddr, &srcAddr);
	pdu->set_source(srcAddr);
	//������ �������� �������������� messageId
	if (replacePduData)
	{
		__require__(replacePduData->strProps.count("smsId"));
		pdu->set_messageId(replacePduData->strProps["smsId"].c_str());
	}
	else
	{
		auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
		pdu->set_messageId(msgId.get());
	}
}

//��������������� ����������� pdu, ��������� ������� �������������
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
		resPdu->get_message().set_replaceIfPresentFlag(1); //��� ���������� ������ registerSubmitSm()
		PduData* pduData = registerSubmitSm(resPdu, replacePduData, submitTime, NULL, NULL, NULL, PDU_NORMAL);
		pduData->strProps["smsId"] = pdu->get_messageId();
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
		pdu->get_header().set_sequenceNumber(0); //�� ��������
		//������� ������� � ������ �������� �������������� �� ����
		return pduData;
	}
}

//�������� sequenceNumber � PduRegistry � ��������� pdu
//��������� ������� �������������
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
void SmppTransmitterTestCases::processReplaceSmAsync(PduData* pduData)
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
					pdu->get_header().set_sequenceNumber(0); //�� ��������
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
			//pdu life time ������������ PduRegistry
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

