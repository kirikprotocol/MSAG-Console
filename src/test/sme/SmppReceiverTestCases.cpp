#include "SmppReceiverTestCases.hpp"
#include "SmppPduChecker.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::sme::SmppTransmitter;
using smsc::test::conf::TestConfig;
using namespace smsc::profiler;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::util;
using namespace smsc::test::core; //constants
using namespace smsc::test::smpp; //constants, SmppUtil

Category& SmppReceiverTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppReceiverTestCases");
	return log;
}

void SmppReceiverTestCases::processSubmitSmResp(PduSubmitSmResp& pdu)
{
	__dumpPdu__("processSubmitSmRespBefore", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(&pdu));
	time_t respTime = time(NULL);
	if (!fixture->pduReg)
	{
		return;
	}
	__decl_tc__;
	__tc__("submitSm.resp.async");
	try
	{
		//�������� ������������ pdu
		MutexGuard mguard(fixture->pduReg->getMutex());
		ResponseMonitor* monitor = fixture->pduReg->getResponseMonitor(
			pdu.get_header().get_sequenceNumber());
		//��� sequence number �� �������� ��� ���������������� pdu
		if (!monitor)
		{
			__tc_fail__(1);
		}
		else
		{
			//��������� � �������� response � delivery receipt ��������
			//�� ������ �� ��������
			fixture->pduReg->removeMonitor(monitor);
			fixture->pduChecker->processSubmitSmResp(monitor, pdu, respTime);
			//fixture->pduReg->registerMonitor(monitor);
			//__dumpPdu__("processSubmitSmRespAfter", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(&pdu));
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processDataSmResp(PduDataSmResp& pdu)
{
	__dumpPdu__("processDataSmRespBefore", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(&pdu));
	time_t respTime = time(NULL);
	if (!fixture->pduReg)
	{
		return;
	}
	__decl_tc__;
	__tc__("dataSm.resp.async");
	try
	{
		//�������� ������������ pdu
		MutexGuard mguard(fixture->pduReg->getMutex());
		ResponseMonitor* monitor = fixture->pduReg->getResponseMonitor(
			pdu.get_header().get_sequenceNumber());
		//��� sequence number �� �������� ��� ���������������� pdu
		if (!monitor)
		{
			__tc_fail__(1);
		}
		else
		{
			//��������� � �������� response � delivery receipt ��������
			//�� ������ �� ��������
			fixture->pduReg->removeMonitor(monitor);
			fixture->pduChecker->processDataSmResp(monitor, pdu, respTime);
			//fixture->pduReg->registerMonitor(monitor);
			//__dumpPdu__("processDataSmRespAfter", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(&pdu));
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processReplaceSmResp(PduReplaceSmResp& pdu)
{
	__dumpPdu__("processReplaceSmRespBefore", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(&pdu));
	time_t respTime = time(NULL);
	if (!fixture->pduReg)
	{
		return;
	}
	__decl_tc__;
	__tc__("replaceSm.resp.async");
	try
	{
		//�������� ������������ pdu
		MutexGuard mguard(fixture->pduReg->getMutex());
		ResponseMonitor* monitor = fixture->pduReg->getResponseMonitor(
			pdu.get_header().get_sequenceNumber());
		//��� sequence number �� �������� ��� ���������������� pdu
		if (!monitor)
		{
			__tc_fail__(1);
		}
		else
		{
			//��������� � �������� response � delivery receipt ��������
			//�� ������ �� ��������
			fixture->pduReg->removeMonitor(monitor);
			fixture->pduChecker->processReplaceSmResp(monitor, pdu, respTime);
			//__dumpPdu__("processReplaceSmRespAfter", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(&pdu));
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

#define __checkForNull__(failureCode, field) \
	if (field) { __tc_fail__(failureCode); }

void SmppReceiverTestCases::processDeliverySm(PduDeliverySm& pdu)
{
	SmppHeader* header = reinterpret_cast<SmppHeader*>(&pdu);
	__dumpPdu__("processDeliverySmBefore", fixture->smeInfo.systemId, header);
	__require__(fixture->session);
	__decl_tc__;
	__tc__("sms.deliverySm.checkFields");
	time_t recvTime = time(NULL);
	//����� �������� �����
	//header
	//pdu.get_header().get_commandLength()
	if (pdu.get_header().get_commandId() != DELIVERY_SM)
	{
		__tc_fail__(1);
	}
	__checkForNull__(2, pdu.get_header().get_commandStatus());
	//pdu.get_header().get_sequenceNumber()
	//message
	__checkForNull__(3, pdu.get_message().get_scheduleDeliveryTime());
	__checkForNull__(4, pdu.get_message().get_validityPeriod());
	__checkForNull__(5, pdu.get_message().get_replaceIfPresentFlag());
	__checkForNull__(6, pdu.get_message().get_smDefaultMsgId());
	__tc_ok_cond__;
	//���������� deliver_sm pdu
	if (fixture->pduReg)
	{
		Address origAddr;
		SmppUtil::convert(pdu.get_message().get_source(), &origAddr);
		if (fixture->smeInfo.wantAlias)
		{
			origAddr = fixture->aliasReg->findAddressByAlias(origAddr);
		}
		//��������� ��� sme
		PduHandlerMap::iterator it = fixture->pduHandler.find(origAddr);
		if (it == fixture->pduHandler.end())
		{
			defaultHandler.processPdu(header, origAddr, recvTime);
		}
		else
		{
			PduHandler* handler = it->second;
			__require__(handler);
			handler->processPdu(header, recvTime);
		}
	}
	else //��������� �������
	{
		fixture->respSender->sendSmsResp(header);
	}
	//__dumpPdu__("processDeliverySmAfter", fixture->smeInfo.systemId, header);
}

void SmppReceiverTestCases::processDataSm(PduDataSm& pdu)
{
	SmppHeader* header = reinterpret_cast<SmppHeader*>(&pdu);
	__dumpPdu__("processDataSmBefore", fixture->smeInfo.systemId, header);
	__require__(fixture->session);
	__decl_tc__;
	__tc__("sms.dataSm.checkFields");
	time_t recvTime = time(NULL);
	//����� �������� �����
	//header
	//pdu.get_header().get_commandLength()
	if (pdu.get_header().get_commandId() != DATA_SM)
	{
		__tc_fail__(1);
	}
	__checkForNull__(2, pdu.get_header().get_commandStatus());
	//pdu.get_header().get_sequenceNumber()
	//message
	//__checkForNull__(3, pdu.get_optional().has_qosTimeToLive());
	__tc_ok_cond__;
	//���������� data_sm pdu
	if (fixture->pduReg)
	{
		Address origAlias;
		SmppUtil::convert(pdu.get_data().get_source(), &origAlias);
		const Address origAddr = fixture->aliasReg->findAddressByAlias(origAlias);
		//��������� ��� sme
		PduHandlerMap::iterator it = fixture->pduHandler.find(origAddr);
		if (it == fixture->pduHandler.end())
		{
			defaultHandler.processPdu(header, origAddr, recvTime);
		}
		else
		{
			PduHandler* handler = it->second;
			__require__(handler);
			handler->processPdu(header, recvTime);
		}
	}
	else //��������� �������
	{
		fixture->respSender->sendSmsResp(header);
	}
	//__dumpPdu__("processDataSmAfter", fixture->smeInfo.systemId, header);
}

void SmppReceiverTestCases::processQuerySmResp(PduQuerySmResp& pdu)
{
	__dumpPdu__("processQuerySmRespBefore", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(&pdu));
	time_t respTime = time(NULL);
	if (!fixture->pduReg)
	{
		return;
	}
	__decl_tc__;
	__tc__("querySm.resp.async");
	try
	{
		//�������� ������������ pdu
		MutexGuard mguard(fixture->pduReg->getMutex());
		ResponseMonitor* monitor = fixture->pduReg->getResponseMonitor(
			pdu.get_header().get_sequenceNumber());
		//��� sequence number �� �������� ��� ���������������� pdu
		if (!monitor)
		{
			__tc_fail__(1);
		}
		else
		{
			fixture->pduReg->removeMonitor(monitor);
			fixture->pduChecker->processQuerySmResp(monitor, pdu, respTime);
			//__dumpPdu__("processQuerySmRespAfter", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(&pdu));
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processCancelSmResp(PduCancelSmResp& pdu)
{
	__dumpPdu__("processCancelSmRespBefore", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(&pdu));
	time_t respTime = time(NULL);
	if (!fixture->pduReg)
	{
		return;
	}
	__decl_tc__;
	__tc__("cancelSm.resp.async");
	try
	{
		//�������� ������������ pdu
		MutexGuard mguard(fixture->pduReg->getMutex());
		ResponseMonitor* monitor = fixture->pduReg->getResponseMonitor(
			pdu.get_header().get_sequenceNumber());
		//��� sequence number �� �������� ��� ���������������� pdu
		if (!monitor)
		{
			__tc_fail__(1);
		}
		else
		{
			//��������� response �������
			fixture->pduReg->removeMonitor(monitor);
			fixture->pduChecker->processCancelSmResp(monitor, pdu, respTime);
			//__dumpPdu__("processCancelSmRespAfter", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(&pdu));
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processGenericNack(PduGenericNack& pdu)
{
	__dumpPdu__("processGenericNackBefore", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(&pdu));
	__unreachable__("Fatal error");
	/*
	__dumpPdu__("processGenericNackBefore", fixture->smeInfo.systemId,
		reinterpret_cast<SmppHeader*>(&pdu));
	time_t respTime = time(NULL);
	if (!fixture->pduReg)
	{
		return;
	}
	__decl_tc__;
	__tc__("processGenericNack.async");
	try
	{
		//�������� ������������ pdu
		MutexGuard mguard(fixture->pduReg->getMutex());
		GenericNackMonitor* monitor = fixture->pduReg->getGenericNackMonitor(
			pdu.get_header().get_sequenceNumber());
		//��� sequence number �� �������� ��� ���������������� pdu
		if (!monitor)
		{
			__tc_fail__(1);
		}
		else
		{
			fixture->pduChecker->processGenericNack(monitor, pdu, respTime);
			//__dumpPdu__("processGenericNackAfter", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(&pdu));
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
	*/
}

void SmppReceiverTestCases::processMultiResp(PduMultiSmResp& pdu)
{
	//__dumpPdu__("processMultiResp", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(&pdu));
	__decl_tc__;
	__tc__("notImplemented.processMultiResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processAlertNotification(PduAlertNotification& pdu)
{
	//__dumpPdu__("processAlertNotification", fixture->smeInfo.systemId, reinterpret_cast<SmppHeader*>(&pdu));
	__decl_tc__;
	__tc__("notImplemented.processAlertNotification");
	__tc_fail__(100);
}

void SmppReceiverTestCases::handleError(int errorCode)
{
	__trace2__("handleError(): errorCode = %d", errorCode);
	__unreachable__("Fatal error");
}

}
}
}

