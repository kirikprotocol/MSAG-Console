#include "SmppTestCases.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using namespace smsc::smpp::SmppCommandSet;

SmppTestCases::SmppTestCases(const SmeConfig& _config, const SmeSystemId& _systemId,
	const Address& addr, const SmeRegistry* _smeReg, const AliasRegistry* _aliasReg,
	const RouteRegistry* _routeReg, ResultHandler* handler)
	: config(_config), session(NULL), systemId(_systemId), smeAddr(addr),
	smeReg(_smeReg), aliasReg(_aliasReg), routeReg(_routeReg), resultHandler(handler)
{
	__require__(smeReg);
	__require__(aliasReg);
	__require__(routeReg);
	__require__(resultHandler);
	pduReg = smeReg->getPduRegistry(smeAddr); //может быть NULL
	routeChecker = new RouteChecker(systemId, smeAddr, smeReg, aliasReg, routeReg);
	pduChecker = new SmppPduChecker(pduReg, routeChecker);
	receiver = new SmppReceiverTestCases(systemId, smeAddr, smeReg,
		aliasReg, routeReg, handler, routeChecker, pduChecker);
	session = new SmppSession(config, receiver);
	receiver->setSession(session);
	transmitter = new SmppTransmitterTestCases(session, systemId, smeAddr,
		smeReg, pduChecker);
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
	//повторный bind убран из тест кейсов, т.к. приводит к пересозданию коннекта
	//без ошибок
	TCSelector s(num, 3);
	TCResult* res = new TCResult(TC_BIND_INCORRECT_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //sme не зарегистрирована в SC
					{
						SmppSession* sess = NULL;
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15); //15 по спецификации
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
				case 2: //bind на недоступный SC (неизвестный хост)
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
				case 3: //bind на недоступный SC (неправильный порт)
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

#define __missingPdu__(tc, pduName) \
	__trace2__("%s(): missing %s for sequenceNumber = %u, submitTime = %d, waitTime = %d, validTime = %d", \
	tc, pduName, pduData->pdu->get_sequenceNumber(), pduData->submitTime, pduData->waitTime, pduData->validTime)
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
	__trace2__("%s(): removed pdu data with sequenceNumber = %u", \
	tc, pduData->pdu->get_sequenceNumber())
	
#define __checkSummary__(tc) \
	__trace2__("%s(): found = %d, deleted = %d", tc, found, deleted);

int SmppTestCases::checkSubmitTime()
{
	int res = 0x0;
	//проверить неполученные респонсы 
	PduRegistry::PduDataIterator* it = pduReg->getPduBySubmitTime(0, __checkTime__);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		found++;
		if (!pduData->responseFlag)
		{
			__missingPdu__("SmppTestCases::checkSubmitTime", "response");
			res |= 0x1;
			pduData->responseFlag = true;
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

int SmppTestCases::checkWaitTime()
{
	int res = 0x0;
	//проверить неполученные доставки и подтверждения доставки 
	//на момент waitTime
	PduRegistry::PduDataIterator* it = pduReg->getPduByWaitTime(0, __checkTime__);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		found++;
		__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
		if (!routeChecker->isDestReachable(pdu->get_message().get_dest(), true))
		{
			continue;
		}
		if (!pduData->deliveryFlag)
		{
			__missingPdu__("SmppTestCases::checkWaitTime", "deliver_sm");
			res |= 0x2;
			pduData->deliveryFlag = true;
		}
		if (!pduData->deliveryReceiptFlag)
		{
			__missingPdu__("SmppTestCases::checkWaitTime", "delivery_receipt");
			res |= 0x4;
			pduData->deliveryReceiptFlag = true;
		}
		if (!pduData->intermediateNotificationFlag)
		{
			__missingPdu__("SmppTestCases::checkWaitTime", "intermediate_notification");
			res |= 0x8;
			pduData->intermediateNotificationFlag = true;
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

int SmppTestCases::checkValidTime()
{
	int res = 0x0;
	//проверить неполученные доставки и подтверждения доставки
	//по окончании validTime
	PduRegistry::PduDataIterator* it = pduReg->getPduByValidTime(0, __checkTime__);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		found++;
		__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
		if (!routeChecker->isDestReachable(pdu->get_message().get_dest(), true))
		{
			pduData->deliveryFlag = true;
		}
		if (!pduData->deliveryFlag)
		{
			__missingPdu__("SmppTestCases::checkValidTime", "deliver_sm");
			res |= 0x2;
			pduData->deliveryFlag = true;
		}
		if (!pduData->deliveryReceiptFlag)
		{
			__missingPdu__("SmppTestCases::checkValidTime", "delivery_receipt");
			res |= 0x4;
			pduData->deliveryReceiptFlag = true;
		}
		if (!pduData->intermediateNotificationFlag)
		{
			__missingPdu__("SmppTestCases::checkValidTime", "intermediate_notification");
			res |= 0x8;
			pduData->intermediateNotificationFlag = true;
		}
		__removedPdu__("SmppTestCases::checkValidTime");
		deleted++;
		pduReg->removePdu(pduData);
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
		MutexGuard(pduReg->getMutex());
		int chk = checkSubmitTime() | checkWaitTime() | checkValidTime();
		if (chk & 0x1) //неполученные респонсы
		{
			res->addFailure(1);
		}
		if (chk & 0x2) //неполученные доставки
		{
			res->addFailure(2);
		}
		if (chk & 0x4) //неполученные подтверждения доставки
		{
			res->addFailure(3);
		}
		if (chk & 0x8) //неполученные промежуточные нотификации
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

