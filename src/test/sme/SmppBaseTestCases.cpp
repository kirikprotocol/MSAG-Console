#include "SmppBaseTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/Util.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::test::conf::TestConfig;
using namespace smsc::test::core; //flags
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet;

Category& SmppBaseTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppBaseTestCases");
	return log;
}

bool SmppBaseTestCases::bindCorrectSme(int num)
{
	TCSelector s(num, 3);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //BIND_RECIEVER
					__tc__("bindCorrectSme.bindReceiver");
					__tc_fail__(1);
					break;
				case 2: //BIND_TRANSMITTER
					__tc__("bindCorrectSme.bindTransmitter");
					__tc_fail__(2);
					break;
				case 3: //BIND_TRANCIEVER
					__tc__("bindCorrectSme.bindTransceiver");
					break;
				default:
					__unreachable__("Invalid num");
			}
			fixture->session->connect();
			__tc_ok_cond__;
			return true;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
			return false;
		}
	}
}

void SmppBaseTestCases::bindIncorrectSme(int num)
{
	//повторный bind убран из тест кейсов, т.к. приводит к пересозданию коннекта
	//без ошибок
	TCSelector s(num, 3);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //sme не зарегистрирована в SC
					{
						__tc__("bindIncorrectSme.smeNotRegistered");
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15); //15 по спецификации
							conf.sid = tmp.get();
							FakeReceiver receiver;
							SmppSession sess(conf, &receiver);
							sess.connect();
						}
						catch(...)
						{
							throw;
						}
					}
					break;
				case 2: //bind на недоступный SC (неизвестный хост)
					{
						__tc__("bindIncorrectSme.unknownHost");
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15);
							conf.host = tmp.get();
							FakeReceiver receiver;
							SmppSession sess(conf, &receiver);
							sess.connect();
						}
						catch(...)
						{
							throw;
						}
					}
					break;
				case 3: //bind на недоступный SC (неправильный порт)
					{
						__tc__("bindIncorrectSme.invalidPort");
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15);
							conf.port += rand1(65535 - conf.port);
							FakeReceiver receiver;
							SmppSession sess(conf, &receiver);
							sess.connect();
						}
						catch(...)
						{
							throw;
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			__tc_fail__(s.value());
		}
		catch(...)
		{
			__tc_ok__;
		}
	}
}

#define __checkPdu__(tc) \
	__trace2__("%s(): checking pdu with msgRef = %u, submitTime = %ld, waitTime = %ld, validTime = %ld", \
	tc, (uint32_t) pduData->msgRef, pduData->submitTime, pduData->waitTime, pduData->validTime)

#define __missingPdu__(tc, pduName) \
	__trace2__("%s(): missing %s for msgRef = %u, submitTime = %ld, waitTime = %ld, validTime = %ld", \
	tc, pduName, (uint32_t) pduData->msgRef, pduData->submitTime, pduData->waitTime, pduData->validTime)
	/*
	static const char* fmt = "%Y-%m-%d %H:%M:%S"; \
	tm t;
	char __submitTime[20]; char __waitTime[20]; char __validTime[20]; \
	strftime(__submitTime, 20, fmt, localtime_r(&pduData->submitTime, &t)); \
	strftime(__waitTime, 20, fmt, localtime_r(&pduData->waitTime, &t)); \
	strftime(__validTime, 20, fmt, localtime_r(&pduData->validTime, &t)); \
	__trace2__("%s(): missing %s for sequenceNumber = %u, submitTime = %s, waitTime = %s, validTime = %s", \
	tc, pduName, pduData->pdu->get_sequenceNumber(), __submitTime, __waitTime, __validTime)
	*/

#define __removedPdu__(tc) \
	__trace2__("%s(): removed pdu data with msgRef = %u", \
	tc, (uint32_t) pduData->msgRef)
	
#define __checkSummary__(tc) \
	__trace2__("%s(): found = %d, deleted = %d", tc, found, deleted);

void SmppBaseTestCases::checkSubmitTime(time_t checkTime)
{
	__decl_tc__;
	//проверить неполученные респонсы 
	PduRegistry::PduDataIterator* it = fixture->pduReg->getPduBySubmitTime(0, checkTime);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		__checkPdu__("checkSubmitTime");
		found++;
		__tc__("checkMissingPdu.response");
		if (pduData->responseFlag == PDU_REQUIRED_FLAG)
		{
			__missingPdu__("checkSubmitTime", "response");
			__tc_fail__(1);
			pduData->responseFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		/*
		if (pduData->complete())
		{
			 __removedPdu__("checkSubmitTime");
			deleted++;
			fixture->pduReg->removePdu(pduData);
		}
		*/
	}
	delete it;
	__checkSummary__("checkSubmitTime");
}

void SmppBaseTestCases::checkWaitTime(time_t checkTime)
{
	__decl_tc__;
	//проверить неполученные доставки и подтверждения доставки 
	//на момент waitTime
	PduRegistry::PduDataIterator* it = fixture->pduReg->getPduByWaitTime(0, checkTime);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		__checkPdu__("checkWaitTime");
		found++;
		__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
		if (!fixture->routeChecker->isDestReachable(
			pdu->get_message().get_source(), pdu->get_message().get_dest(), true))
		{
			continue;
		}
		__tc__("checkMissingPdu.delivery.waitTime");
		if (pduData->deliveryFlag.isPduMissing(checkTime))
		{
			__missingPdu__("checkWaitTime", "deliver_sm");
			__tc_fail__(1);
			pduData->deliveryFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		__tc__("checkMissingPdu.deliveryReceipt.waitTime");
		if (pduData->deliveryReceiptFlag.isPduMissing(checkTime))
		{
			__missingPdu__("checkWaitTime", "delivery_receipt");
			__tc_fail__(1);
			pduData->deliveryReceiptFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		__tc__("checkMissingPdu.intermediateNotification.waitTime");
		if (pduData->intermediateNotificationFlag == PDU_REQUIRED_FLAG)
		{
			__missingPdu__("checkWaitTime", "intermediate_notification");
			__tc_fail__(1);
			pduData->intermediateNotificationFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		/*
		if (pduData->complete())
		{
			__removedPdu__("checkWaitTime");
			deleted++;
			fixture->pduReg->removePdu(pduData);
		}
		*/
	}
	delete it;
	__checkSummary__("checkWaitTime");
}

void SmppBaseTestCases::checkValidTime(time_t checkTime)
{
	__decl_tc__;
	//проверить неполученные доставки и подтверждения доставки
	//по окончании validTime
	PduRegistry::PduDataIterator* it = fixture->pduReg->getPduByValidTime(0, checkTime);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		__checkPdu__("checkValidTime");
		found++;
		__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
		if (!fixture->routeChecker->isDestReachable(
			pdu->get_message().get_source(), pdu->get_message().get_dest(), true))
		{
			pduData->deliveryFlag = PDU_NOT_EXPECTED_FLAG;
		}
		__tc__("checkMissingPdu.delivery.validTime");
		if (pduData->deliveryFlag.isPduMissing(checkTime))
		{
			__missingPdu__("checkValidTime", "deliver_sm");
			__tc_fail__(1);
			pduData->deliveryFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		__tc__("checkMissingPdu.deliveryReceipt.validTime");
		if (pduData->deliveryReceiptFlag.isPduMissing(checkTime))
		{
			__missingPdu__("checkValidTime", "delivery_receipt");
			__tc_fail__(1);
			pduData->deliveryReceiptFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		__tc__("checkMissingPdu.intermediateNotification.validTime");
		if (pduData->intermediateNotificationFlag == PDU_REQUIRED_FLAG)
		{
			__missingPdu__("checkValidTime", "intermediate_notification");
			__tc_fail__(1);
			pduData->intermediateNotificationFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		//для pdu с validTime меньше текущего мог быть неполучен респонс
		if (pduData->submitTime <= checkTime)
		{
			__removedPdu__("checkValidTime");
			deleted++;
			fixture->pduReg->removePdu(pduData);
		}
	}
	delete it;
	__checkSummary__("checkValidTime");
}

void SmppBaseTestCases::checkMissingPdu()
{
	if (fixture->pduReg)
	{
		MutexGuard mguard(fixture->pduReg->getMutex());
		__cfg_int__(timeCheckAccuracy);
		time_t checkTime = time(NULL) - timeCheckAccuracy;
		checkSubmitTime(checkTime);
		checkWaitTime(checkTime);
		checkValidTime(checkTime);
		__trace2__("checkMissingPdu(): pduReg size = %d", fixture->pduReg->size());
	}
}

void SmppBaseTestCases::unbind()
{
	__decl_tc__;
	__tc__("unbind");
	try
	{
		fixture->session->close();
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

