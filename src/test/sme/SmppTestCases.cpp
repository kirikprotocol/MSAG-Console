#include "SmppTestCases.hpp"
#include "test/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using namespace smsc::test; //config constants
using namespace smsc::smpp::SmppCommandSet;

SmppTestCases::SmppTestCases(const SmeConfig& _config, const SmeSystemId& _systemId,
	const Address& addr, const SmeRegistry* _smeReg, const AliasRegistry* _aliasReg,
	const RouteRegistry* _routeReg, ResultHandler* handler)
	: config(_config), session(NULL), systemId(_systemId), smeAlias(addr),
	smeReg(_smeReg), aliasReg(_aliasReg), routeReg(_routeReg), resultHandler(handler)
{
	__require__(smeReg);
	__require__(aliasReg);
	__require__(routeReg);
	__require__(resultHandler);
	pduReg = smeReg->getPduRegistry(smeAlias); //����� ���� NULL
	routeChecker = new RouteChecker(systemId, smeAlias, smeReg, aliasReg, routeReg);
	pduChecker = new SmppPduChecker(pduReg, routeChecker);
	receiver = new SmppReceiverTestCases(systemId, smeAlias, smeReg,
		aliasReg, routeReg, handler, routeChecker, pduChecker);
	session = new SmppSession(config, receiver);
	receiver->setSession(session);
	transmitter = new SmppTransmitterTestCases(session, systemId, smeAlias,
		smeReg, routeChecker, pduChecker);
}

SmppTestCases::~SmppTestCases()
{
	if (session)
	{
		delete routeChecker;
		delete pduChecker;
		delete receiver;
		delete transmitter;
		try
		{
			session->close();
		}
		catch(...)
		{
			//nothing
		}
		delete session;
	}
}

SmppReceiverTestCases& SmppTestCases::getReceiver()
{
	return *receiver;
}

SmppTransmitterTestCases& SmppTestCases::getTransmitter()
{
	return *transmitter;
}

Category& SmppTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppTestCases");
	return log;
}

TCResult* SmppTestCases::bindCorrectSme(int num)
{
	TCSelector s(num, 3);
	TCResult* res = new TCResult(TC_BIND_CORRECT_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //BIND_RECIEVER
					res->addFailure(1);
					break;
				case 2: //BIND_TRANSMITTER
					res->addFailure(2);
					break;
				case 3: //BIND_TRANCIEVER
					break;
				default:
					throw s;
			}
			session->connect();
		}
		catch(...)
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* SmppTestCases::bindIncorrectSme(int num)
{
	//��������� bind ����� �� ���� ������, �.�. �������� � ������������ ��������
	//��� ������
	TCSelector s(num, 3);
	TCResult* res = new TCResult(TC_BIND_INCORRECT_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //sme �� ���������������� � SC
					{
						SmppSession* sess = NULL;
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15); //15 �� ������������
							conf.sid = tmp.get();
							SmppSession* sess = new SmppSession(conf, receiver);
							sess->connect();
							delete sess;
						}
						catch(...)
						{
							if (sess) { delete sess; }
							throw;
						}
					}
					break;
				case 2: //bind �� ����������� SC (����������� ����)
					{
						SmppSession* sess = NULL;
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15);
							conf.host = tmp.get();
							SmppSession* sess = new SmppSession(conf, receiver);
							sess->connect();
							delete sess;
						}
						catch(...)
						{
							if (sess) { delete sess; }
							throw;
						}
					}
					break;
				case 3: //bind �� ����������� SC (������������ ����)
					{
						SmppSession* sess = NULL;
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15);
							conf.port += rand1(65535 - conf.port);
							SmppSession* sess = new SmppSession(conf, receiver);
							sess->connect();
							delete sess;
						}
						catch(...)
						{
							if (sess) { delete sess; }
							throw;
						}
					}
					break;
				default:
					res->addFailure(s.value());
			}
			res->addFailure(s.value());
		}
		catch(...)
		{
			//ok
		}
	}
	debug(res);
	return res;
}

#define __checkPdu__(tc) \
	__trace2__("%s(): checking pdu with msgRef = %u, submitTime = %d, waitTime = %d, validTime = %d", \
	tc, (uint32_t) pduData->msgRef, pduData->submitTime, pduData->waitTime, pduData->validTime)

#define __missingPdu__(tc, pduName) \
	__trace2__("%s(): missing %s for msgRef = %u, submitTime = %d, waitTime = %d, validTime = %d", \
	tc, pduName, (uint32_t) pduData->msgRef, pduData->submitTime, pduData->waitTime, pduData->validTime)
	/*
	static const char* fmt = "%Y-%m-%d %H:%M:%S"; \
	char __submitTime[20]; char __waitTime[20]; char __validTime[20]; \
	strftime(__submitTime, 20, fmt, localtime(&pduData->submitTime)); \
	strftime(__waitTime, 20, fmt, localtime(&pduData->waitTime)); \
	strftime(__validTime, 20, fmt, localtime(&pduData->validTime)); \
	__trace2__("%s(): missing %s for sequenceNumber = %u, submitTime = %s, waitTime = %s, validTime = %s", \
	tc, pduName, pduData->pdu->get_sequenceNumber(), __submitTime, __waitTime, __validTime)
	*/

#define __removedPdu__(tc) \
	__trace2__("%s(): removed pdu data with msgRef = %u", \
	tc, (uint32_t) pduData->msgRef)
	
#define __checkSummary__(tc) \
	__trace2__("%s(): found = %d, deleted = %d", tc, found, deleted);

int SmppTestCases::checkSubmitTime(time_t checkTime)
{
	int res = 0x0;
	//��������� ������������ �������� 
	PduRegistry::PduDataIterator* it = pduReg->getPduBySubmitTime(0, checkTime);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		__checkPdu__("SmppTestCases::checkSubmitTime");
		found++;
		if (pduData->responseFlag == PDU_REQUIRED_FLAG)
		{
			__missingPdu__("SmppTestCases::checkSubmitTime", "response");
			res |= 0x1;
			pduData->responseFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		if (pduData->complete())
		{
			 __removedPdu__("SmppTestCases::checkSubmitTime");
			deleted++;
			pduReg->removePdu(pduData);
		}
	}
	delete it;
	__checkSummary__("SmppTestCases::checkSubmitTime");
	return res;
}

int SmppTestCases::checkWaitTime(time_t checkTime)
{
	int res = 0x0;
	//��������� ������������ �������� � ������������� �������� 
	//�� ������ waitTime
	PduRegistry::PduDataIterator* it = pduReg->getPduByWaitTime(0, checkTime);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		__checkPdu__("SmppTestCases::checkWaitTime");
		found++;
		__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
		if (!routeChecker->isDestReachable(pdu->get_message().get_dest(), true))
		{
			continue;
		}
		if (pduData->deliveryFlag.isPduMissing(checkTime))
		{
			__missingPdu__("SmppTestCases::checkWaitTime", "deliver_sm");
			res |= 0x2;
			pduData->deliveryFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		if (pduData->deliveryReceiptFlag.isPduMissing(checkTime))
		{
			__missingPdu__("SmppTestCases::checkWaitTime", "delivery_receipt");
			res |= 0x4;
			pduData->deliveryReceiptFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		if (pduData->intermediateNotificationFlag == PDU_REQUIRED_FLAG)
		{
			__missingPdu__("SmppTestCases::checkWaitTime", "intermediate_notification");
			res |= 0x8;
			pduData->intermediateNotificationFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		if (pduData->complete())
		{
			__removedPdu__("SmppTestCases::checkWaitTime");
			deleted++;
			pduReg->removePdu(pduData);
		}
	}
	delete it;
	__checkSummary__("SmppTestCases::checkWaitTime");
	return res;
}

int SmppTestCases::checkValidTime(time_t checkTime)
{
	int res = 0x0;
	//��������� ������������ �������� � ������������� ��������
	//�� ��������� validTime
	PduRegistry::PduDataIterator* it = pduReg->getPduByValidTime(0, checkTime);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		__checkPdu__("SmppTestCases::checkValidTime");
		found++;
		__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
		if (!routeChecker->isDestReachable(pdu->get_message().get_dest(), true))
		{
			pduData->deliveryFlag = PDU_NOT_EXPECTED_FLAG;
		}
		if (pduData->deliveryFlag.isPduMissing(checkTime))
		{
			__missingPdu__("SmppTestCases::checkValidTime", "deliver_sm");
			res |= 0x2;
			pduData->deliveryFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		if (pduData->deliveryReceiptFlag.isPduMissing(checkTime))
		{
			__missingPdu__("SmppTestCases::checkValidTime", "delivery_receipt");
			res |= 0x4;
			pduData->deliveryReceiptFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		if (pduData->intermediateNotificationFlag == PDU_REQUIRED_FLAG)
		{
			__missingPdu__("SmppTestCases::checkValidTime", "intermediate_notification");
			res |= 0x8;
			pduData->intermediateNotificationFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		//��� pdu � validTime ������ �������� ��� ���� ��������� �������
		if (pduData->submitTime <= checkTime)
		{
			__removedPdu__("SmppTestCases::checkValidTime");
			deleted++;
			pduReg->removePdu(pduData);
		}
	}
	delete it;
	__checkSummary__("SmppTestCases::checkValidTime");
	return res;
}

TCResult* SmppTestCases::checkMissingPdu()
{
	TCResult* res = new TCResult(TC_CHECK_MISSING_PDU);
	if (pduReg)
	{
		MutexGuard mguard(pduReg->getMutex());
		time_t checkTime = time(NULL) - timeCheckAccuracy;
		int chk = checkSubmitTime(checkTime) | checkWaitTime(checkTime) | checkValidTime(checkTime);
		if (chk & 0x1) //������������ ��������
		{
			res->addFailure(1);
		}
		if (chk & 0x2) //������������ ��������
		{
			res->addFailure(2);
		}
		if (chk & 0x4) //������������ ������������� ��������
		{
			res->addFailure(3);
		}
		if (chk & 0x8) //������������ ������������� �����������
		{
			res->addFailure(4);
		}
		__trace2__("SmppTestCases::checkMissingPdu(): pduReg size = %d", pduReg->size());
	}
	debug(res);
	return res;
}

TCResult* SmppTestCases::unbind()
{
	TCResult* res = new TCResult(TC_UNBIND);
	try
	{
		session->close();
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

}
}
}

