#include "DeliveryReportHandler.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "profiler/profiler.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::sms::Address;
using smsc::core::synchronization::MutexGuard;
using smsc::test::sms::operator!=;
using smsc::test::conf::TestConfig;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::profiler;
using namespace smsc::test::core;
using namespace smsc::test::smpp; //SmppUtil, constants
using namespace smsc::test::util;

DeliveryReportHandler::DeliveryReportHandler(SmppFixture* _fixture,
	const Address& _smeAddr, const Address& _smeAlias,
	const SmeSystemId& _smeServiceType, uint8_t _smeProtocolId)
: fixture(_fixture), chkList(_fixture->chkList), smeAddr(_smeAddr),
	smeAlias(_smeAlias), smeServiceType(_smeServiceType),
	smeProtocolId(_smeProtocolId) {}

vector<int> DeliveryReportHandler::checkRoute(PduSubmitSm& pdu1,
	PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//������������ destAddr ��� pdu2
	if (destAddr2 != origAddr1)
	{
		res.push_back(1);
	}
	//������������ ��������
	const RouteHolder* routeHolder = NULL;
	if (fixture->smeInfo.wantAlias)
	{
		const Address origAddr2 = fixture->aliasReg->findAddressByAlias(origAlias2);
		routeHolder = fixture->routeReg->lookup(origAddr2, destAddr2);
	}
	else
	{
		routeHolder = fixture->routeReg->lookup(origAlias2, destAddr2);
	}
	if (!routeHolder)
	{
		res.push_back(3);
	}
	else if (fixture->smeInfo.systemId != routeHolder->route.smeSystemId)
	{
		res.push_back(4);
	}
	return res;
}

#define __compare__(errCode, field, value) \
	if (value != field) { __tc_fail__(errCode); }

void DeliveryReportHandler::processPdu(PduDeliverySm& pdu, time_t recvTime)
{
	__trace__("processDeliveryReport()");
	__decl_tc__;
	static const char DELIVERY_RECEIPT = '*';
	static const char INTERMEDIATE_NOTIFICATION = '$';
	try
	{
		__tc__("processDeliverySm.deliveryReport");
		//������������ ��� delivery receipt � intermediate notification
		//������������ ����
		if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(1);
			throw TCException();
		}
		//��������� pduReg ������
		Address destAddr;
		SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(destAddr);
		__require__(pduReg);
		//��� pdu (delivery receipt '*' ��� intermediate notification '$')
		char pduType;
		switch (pdu.get_message().get_dataCoding())
		{
			case DATA_CODING_SMSC_DEFAULT:
				__require__(pdu.get_message().size_shortMessage() > 0);
				pduType = *pdu.get_message().get_shortMessage();
				break;
			case DATA_CODING_UCS2:
				__require__(pdu.get_message().size_shortMessage() > 1);
				pduType = *(pdu.get_message().get_shortMessage() + 1);
				break;
			default:
				__unreachable__("Invalid data coding");

		}
		//�������� ������������ pdu
		MutexGuard mguard(pduReg->getMutex());
		DeliveryReportMonitor* monitor = NULL;
		switch (pduType)
		{
			case DELIVERY_RECEIPT:
				monitor = pduReg->getDeliveryReceiptMonitor(
					pdu.get_optional().get_userMessageReference());
				break;
			case INTERMEDIATE_NOTIFICATION:
				monitor = pduReg->getIntermediateNotificationMonitor(
					pdu.get_optional().get_userMessageReference());
				break;
			default:
				monitor = NULL;
		}
		//��� user_message_reference �� ���������� pdu
		//��� ���������������� ������������� pdu
		if (!monitor)
		{
			__tc_fail__(2);
			__trace2__("getDeliveryReportMonitor(): pduReg = %p, userMessageReference = %d, pduType = %c, monitor = NULL",
				pduReg, pdu.get_optional().get_userMessageReference(), pduType);
			throw TCException();
		}
		//� ������ ������� delivery report �������� ������ submit_sm_resp
		if (!monitor->pduData->valid)
		{
			__tc_fail__(3);
			//throw TCException();
		}
		__tc_ok_cond__;
		if (pduType == DELIVERY_RECEIPT)
		{
			__tc__("processDeliverySm.deliveryReport.deliveryReceipt.checkAllowed");
		}
		else
		{
			__require__(pduType == INTERMEDIATE_NOTIFICATION);
			__tc__("processDeliverySm.deliveryReport.intermediateNotification.checkAllowed");
		}
		switch (monitor->getFlag())
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
				//ok
				break;
			case PDU_RECEIVED_FLAG:
				//������ ������� ������ ��� ����
				__tc_fail__(1);
				throw TCException();
			case PDU_NOT_EXPECTED_FLAG:
			case PDU_EXPIRED_FLAG:
			case PDU_ERROR_FLAG:
				__tc_fail__(2);
				throw TCException();
			default:
				__unreachable__("Unknown flag");
		}
		__tc_ok_cond__;
		__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* origPdu =
			reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
		__require__(origPdu);
		//�������� ������������ ��������
		__tc__("processDeliverySm.deliveryReport.checkRoute");
		__tc_fail2__(checkRoute(*origPdu, pdu), 0);
		__tc_ok_cond__;
		//��������� ���������� ���������� pdu
		__tc__("processDeliverySm.deliveryReport.checkFields");
		//���� header ����������� � processDeliverySm()
		//���� message ����������� � processDeliveryReport()
		//������������ ������� ����������� � checkRoute()
		__compare__(1, nvl(pdu.get_message().get_serviceType()), smeServiceType);
		Address srcAlias;
		SmppUtil::convert(pdu.get_message().get_source(), &srcAlias);
		if (fixture->smeInfo.wantAlias && srcAlias != smeAlias)
		{
			__tc_fail__(2);
		}
		else if (!fixture->smeInfo.wantAlias && srcAlias != smeAddr)
		{
			__tc_fail__(3);
		}
		//__compare__(4, pdu.get_message().get_dest(), origPdu->get_message().get_source());
		bool statusReport;
		switch (origPdu->get_message().get_registredDelivery() & SMSC_DELIVERY_RECEIPT_BITS)
		{
			case NO_SMSC_DELIVERY_RECEIPT:
			case SMSC_DELIVERY_RECEIPT_RESERVED:
				statusReport = false;
				break;
			case FAILURE_SMSC_DELIVERY_RECEIPT:
				statusReport = monitor->deliveryStatus != ESME_ROK;
				break;
			case FINAL_SMSC_DELIVERY_RECEIPT:
				statusReport = true;
				break;
			default:
				__unreachable__("Invalid reg dilivery");
		}
		if (statusReport)
		{
			if (pduType == DELIVERY_RECEIPT)
			{
				__compare__(4, pdu.get_message().get_esmClass(),
					ESM_CLASS_DELIVERY_RECEIPT);
			}
			else
			{
				__require__(pduType == INTERMEDIATE_NOTIFICATION);
				__compare__(5, pdu.get_message().get_esmClass(),
					ESM_CLASS_INTERMEDIATE_NOTIFICATION);
			}
		}
		else if (monitor->pduData->reportOptions == ProfileReportOptions::ReportFull)
		{
			__compare__(6, pdu.get_message().get_esmClass(),
				ESM_CLASS_NORMAL_MESSAGE);
		}
		else
		{
			__tc_fail__(7);
		}
		__compare__(8, pdu.get_message().get_protocolId(), smeProtocolId);
		__compare__(9, pdu.get_message().get_priorityFlag(), 0);
		__compare__(10, pdu.get_message().get_registredDelivery(), 0);
		__tc_ok_cond__;
		//�������� ����� pdu
		pduReg->removeMonitor(monitor);
		if (pduType == DELIVERY_RECEIPT)
		{
			processDeliveryReceipt(dynamic_cast<DeliveryReceiptMonitor*>(monitor),
				pdu, recvTime);
		}
		else
		{
			__require__(pduType == INTERMEDIATE_NOTIFICATION);
			processIntermediateNotification(
				dynamic_cast<IntermediateNotificationMonitor*>(monitor),
				pdu, recvTime);
		}
		pduReg->registerMonitor(monitor);
		//��� delivery report �� �������� ��������� ��������
		if (pduType == DELIVERY_RECEIPT)
		{
			__tc__("processDeliverySm.deliveryReport.deliveryReceipt.recvTimeChecks");
		}
		else
		{
			__require__(pduType == INTERMEDIATE_NOTIFICATION);
			__tc__("processDeliverySm.deliveryReport.intermediateNotification.recvTimeChecks");
		}
		__cfg_int__(timeCheckAccuracy);
		if (recvTime < monitor->getCheckTime())
		{
			__trace2__("expected time = %ld", monitor->getStartTime());
			__tc_fail__(1);
		}
		else if (recvTime > monitor->getCheckTime() + timeCheckAccuracy)
		{
			__trace2__("expected time = %ld", monitor->getStartTime());
			__tc_fail__(2);
		}
		__tc_ok_cond__;
		//��������� �������, ������ ESME_ROK ���������
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendDeliverySmResp(pdu);
		__require__(deliveryResp.first == ESME_ROK);
		RespPduFlag respFlag = isAccepted(deliveryResp.first);
	}
	catch (TCException&)
	{
		//��������� �������, ������ ESME_ROK ���������
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendDeliverySmResp(pdu);
		__require__(deliveryResp.first == ESME_ROK);
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

