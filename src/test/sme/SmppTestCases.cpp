#include "SmppTestCases.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;

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
	static Category& log = Logger::getCategory("SmeTestCases");
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
	TCSelector s(num, 4);
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
				case 2: //повторный bind
					session->connect();
					break;
				case 3: //bind на недоступный SC (неизвестный хост)
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
				case 4: //bind на недоступный SC (неправильный порт)
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
					throw s;
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
		vector<PduData*> pduData = pduReg->getOverduePdu(time(NULL) - 5);
		bool missingResp = false;
		bool missingDelivery = false;
		bool missingDeliveryReceipt = false;
		bool missingIntermediateNotification = false;
		for (int i = 0; i < pduData.size(); i++)
		{
			missingResp |= !pduData[i]->responseFlag;
			missingDelivery |= !pduData[i]->deliveryFlag;
			missingDeliveryReceipt |= !pduData[i]->deliveryReceiptFlag;
			missingIntermediateNotification |= !pduData[i]->intermediateNotificationFlag;
			pduReg->removePdu(pduData[i]);
		}
		if (missingResp) { res->addFailure(1); }
		if (missingDelivery) { res->addFailure(2); }
		if (missingDeliveryReceipt) { res->addFailure(3); }
		if (missingIntermediateNotification) { res->addFailure(4); }
	}
	debug(res);
	return res;
}

TCResult* SmppTestCases::unbindBounded()
{
	TCResult* res = new TCResult(TC_UNBIND_BOUNDED);
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

TCResult* SmppTestCases::unbindNonBounded()
{
	TCResult* res = new TCResult(TC_UNBIND_NON_BOUNDED);
	try
	{
		session->close();
		res->addFailure(1);
	}
	catch(...)
	{
		//ok
	}
	debug(res);
	return res;
}

}
}
}

