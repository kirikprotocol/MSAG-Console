#include "sme/SmppBase.hpp"
#include "test/util/Util.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"
#include "readline/readline.h"
#include "readline/history.h"

using namespace std;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::smpp;
using namespace smsc::test::util;

/*
 *  онфигураци€ теста.
 * —писок sme:
 * <ul>
 * <li>smeId=sme1, addr=111, alias=1110 - базова€ sme, только эта sme отправл€ет
 * submit_multi, * все остальные получают deliver_sm и отправл€ют deliver_sm_resp
 * с command_status = ESME_ROK
 * <li>smeId=sme0, addr=000, alias=0000 - sme не забинтована вообще
 * <li>smeId=sme2, addr=222, alias=2220 - transmitter
 * <li>smeId=sme3, addr = 333, alias=3330 - transceiver
 * <li>smeId=sme4, addr = 444, alias=4440 - transceiver
 * <li>smeId=MAP_PROXY, addr = 555, alias=5550 - transceiver, map proxy
 * </ul>
 *
 * —писок маршрутов:
 * <ul>
 * <li>111 -X-> 000 - маршрут не прописан
 * <li>111 ---> 222 на sme2
 * <li>111 ---> 333 на sme3
 * <li>111 ---> 444 на sme4
 * <li>111 ---> 555 на MAP_PROXY
 * </ul>
 *
 * —писки рассылки:
 * <ul>
 * <li>list0: несуществующий список рассылки
 * <li>list1: members={3330, 0000}, submitters={111}
 * <li>list2: members={4440, 2220}, submitters={111}
 * <li>list3: members={пусто}, submitters={111}
 * <li>list4: members={пусто}, submitters={пусто, нет прав на submit}
 * </ul>
*/

class SessionListener : public SmppPduEventListener
{
	const string systemId;
	SmppSession* session;

public:
	SessionListener(const string& _systemId) : systemId(_systemId) {}
	void setSession(SmppSession* _session) { session = _session; }
	virtual void handleEvent(SmppHeader* pdu)
	{
		dumpPdu("handleEvent", systemId, pdu);
		if (pdu->get_commandId() == DELIVERY_SM)
		{
			PduDeliverySmResp respPdu;
			respPdu.get_header().set_commandStatus(ESME_ROK);
			respPdu.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
			session->getAsyncTransmitter()->sendDeliverySmResp(respPdu);
			dumpPdu("sendResp", systemId, reinterpret_cast<SmppHeader*>(&respPdu));
		}
	}
	virtual void handleError(int errorCode)
	{
		__warning2__("handleError(): systemId = %s, errorCode = %d",
			systemId.c_str(), errorCode);
		exit(-1);
	}
};

struct SessionData
{
	SmeConfig config;
	PduAddress addr;
	SmppSession* session;
	SessionListener* listener;

	SessionData() : session(NULL), listener(NULL) {}
};

class SubmitMultiTest
{
	const string host;
	const int port;
	vector<SessionData*> smeList;
	SessionData* sme;
	
	SessionData* bindSme(const string& addr, const string& systemId,
		const string& passwd, int bindType);
	void submitMulti(PduMultiSm& pdu);
public:
	SubmitMultiTest(const string& _host, int _port)
		: host(_host), port(_port), sme(NULL) { srand(time(NULL)); }
	void bindAllSme();
	void submitMultiSeveralDistLists();
	void submitMultiSeveralAddresses();
	void submitMultiEmptyDistList();
	void submitMultiNotPrincipalDistList();
	void submitMultiNotPrincipalDistList2();
	void submitMultiNonExistentDistList();
	void submitMultiNoRecipient();
	void submitMultiMap();
};

SessionData* SubmitMultiTest::bindSme(const string& addr, const string& systemId,
	const string& passwd, int bindType)
{
	SessionData* data = new SessionData();
	const Address a(addr.c_str());
	PduAddress pduAddr;
	data->addr = *SmppUtil::convert(a, &pduAddr);
	data->config.host = host;
	data->config.port = port;
	data->config.sid = systemId;
	data->config.timeOut = 8;
	data->config.password = passwd;
	try
	{
		data->listener = new SessionListener(systemId);
		data->session = new SmppSession(data->config, data->listener);
		data->listener->setSession(data->session);
		data->session->connect(bindType);
		__trace2__("session %s connected: bindType = %d", systemId.c_str(), bindType);
		return data;
	}
	catch (SmppConnectException& e)
	{
		__warning2__("session %s connect failed: %s", systemId.c_str(), e.what());
		exit(-1);
	}
}

void SubmitMultiTest::bindAllSme()
{
	sme = bindSme("111", "sme1", "sme1", BindType::Transceiver);

	smeList.push_back(bindSme("222", "sme2", "sme2", BindType::Transmitter));
	smeList.push_back(bindSme("333", "sme3", "sme3", BindType::Transceiver));
	smeList.push_back(bindSme("444", "sme4", "sme4", BindType::Transceiver));
	smeList.push_back(bindSme("555", "MAP_PROXY", "sme5", BindType::Transceiver));
}

void check(SmppHeader* pdu, const string& sid)
{
	//serialize
	__trace__("serialization started");
	int sz = calcSmppPacketLength(pdu);
	char buf[sz];
	SmppStream s1;
	assignStreamWith(&s1, buf, sz, false);
	if (!fillSmppPdu(&s1, pdu)) throw Exception("Failed to fill smpp packet");
	__trace__("serialization finished");
	//deserialize
	__trace__("deserialization started");
    SmppStream s2;
    assignStreamWith(&s2, buf, sz, true);
    SmppHeader* header = fetchSmppPdu(&s2);
	dumpPdu("submitMultiDeserialized", sid, header);
	__trace__("deserialization finished");
}

void SubmitMultiTest::submitMulti(PduMultiSm& pdu)
{
	try
	{
		static int msgRef = 1;
		pdu.get_header().set_commandId(SUBMIT_MULTI);
		pdu.get_message().set_source(sme->addr);
		pdu.get_message().set_scheduleDeliveryTime("");
		pdu.get_message().set_validityPeriod("");
		pdu.get_optional().set_userMessageReference(msgRef++);
		//dumpPdu("submitMultiBefore", sme->config.sid, reinterpret_cast<SmppHeader*>(&pdu));
		//check(reinterpret_cast<SmppHeader*>(&pdu), sme->config.sid);
		sme->session->getAsyncTransmitter()->submitm(pdu);
		dumpPdu("submitMultiAfter", sme->config.sid, reinterpret_cast<SmppHeader*>(&pdu));
	}
	catch (exception& e)
	{
		__warning2__("exception: %s", e.what());
	}
	catch (...)
	{
		__warning__("unknown exception caught");
	}
}

//несколько списков рассылки (не map)
void SubmitMultiTest::submitMultiSeveralDistLists()
{
	__trace__("*** submitMultiSeveralDistLists ***");
	PduMultiSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, rand0(1), false);
	PduDestAddress dests[2];
	dests[0].set_flag(2); //Distribution List Name
	dests[0].set_value("list1");
	dests[1].set_flag(2); //Distribution List Name
	dests[1].set_value("list2");
	pdu.get_message().set_dests(dests, 2);
	submitMulti(pdu);
	sleep(160);
}

//несколько адресов (не map)
void SubmitMultiTest::submitMultiSeveralAddresses()
{
	__trace__("*** submitMultiSeveralAddresses ***");
	PduAddress tmp;
	PduMultiSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, rand0(1), false);
	PduDestAddress dests[3];
	dests[0].set_flag(1); //SME Address
	static_cast<PduAddress&>(dests[0]) = *SmppUtil::convert("0000", &tmp);
	dests[1].set_flag(1); //SME Address
	static_cast<PduAddress&>(dests[1]) = *SmppUtil::convert("2220", &tmp);
	dests[2].set_flag(1); //SME Address
	static_cast<PduAddress&>(dests[2]) = *SmppUtil::convert("3330", &tmp);
	pdu.get_message().set_dests(dests, 3);
	submitMulti(pdu);
	sleep(160);
}

//пустой список рассылки
void SubmitMultiTest::submitMultiEmptyDistList()
{
	__trace__("*** submitMultiEmptyDistList ***");
	PduMultiSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, rand0(1), false);
	PduDestAddress dest;
	dest.set_flag(2); //Distribution List Name
	dest.set_value("list3");
	pdu.get_message().set_dests(&dest, 1);
	submitMulti(pdu);
	sleep(5);
}

//нет прав отправл€ть на список рассылки
void SubmitMultiTest::submitMultiNotPrincipalDistList()
{
	__trace__("*** submitMultiNotPrincipalDistList ***");
	PduMultiSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, rand0(1), false);
	PduDestAddress dest;
	dest.set_flag(2); //Distribution List Name
	dest.set_value("list4");
	pdu.get_message().set_dests(&dest, 1);
	submitMulti(pdu);
	sleep(5);
}

//нет прав отправл€ть на список рассылки
void SubmitMultiTest::submitMultiNotPrincipalDistList2()
{
	__trace__("*** submitMultiNotPrincipalDistList2 ***");
	PduMultiSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, rand0(1), false);
	PduDestAddress dest[2];
	dest[0].set_flag(2); //Distribution List Name
	dest[0].set_value("list4");
	dest[1].set_flag(2); //Distribution List Name
	dest[1].set_value("list1");
	pdu.get_message().set_dests(dest, 2);
	submitMulti(pdu);
	sleep(5);
}

//несуществующий список рассылки
void SubmitMultiTest::submitMultiNonExistentDistList()
{
	__trace__("*** submitMultiNonExistentDistList ***");
	PduMultiSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, rand0(1), false);
	PduDestAddress dest;
	dest.set_flag(2); //Distribution List Name
	dest.set_value("list5");
	pdu.get_message().set_dests(&dest, 1);
	submitMulti(pdu);
	sleep(5);
}

//не задано адресатов
void SubmitMultiTest::submitMultiNoRecipient()
{
	__trace__("*** submitMultiNoRecipient ***");
	PduMultiSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, rand0(1), false);
	PduDestAddress dest[0];
	pdu.get_message().set_dests(dest, 0);
	submitMulti(pdu);
	sleep(5);
}

//длинное сообщение на map с udhi
//длинное сообщение на map c разрезанием
void SubmitMultiTest::submitMultiMap()
{
	__trace__("*** submitMultiMap ***");
	PduAddress tmp;
	PduMultiSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, rand0(1), false);
	//PduDestAddress dests[2];
	PduDestAddress dest;
	dest.set_flag(1); //SME Address
	static_cast<PduAddress&>(dest) = *SmppUtil::convert("555", &tmp);
	pdu.get_message().set_dests(&dest, 1);
	submitMulti(pdu);
	sleep(5);
}

int main(int argc, char* argv[])
{
	if (argc != 1 && argc != 3)
	{
		printf("Usage: TestSmsc [host] [port]");
		exit(0);
	}
	string smscHost = "smsc";
	int smscPort = 15971;
	if (argc == 3)
	{
		smscHost = argv[1];
		smscPort = atoi(argv[2]);
	}
	SubmitMultiTest test(smscHost, smscPort);
	test.bindAllSme();
	test.submitMultiSeveralDistLists();
	test.submitMultiSeveralAddresses();
	test.submitMultiEmptyDistList();
	test.submitMultiNoRecipient();
	test.submitMultiNotPrincipalDistList();
	test.submitMultiNotPrincipalDistList2();
	test.submitMultiNonExistentDistList();
	test.submitMultiMap();
	while (true)
	{
		//обработка команд
		const string cmd = readline(">");
		if (!cmd.length())
		{
			continue;
		}
		add_history(cmd.c_str());
		if (cmd == "quit")
		{
			exit(0);
		}
	}
}

