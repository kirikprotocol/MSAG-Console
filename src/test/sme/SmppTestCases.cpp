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
	responseChecker = new SmppResponsePduChecker(pduReg, routeChecker);
	receiver = new SmppReceiverTestCases(systemId, smeAddr, smeReg,
		aliasReg, routeReg, handler, routeChecker, responseChecker);
	session = new SmppSession(config, receiver);
	receiver->setSession(session);
	transmitter = new SmppTransmitterTestCases(session, smeAddr,
		smeReg, responseChecker);
}

SmppTestCases::~SmppTestCases()
{
	if (session)
	{
		delete routeChecker;
		delete responseChecker;
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

TCResult* SmppTestCases::processInvalidSms()
{
	TCResult* res = new TCResult(TC_PROCESS_INVALID_SMS);
	if (pduReg)
	{
		MutexGuard(pduReg->getMutex());
		bool missingResp = false;
		bool missingDelivery = false;
		bool missingDeliveryReceipt = false;
		bool missingIntermediateNotification = false;
		//проверить неполученные респонсы
		PduRegistry::PduDataIterator* sit = pduReg->getPduBySubmitTime(0, __checkTime__);
		int sitCount = 0;
		while (PduData* pduData = sit->next())
		{
			sitCount++;
			if (!pduData->responseFlag)
			{
				__trace2__("SmppTestCases::processInvalidSms(): missing response for seqNum = %d",
					pduData->pdu->get_sequenceNumber());
				missingResp = true;
				pduData->responseFlag = true;
				if (pduData->complete())
				{
					pduReg->removePdu(pduData);
				}
			}
		}
		delete sit;
		__trace2__("SmppTestCases::processInvalidSms(): sitCount = %d", sitCount);
		//проверить неполученные доставки и подтверждения доставки 
		//на момент waitTime
		PduRegistry::PduDataIterator* wit = pduReg->getPduByWaitTime(0, __checkTime__);
		int witCount = 0;
		while (PduData* pduData = wit->next())
		{
			witCount++;
			if (pduData->deliveryFlag && pduData->deliveryReceiptFlag)
			{
				continue;
			}
			__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
			if (routeChecker->checkExistsUnreachableRoute(
				pdu->get_message().get_dest(), true))
			{
				continue;
			}
			if (!pduData->deliveryFlag)
			{
				__trace2__("SmppTestCases::processInvalidSms(): missing delivery for seqNum = %d",
					pduData->pdu->get_sequenceNumber());
				missingDelivery = true;
				pduData->deliveryFlag = true;
			}
			if (!pduData->deliveryReceiptFlag)
			{
				__trace2__("SmppTestCases::processInvalidSms(): missing delivery receipt for seqNum = %d",
					pduData->pdu->get_sequenceNumber());
				missingDeliveryReceipt = true;
				pduData->deliveryReceiptFlag = true;
			}
			if (pduData->complete())
			{
				pduReg->removePdu(pduData);
			}
		}
		delete wit;
		__trace2__("SmppTestCases::processInvalidSms(): witCount = %d", witCount);
		//проверить неполученные доставки и подтверждения доставки
		//по окончании validTime
		PduRegistry::PduDataIterator* vit = pduReg->getPduByValidTime(0, __checkTime__);
		int vitCount = 0;
		while (PduData* pduData = vit->next())
		{
			vitCount++;
			if (pduData->responseFlag && pduData->deliveryReceiptFlag)
			{
				continue;
			}
			__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
			if (routeChecker->checkExistsUnreachableRoute(
				pdu->get_message().get_dest(), true))
			{
				pduData->deliveryFlag = true;
				pduData->deliveryReceiptFlag = true;
			}
			if (!pduData->deliveryFlag)
			{
				__trace2__("SmppTestCases::processInvalidSms(): missing delivery for seqNum = %d",
					pduData->pdu->get_sequenceNumber());
				missingDelivery = true;
				pduData->deliveryFlag = true;
			}
			if (!pduData->deliveryReceiptFlag)
			{
				__trace2__("SmppTestCases::processInvalidSms(): missing delivery receipt for seqNum = %d",
					pduData->pdu->get_sequenceNumber());
				missingDeliveryReceipt = true;
				pduData->deliveryReceiptFlag = true;
			}
			if (!pduData->intermediateNotificationFlag)
			{
				__trace2__("SmppTestCases::processInvalidSms(): missing intermediate notification for seqNum = %d",
					pduData->pdu->get_sequenceNumber());
				missingIntermediateNotification = true;
				pduData->intermediateNotificationFlag = true;
			}
			pduReg->removePdu(pduData);
		}
		delete vit;
		__trace2__("SmppTestCases::processInvalidSms(): vitCount = %d", vitCount);
		if (missingResp) { res->addFailure(1); }
		if (missingDelivery) { res->addFailure(2); }
		if (missingDeliveryReceipt) { res->addFailure(3); }
		if (missingIntermediateNotification) { res->addFailure(4); }
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

