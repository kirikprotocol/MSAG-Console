#include "sme/SmppBase.hpp"
#include "test/util/Util.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

using namespace std;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::smpp;
using namespace smsc::test::util;

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
		: host(_host), port(_port), sme(NULL) {}
	void bindAllSme();
	void submitMultiSeveralDistLists();
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
		pdu.get_optional().set_userMessageReference(msgRef++);
		dumpPdu("submitMultiBefore", sme->config.sid, reinterpret_cast<SmppHeader*>(&pdu));
		check(reinterpret_cast<SmppHeader*>(&pdu), sme->config.sid);
		/*
		sme->session->getAsyncTransmitter()->submitm(pdu);
		dumpPdu("submitMultiAfter", sme->config.sid, reinterpret_cast<SmppHeader*>(&pdu));
		*/
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

//��������� ������� �������� (�� map)
void SubmitMultiTest::submitMultiSeveralDistLists()
{
	PduMultiSm pdu;
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, false, false, OPT_ALL & ~OPT_MSG_PAYLOAD);
	PduDestAddress dests[2];
	dests[0].set_flag(2); //Distribution List Name
	dests[0].set_value("list1");
	dests[1].set_flag(2); //Distribution List Name
	dests[1].set_value("list2");
	pdu.get_message().set_dests(dests);
	pdu.get_message().set_numberOfDests(2);
	submitMulti(pdu);
}

//��������� �������
//������ ������ ��������
//�������������� ������ ��������
//������� ��������� �� map � udhi
//������� ��������� �� map c �����������

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
}

