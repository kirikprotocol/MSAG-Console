#include "util/debug.h"
#include "sms/sms.h"
#include "sme/SmppBase.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "core/threads/ThreadPool.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "readline/readline.h"
#include "readline/history.h"
#include <map>
#include <iostream>

using smsc::sms::Address;
using namespace std;
using namespace smsc::sme;
using namespace smsc::core::threads;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::smpp;

const char* host = "localhost";
int port = 15971;
FILE* resFile = stderr;

class TestSme : public SmppPduEventListener, public ThreadedTask
{
	PduAddress addr;
	const string systemId;
	SmppSession* session;

	void bind(const string& systemId, const string& passwd);
	void submit();

public:
	TestSme(const string& _addr, const string& _systemId, const string& passwd)
		: systemId(_systemId)
	{
		Address tmp(_addr.c_str());
		addr.set_typeOfNumber(tmp.type);
        addr.set_numberingPlan(tmp.plan);
		addr.set_value(tmp.value);
		bind(systemId, passwd);
	}
	~TestSme()
	{
		session->close();
		delete session;
	}
	virtual void handleEvent(SmppHeader* pdu);
	virtual void handleError(int errorCode);
	virtual int Execute();
	virtual const char* taskName() { return systemId.c_str(); }
};

void TestSme::bind(const string& systemId, const string& passwd)
{
	SmeConfig config;
	config.host = host;
	config.port = port;
	config.sid = systemId;
	config.timeOut = 8;
	config.password = passwd;
	try
	{
		session = new SmppSession(config, this);
		session->connect();
		cout << "Sme " << systemId << " connected successfully" << endl;
	}
	catch (exception& e)
	{
		cout << "exception: " << e.what() << endl;
	}
}

void TestSme::submit()
{
	try
	{
		PduSubmitSm pdu;
		/*
		pdu.get_message().set_serviceType("aaa");
		pdu.get_message().set_source(addr);
		pdu.get_message().set_dest(addr);
		pdu.get_message().set_esmClass(0);
		pdu.get_message().set_priorityFlag(0);
		pdu.get_message().set_scheduleDeliveryTime("");
		pdu.get_message().set_validityPeriod("");
		pdu.get_message().set_registredDelivery(0);
		pdu.get_message().set_replaceIfPresentFlag(0);
		pdu.get_message().set_dataCoding(0);
		pdu.get_message().set_shortMessage("Test message");
		*/
		SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, true, false,
			OPT_ALL & ~OPT_USSD_SERVICE_OP);
		pdu.get_message().set_source(addr);
		pdu.get_message().set_dest(addr);
		pdu.get_message().set_scheduleDeliveryTime("");
		pdu.get_message().set_validityPeriod("");
		timeb t1, t2;
		ftime(&t1);
		PduSubmitSmResp* respPdu = session->getSyncTransmitter()->submit(pdu);
		ftime(&t2);
		double t = (t2.time - t1.time) + (t2.millitm - t1.millitm) / 1000.0;
		fprintf(resFile, "%.3f\n", t);
	}
	catch (exception& e)
	{
		cout << "exception: " << e.what() << endl;
	}
}

int TestSme::Execute()
{
	while (!isStopping)
	{
		submit();
	}
}

void TestSme::handleEvent(SmppHeader* pdu)
{
	__require__(pdu);
	switch (pdu->get_commandId())
	{
		case SUBMIT_SM_RESP:
		case DATA_SM_RESP:
			//ok
			break;
		case DELIVERY_SM:
		case DATA_SM:
			{
				PduDeliverySmResp respPdu;
				respPdu.get_header().set_commandStatus(ESME_ROK);
				respPdu.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
				session->getAsyncTransmitter()->sendDeliverySmResp(respPdu);
			}
			break;
		default:
			__trace2__("handleEvent(): invalid pdu received with commandId = %x", pdu->get_commandId());
			__unreachable__("fatal error");
	}
}

void TestSme::handleError(int errorCode)
{
	__trace2__("handleError(): errorCode = %d", errorCode);
	__unreachable__("fatal error");
}

void executeTest()
{
	ThreadPool threadPool;
	char addr[30];
	char smeId[10];
	for (int i = 0; i < 1; i++)
	{
		sprintf(addr, "+%d", i);
		sprintf(smeId, "sme%d", i);
		threadPool.startTask(new TestSme(addr, smeId, "pass"));
	}
    using_history();
	while (true)
	{
		const char* cmdLine = readline(">");
		if (!cmdLine)
		{
			continue;
		}
		if (!strcmp(cmdLine, "quit"))
		{
			return;
		}
		add_history(cmdLine);
	}
}

int main(int argc, char* argv[])
{
	if (argc != 1 && argc != 3)
	{
		cout << "Usage: SimpleSmppTest [host] [port]" << endl;
		exit(-1);
	}
	if (argc == 3)
	{
		host = argv[1];
		port = atoi(argv[2]);
	}
	executeTest();
}

