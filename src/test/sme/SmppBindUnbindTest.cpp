#include "sms/sms.h"
#include "smeman/smeinfo.h"
#include "sme/SmppBase.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Event.hpp"
#include "test/util/Util.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/core/SmeRegistry.hpp"
#include "test/core/AliasRegistry.hpp"
#include "test/core/RouteRegistry.hpp"
#include "test/config/SmeConfigGen.hpp"
#include "test/config/AliasConfigGen.hpp"
#include "test/config/RouteConfigGen.hpp"
#include "test/config/ConfigUtil.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"

using smsc::sms::Address;
using smsc::smeman::SmeInfo;
using smsc::router::RouteInfo;
using smsc::core::threads::ThreadPool;
using smsc::core::threads::ThreadedTask;
using smsc::core::synchronization::Event;
using smsc::test::smeman::SmeManagerTestCases;
using namespace std;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::test::core;
using namespace smsc::test::config;
using namespace smsc::test::smpp; //SmppUtil, constants
using namespace smsc::test::util;

class TestSme : public ThreadedTask, public SmppPduEventListener
{
	const string smscHost;
	const int smscPort;
	const Address smeAddr;
	const SmeInfo smeInfo;
	vector<SmppSession*> sess;
	Event event;
	int count;

	SmppTransmitter* getTransmitter(bool sync);

public:
	TestSme(const string& smscHost, int smscPort, const Address& smeAddr,
		const SmeInfo& smeInfo);
	~TestSme();
	int getCount() { return count; }
	void bind();
	void unbind();
	void submitSm();
	void permanentAppError(PduDeliverySm& pdu);
	void genericNack(PduDeliverySm& pdu);
	void handleDeliverSm(PduDeliverySm& pdu);
	virtual void handleEvent(SmppHeader* pdu);
	virtual void handleError(int errorCode);
	virtual int Execute();
	virtual const char* taskName() { return "TestSme"; }
};

TestSme::TestSme(const string& _smscHost, int _smscPort, const Address& _smeAddr,
	const SmeInfo& _smeInfo) : smscHost(_smscHost), smscPort(_smscPort),
	smeAddr(_smeAddr), smeInfo(_smeInfo), count(0) {}

TestSme::~TestSme()
{
	for (int i = 0; i < sess.size(); i++)
	{
		delete sess[i];
	}
}

SmppTransmitter* TestSme::getTransmitter(bool sync)
{
	return (sync ? sess.back()->getSyncTransmitter() : sess.back()->getAsyncTransmitter());
}

void TestSme::bind()
{
	__trace2__("bind(): sme = %p", this);
	SmeConfig conf;
	conf.host = smscHost;
	conf.port = smscPort;
	conf.sid = smeInfo.systemId;
	conf.timeOut = smeInfo.timeout;
	conf.password = smeInfo.password;
	sess.push_back(new SmppSession(conf, this));
	sess.back()->connect();
	//почистить лишние сессии
	while (sess.size() > 3)
	{
		delete sess.front();
		sess.erase(sess.begin());
	}
}

void TestSme::unbind()
{
	__trace2__("unbind(): sme = %p", this);
	sess.back()->close();
}

void TestSme::submitSm()
{
	PduSubmitSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, OPT_ALL);
	PduAddress addr;
	SmppUtil::convert(smeAddr, &addr);
	pdu.get_message().set_source(addr);
	pdu.get_message().set_dest(addr);
	pdu.get_message().set_esmClass(ESM_CLASS_NORMAL_MESSAGE);
	pdu.get_message().set_scheduleDeliveryTime("");
	pdu.get_message().set_registredDelivery(0);
	getTransmitter(rand0(1))->submit(pdu);
	__trace2__("submitSm(): sme = %p, pdu = %u",
		this, pdu.get_header().get_sequenceNumber());
}

void TestSme::permanentAppError(PduDeliverySm& pdu)
{
	PduDeliverySmResp respPdu;
	respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
	respPdu.get_header().set_commandStatus(ESME_RX_P_APPN);
	getTransmitter(rand0(1))->sendDeliverySmResp(respPdu);
	__trace2__("permanentAppError(): sme = %p, pdu = %u",
		this, respPdu.get_header().get_sequenceNumber());
}

void TestSme::genericNack(PduDeliverySm& pdu)
{
	PduGenericNack respPdu;
	respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
	respPdu.get_header().set_commandStatus(rand1(INT_MAX));
	getTransmitter(rand0(1))->sendGenericNack(respPdu);
	__trace2__("genericNack(): sme = %p, pdu = %u",
		this, respPdu.get_header().get_sequenceNumber());
}

void TestSme::handleDeliverSm(PduDeliverySm& pdu)
{
	switch (rand1(3))
	{
		case 1:
			unbind();
			break;
		case 2:
			permanentAppError(pdu);
			break;
		case 3:
			genericNack(pdu);
			break;
		default:
			__unreachable__("");
	}
	event.Signal();
}

void TestSme::handleEvent(SmppHeader* p)
{
	switch (p->get_commandId())
	{
		case SUBMIT_SM_RESP:
			__trace2__("handleEvent(): sme = %p, pdu = %u, type = submit_sm_resp",
				this, p->get_sequenceNumber());
			break;
		case DELIVERY_SM:
			__trace2__("handleEvent(): sme = %p, pdu = %u, type = deliver_sm",
				this, p->get_sequenceNumber());
			handleDeliverSm(*reinterpret_cast<PduDeliverySm*>(p));
			break;
		default:
			__unreachable__("Invalid pdu type");
	}
}

void TestSme::handleError(int errorCode)
{
	__unreachable__("handleError()");
}

int TestSme::Execute()
{
	try
	{
		while (!isStopping)
		{
			bind();
			submitSm();
			event.Wait(5000);
			count++;
		}
	}
	catch(...)
	{
		__warning2__("exception in sme = %p", this);
	}
}

vector<TestSme*> genConfig(int numSme, const string& smscHost, int smscPort)
{
	vector<TestSme*> sme;
	SmeRegistry smeReg;
	AliasRegistry aliasReg;
	RouteRegistry routeReg;
	//системные sme
	ConfigUtil cfgUtil(&smeReg, &aliasReg, &routeReg);
	cfgUtil.setupSystemSme();
	//тестовые sme
	for (int i = 0; i < numSme; i++)
	{
		Address smeAddr;
		SmeInfo smeInfo;
		SmsUtil::setupRandomCorrectAddress(&smeAddr);
		SmeManagerTestCases::setupRandomCorrectSmeInfo(&smeInfo);
		smeReg.registerSme(smeAddr, smeInfo);
		//маршрут на самого себя
		RouteInfo route;
		route.enabling = true;
		route.routeId = i;
		route.smeSystemId = smeInfo.systemId;
		route.source = smeAddr;
		route.dest = smeAddr;
		routeReg.putRoute(route, NULL);
		sme.push_back(new TestSme(smscHost, smscPort, smeAddr, smeInfo));
	}
	//сохранение конфигов
	SmeConfigGen smeCfg(&smeReg, NULL);
	AliasConfigGen aliasCfg(&aliasReg, NULL);
	RouteConfigGen routeCfg(&routeReg, NULL);
	smeCfg.saveConfig("../conf/sme.xml");
	aliasCfg.saveConfig("../conf/aliases.xml");
	routeCfg.saveConfig("../conf/routes.xml");
	return sme;
}

void executeTest(const string& smscHost, int smscPort)
{
	vector<TestSme*> sme;
	ThreadPool threadPool;
	//обработка команд консоли
	bool help = true;
	string cmdLine;
	while(help || getline(cin, cmdLine))
	{
		//хелп
		if (help)
		{
			help = false;
			cout << "conf <numSme> - generate config files" << endl;
			cout << "test start - start test execution" << endl;
			cout << "stat - print statistics" << endl;
			cout << "quit - quit test" << endl;
			continue;
		}
		//обработка команд
		istringstream is(cmdLine);
		string cmd;
		is >> cmd;
		if (cmd == "conf")
		{
			int numSme = -1;
			is >> numSme;
			if (numSme < 0)
			{
				cout << "Required parameters missing" << endl;
				continue;
			}
			else
			{
				sme = genConfig(numSme, smscHost, smscPort);
				cout << "Config generated" << endl;
				continue;
			}
		}
		else if (cmd == "test")
		{
			is >> cmd;
			if (cmd == "start")
			{
				threadPool.preCreateThreads(sme.size());
				for (int i = 0; i < sme.size(); i++)
				{
					threadPool.startTask(sme[i]);
				}
				cout << "Started " << sme.size() << " sme" << endl;
				continue;
			}
		}
		else if (cmd == "stat")
		{
			for (int i = 0; i < sme.size(); i++)
			{
				cout << "sme[" << i << "]: " << sme[i]->getCount() << endl;
			}
			continue;
		}
		else if (cmd == "quit")
		{
			threadPool.shutdown();
			return;
		}
		help = true;
	}
}

int main(int argc, char* argv[])
{
	if (argc != 1 && argc != 3)
	{
		cout << "Usage: TestSmsc [host] [port]" << endl;
		exit(0);
	}
	string smscHost = "smsc";
	int smscPort = 15971;
	if (argc == 3)
	{
		smscHost = argv[1];
		smscPort = atoi(argv[2]);
	}
	executeTest(smscHost, smscPort);
}
