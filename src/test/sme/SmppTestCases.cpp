#include "SmppTestCases.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;

SmppTestCases::SmppTestCases(const SmeConfig& config, const SmeSystemId& _systemId,
	const Address& addr, const SmeRegistry* _smeReg, const AliasRegistry* _aliasReg,
	const RouteRegistry* _routeReg, ResultHandler* handler)
	: session(NULL), systemId(_systemId), smeAddr(addr), smeReg(_smeReg),
	aliasReg(_aliasReg), routeReg(_routeReg), resultHandler(handler)
{
	__require__(smeReg);
	__require__(aliasReg);
	__require__(routeReg);
	__require__(resultHandler);
	pduReg = smeReg->getPduRegistry(smeAddr); //может быть NULL
	routeChecker = new RouteChecker(aliasReg, routeReg);
	responseChecker = new SmppResponsePduChecker();
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

TCResult* SmppTestCases::bindRegisteredSme(int num)
{
	TCSelector s(num, 3);
	TCResult* res = new TCResult(TC_BIND_REGISTERED_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //BIND_RECIEVER
				case 2: //BIND_TRANSMITTER
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

TCResult* SmppTestCases::bindNonRegisteredSme(int num)
{
	TCSelector s(num, 3);
	TCResult* res = new TCResult(TC_BIND_NON_REGISTERED_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //BIND_RECIEVER
				case 2: //BIND_TRANSMITTER
				case 3: //BIND_TRANCIEVER
					break;
				default:
					throw s;
			}
			session->connect();
			res->addFailure(101);
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

