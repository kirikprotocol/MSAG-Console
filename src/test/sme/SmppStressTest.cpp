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

struct CorruptedPduTestCasesImpl : public CorruptedPduTestCases
{
	virtual void process(char* buf, int size);
};

class TestSme : public ThreadedTask, public SmppPduEventListener
{
	const string smscHost;
	const int smscPort;
	const Address smeAddr;
	const SmeInfo smeInfo;
	SmppSession* sess;
	Event event;
	int count;
	bool paused;
	Event pauseEvent;

	SmppTransmitter* getTransmitter(bool sync);

public:
	TestSme(const string& smscHost, int smscPort, const Address& smeAddr,
		const SmeInfo& smeInfo);
	~TestSme();
	int getCount() { return count; }
	void bind();
	void unbind();
	void submitSm();
	void sendCorruptedPdu(bool sync);
	void permanentAppError(PduDeliverySm& pdu);
	void genericNack(PduDeliverySm& pdu);
	void handleGenericNack(PduGenericNack* pdu);
	void handleSubmitSmResp(PduSubmitSmResp* pdu);
	void handleDeliverSm(PduDeliverySm* pdu);
	virtual void handleEvent(SmppHeader* pdu);
	virtual void handleError(int errorCode);
	virtual int Execute();
	virtual const char* taskName() { return "TestSme"; }
	void pause() { paused = true; }
	void resume() { paused = false; pauseEvent.Signal(); }
	void stop() { isStopping = true; pauseEvent.Signal(); }
};

TestSme::TestSme(const string& _smscHost, int _smscPort, const Address& _smeAddr,
	const SmeInfo& _smeInfo) : smscHost(_smscHost), smscPort(_smscPort),
	smeAddr(_smeAddr), smeInfo(_smeInfo), count(0), paused(false) {}

TestSme::~TestSme()
{
	if (sess)
	{
		delete sess;
	}
}

inline SmppTransmitter* TestSme::getTransmitter(bool sync)
{
	return (sync ? sess->getSyncTransmitter() : sess->getAsyncTransmitter());
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
	sess = new SmppSession(conf, this);
	sess->connect();
}

void TestSme::unbind()
{
	__trace2__("unbind(): sme = %p", this);
	delete sess; sess = NULL;
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
	bool sync = rand0(1);
	PduSubmitSmResp* respPdu = getTransmitter(sync)->submit(pdu);
	__trace2__("submitSm(): sme = %p, pdu = %u, sync = %s",
		this, pdu.get_header().get_sequenceNumber(), sync ? "true" : "false");
	if (sync)
	{
		if (respPdu)
		{
			handleSubmitSmResp(respPdu);
			delete respPdu;
		}
		else
		{
			__warning__("null response for sync request");
		}
	}
}

void TestSme::sendCorruptedPdu(bool sync)
{
	PduSubmitSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, OPT_ALL);
	CorruptedPduTestCasesImpl tc;
	sess->writer.tc = &tc;
	__trace2__("sendCorruptedPdu(): before submit");
	PduSubmitSmResp* respPdu = getTransmitter(sync)->submit(pdu);
	sess->writer.tc = NULL;
	__trace2__("sendCorruptedPdu(): sme = %p, pdu = %u, sync = %s",
		this, pdu.get_header().get_sequenceNumber(), sync ? "true" : "false");
	__require__(!respPdu);
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

void TestSme::handleGenericNack(PduGenericNack* pdu)
{
	__require__(pdu);
	__trace2__("handleGenericNack(): sme = %p, pdu:\n%s", this, str(*pdu).c_str());
}

void TestSme::handleSubmitSmResp(PduSubmitSmResp* pdu)
{
	__require__(pdu);
	__trace2__("handleSubmitSmResp(): sme = %p, pdu:\n%s", this, str(*pdu).c_str());
}

void TestSme::handleDeliverSm(PduDeliverySm* pdu)
{
	__require__(pdu);
	__trace2__("handleDeliverSm(): sme = %p, pdu:\n%s", this, str(*pdu).c_str());
}

void TestSme::handleEvent(SmppHeader* pdu)
{
	switch (pdu->get_commandId())
	{
		case SUBMIT_SM_RESP:
			handleSubmitSmResp(reinterpret_cast<PduSubmitSmResp*>(pdu));
			break;
		case DELIVERY_SM:
			handleDeliverSm(reinterpret_cast<PduDeliverySm*>(pdu));
			break;
		case GENERIC_NACK:
			handleGenericNack(reinterpret_cast<PduGenericNack*>(pdu));
			break;
		default:
			__unreachable__("Invalid pdu type");
	}
	disposePdu(pdu);
}

void TestSme::handleError(int errorCode)
{
	__trace2__("handleError(): errorCode = %d", errorCode);
	//__unreachable__("handleError()");
}

int TestSme::Execute()
{
	try
	{
		while (!isStopping)
		{
			bind();
			sendCorruptedPdu(rand0(1));
			event.Wait(5000);
			unbind();
			//unbind();
			//submitSm();
			count++;
			if (paused)
			{
				pauseEvent.Wait();
			}
		}
		return 0;
	}
	catch(...)
	{
		__warning2__("exception in sme = %p", this);
		return -1;
	}
}

void CorruptedPduTestCasesImpl::process(char* buf, int size)
{
	__require__(size >= 16);
	SmppHeader* header = reinterpret_cast<SmppHeader*>(buf);
	header->set_commandLength(rand2(0, 2 * size));
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
			cout << "test <start|pause|resume> - start/pause/resume test execution" << endl;
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
			else if (cmd == "pause")
			{
				for (int i = 0; i < sme.size(); i++)
				{
					sme[i]->pause();
				}
				cout << "Paused " << sme.size() << " sme" << endl;
				continue;
			}
			else if (cmd == "resume")
			{
				for (int i = 0; i < sme.size(); i++)
				{
					sme[i]->resume();
				}
				cout << "Resumed " << sme.size() << " sme" << endl;
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
			for (int i = 0; i < sme.size(); i++)
			{
				sme[i]->stop();
			}
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
