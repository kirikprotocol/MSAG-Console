#include "util/debug.h"
#include "sms/sms.h"
#include "sme/SmppBase.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "core/threads/ThreadPool.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/Util.hpp"
#include "readline/readline.h"
#include "readline/history.h"
#include <map>
#include <vector>
#include <iostream>

using smsc::sms::Address;
using namespace std;
using namespace smsc::sme;
using namespace smsc::core::threads;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::smpp;
using namespace smsc::test::util;

int numSme = 1;
const char* host = "smsc";
int port = 15971;
FILE* resFile = stderr;

PduAddress convert(const string& addr)
{
	PduAddress res;
	Address tmp(addr.c_str());
	res.set_typeOfNumber(tmp.type);
	res.set_numberingPlan(tmp.plan);
	res.set_value(tmp.value);
	return res;
}

class TestSme : public SmppPduEventListener, public ThreadedTask
{
	PduAddress addr;
	const string systemId;
	const string passwd;
	vector<PduAddress> destAddr;
	SmppSession* session;
	vector<SmppHeader*> pdu;
	int pos;

	void preparePdu(int num);
	void bind();
	void submit();

public:
	TestSme(const string& _addr, const string& _systemId, const string& _passwd,
		const vector<string>& _destAddr)
	: addr(convert(_addr)), systemId(_systemId), passwd(_passwd),
		session(NULL), pos(0)
	{
		__require__(_destAddr.size());
		destAddr.reserve(_destAddr.size());
		for (int i = 0; i < _destAddr.size(); i++)
		{
			destAddr.push_back(convert(_destAddr[i]));
		}
		preparePdu(100);
	}
	~TestSme()
	{
		if (session)
		{
			session->close();
			delete session;
		}
		for (int i = 0; i < pdu.size(); i++)
		{
			disposePdu(pdu[i]);
		}
	}
	virtual void handleEvent(SmppHeader* pdu);
	virtual void handleError(int errorCode);
	virtual int Execute();
	virtual const char* taskName() { return systemId.c_str(); }
};

void TestSme::preparePdu(int num)
{
	for (int i = 0; i < num; i++)
	{
		PduSubmitSm* p = new PduSubmitSm();
		/*
		p->get_message().set_serviceType("aaa");
		p->get_message().set_source(addr);
		p->get_message().set_dest(addr);
		p->get_message().set_esmClass(0);
		p->get_message().set_priorityFlag(0);
		p->get_message().set_scheduleDeliveryTime("");
		p->get_message().set_validityPeriod("");
		p->get_message().set_registredDelivery(0);
		p->get_message().set_replaceIfPresentFlag(0);
		p->get_message().set_dataCoding(0);
		p->get_message().set_shortMessage("Test message");
		*/
		SmppUtil::setupRandomCorrectSubmitSmPdu(p, true, false,
			OPT_ALL & ~OPT_USSD_SERVICE_OP & ~OPT_RCPT_MSG_ID);
		p->get_message().set_source(addr);
		p->get_message().set_dest(destAddr[i % destAddr.size()]);
		p->get_message().set_scheduleDeliveryTime("");
		p->get_message().set_validityPeriod("");
		p->get_message().set_registredDelivery(rand0(3) ? 0 : 1);
		pdu.push_back(reinterpret_cast<SmppHeader*>(p));
	}
}

void TestSme::bind()
{
	SmeConfig config;
	config.host = host;
	config.port = port;
	config.sid = systemId;
	config.timeOut = 8;
	config.password = passwd;
	__trace2__("bind(): host = %s, port = %d, systemId = %s, passwd = %s",
		config.host.c_str(), config.port, config.sid.c_str(), config.password.c_str());
	try
	{
		session = new SmppSession(config, this);
		session->connect();
		cout << "Sme " << systemId << " connected successfully" << endl;
	}
	catch (exception& e)
	{
		cout << "exception: " << e.what() << endl;
		exit(-1);
	}
	catch (...)
	{
		cout << "unknown exception" << endl;
		exit(-1);
	}
}

void TestSme::submit()
{
	try
	{
		int i = pos++ % pdu.size();
		timeb t1, t2;
		ftime(&t1);
		SmppHeader* respPdu = session->getSyncTransmitter()->sendPdu(pdu[i]);
		ftime(&t2);
		double t = (t2.time - t1.time) + (t2.millitm - t1.millitm) / 1000.0;
		fprintf(resFile, "%.3f\n", t);
	}
	catch (exception& e)
	{
		cout << "exception: " << e.what() << endl;
		exit(-1);
	}
	catch (...)
	{
		cout << "unknown exception" << endl;
		exit(-1);
	}
}

int TestSme::Execute()
{
	bind();
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
	//sme0 эмулирует map_proxy (все отправл€ют на sme0)
	vector<string> destAddr;
	destAddr.push_back("0");
	for (int i = 0; i < numSme; i++)
	{
		sprintf(addr, "%d", i);
		sprintf(smeId, "sme%d", i);
		threadPool.startTask(new TestSme(addr, smeId, "pass", destAddr));
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
	if (argc != 1 && argc != 2 && argc != 4)
	{
		cout << "Usage: RespDelayTest [numSme=1] [host=smsc] [port=15971]" << endl;
		exit(-1);
	}
	if (argc == 2)
	{
		numSme = atoi(argv[1]);
	}
	if (argc == 4)
	{
		numSme = atoi(argv[1]);
		host = argv[2];
		port = atoi(argv[3]);
	}
	executeTest();
}

