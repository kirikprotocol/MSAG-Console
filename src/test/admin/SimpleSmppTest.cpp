#include "sme/SmppBase.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "readline/readline.h"
#include "readline/history.h"
#include <map>
#include <iostream>

using smsc::sms::Address;
using namespace std;
using namespace smsc::sme;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::test::smpp;

class SimpleSmppListener : public SmppPduEventListener
{
	const string systemId;
	SmppSession* session;

public:
	SimpleSmppListener(const string& _systemId) : systemId(_systemId) {}
	void setSession(SmppSession* _session) { session = _session; }
	virtual void handleEvent(SmppHeader* pdu);
	virtual void handleError(int errorCode);
};

struct SessionData
{
	SmppSession* session;
	SimpleSmppListener* listener;
};

class SimpleSmppTest
{
	const string host;
	const int port;
	typedef map<const string, SessionData*> Sessions;
	Sessions sessions;
	string selectedId;

public:
	SimpleSmppTest(const string& _host, int _port)
		: host(_host), port(_port) {}
	void bind(const string& systemId, const string& passwd);
	void select(const string& systemId);
	void list();
	void unbind();
	void submit(const string& src, const string& dest);
};

#define __check_exception__ \
	catch (Exception& e) { cout << "exception: " << e.what() << endl; } \
	catch (...) { cout << "unknown exception" << endl; }

void SimpleSmppTest::bind(const string& systemId, const string& passwd)
{
	SmeConfig config;
	config.host = host;
	config.port = port;
	config.sid = systemId;
	config.timeOut = 8;
	config.password = passwd;
	try
	{
		SessionData* data = new SessionData();
		data->listener = new SimpleSmppListener(systemId);
		data->session = new SmppSession(config, data->listener);
		data->listener->setSession(data->session);
		data->session->connect();
		sessions[systemId] = data;
		select(systemId);
	}
	__check_exception__
}

void SimpleSmppTest::select(const string& systemId)
{
	Sessions::iterator it = sessions.find(systemId);
	if (it == sessions.end())
	{
		cout << "session not selected" << endl;
		return;
	}
	selectedId = systemId;
	cout << "session " << systemId << " selected" << endl;
}

void SimpleSmppTest::list()
{
	cout << "sessions:" << endl;
	for (Sessions::const_iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		cout << "  " << it->first << endl;
	}
}

#define __check_selected__ \
	Sessions::iterator it = sessions.find(selectedId); \
	SessionData* data = it == sessions.end() ? NULL : it->second; \
	if (!data) { \
		cout << "no session selected" << endl; \
		return; \
	}

void SimpleSmppTest::unbind()
{
	__check_selected__;
	try
	{
		data->session->close();
		delete data->session;
		delete data->listener;
		sessions.erase(it);
	}
	__check_exception__
}

void SimpleSmppTest::submit(const string& src, const string& dest)
{
	static uint16_t msgRef = 1;
	__check_selected__;
	try
	{
		PduSubmitSm pdu;
		SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, true);
		PduAddress addr;
		const Address srcAddr(src.c_str());
		const Address destAddr(dest.c_str());
		pdu.get_message().set_source(*SmppUtil::convert(srcAddr, &addr));
		pdu.get_message().set_dest(*SmppUtil::convert(destAddr, &addr));
		//неменденная доставка
		pdu.get_message().set_scheduleDeliveryTime("");
		pdu.get_message().set_validityPeriod("");
		pdu.get_optional().set_userMessageReference(++msgRef);
		PduSubmitSmResp* respPdu = data->session->getSyncTransmitter()->submit(pdu);
		dumpPdu("submit", selectedId, reinterpret_cast<SmppHeader*>(&pdu));
		char res[128];
		sprintf(res, "submit: sme = %s, seqNum = %u, commandStatus = 0x%x, msgRef = %u",
			selectedId.c_str(), pdu.get_header().get_sequenceNumber(),
			respPdu->get_header().get_commandStatus(),
			(uint32_t) pdu.get_optional().get_userMessageReference());
		cout << res << endl;
	}
	__check_exception__
}

void SimpleSmppListener::handleEvent(SmppHeader* pdu)
{
	__require__(pdu);
	dumpPdu("handleEvent", systemId, pdu);
	char res[128];
	switch (pdu->get_commandId())
	{
		case GENERIC_NACK:
			sprintf(res, "handle generick_nack: sme = %s, seqNum = %u, commandStatus = 0x%x",
				systemId.c_str(), pdu->get_sequenceNumber(), pdu->get_commandStatus());
			break;
		case SUBMIT_SM_RESP:
			sprintf(res, "handle submit_sm_resp: sme = %s, seqNum = %u, commandStatus = 0x%x",
				systemId.c_str(), pdu->get_sequenceNumber(), pdu->get_commandStatus());
			break;
		case DELIVERY_SM:
			{
				PduDeliverySm* p = reinterpret_cast<PduDeliverySm*>(pdu);
				__require__(p->get_optional().has_userMessageReference());
				sprintf(res, "handle delivery_sm: sme = %s, msgRef = %u",
					systemId.c_str(), (uint32_t) p->get_optional().get_userMessageReference());
			}
			break;
		default:
			sprintf(res, "handle pdu: sme = %s, commandId = 0x%x",
				systemId.c_str(), pdu->get_commandId());
	}
	cout << res << endl;
}

void SimpleSmppListener::handleError(int errorCode)
{
	char res[128];
	sprintf(res, "handle error: sme = %s, errorCode = %d",
		systemId.c_str(), errorCode);
	cout << res << endl;
}

void executeTest(const string& smscHost, int smscPort)
{
	SimpleSmppTest test(smscHost, smscPort);
	//обработка команд консоли
	bool help = true;
	string cmdLine, cmd;
	while(true)
	{
		//хелп
		if (help)
		{
			help = false;
			cout << "bind <systemId> <password> - bind sme" << endl;
			cout << "select <systemId> - select sme" << endl;
			cout << "list - list sessions" << endl;
			cout << "unbind - unbind selected sme" << endl;
			cout << "submit <src> <dest> - submit sms" << endl;
			cout << "quit - quit test" << endl;
			continue;
		}
		cmdLine = readline(">");
		istringstream is(cmdLine);
		is >> cmd;
		if (cmd == "bind")
		{
			string systemId;
			string passwd;
			is >> systemId;
			is >> passwd;
			if (systemId.length())
			{
				test.bind(systemId, passwd);
				continue;
			}
		}
		else if (cmd == "select")
		{
			string systemId;
			is >> systemId;
			if (systemId.length())
			{
				test.select(systemId);
				continue;
			}
		}
		else if (cmd == "list")
		{
			test.list();
			continue;
		}
		else if (cmd == "unbind")
		{
			test.unbind();
			continue;
		}
		else if (cmd == "submit")
		{
			string src, dest;
			is >> src;
			is >> dest;
			test.submit(src, dest);
			continue;
		}
		else if (cmd == "quit")
		{
			return;
		}
		help = true;
	}
}

int main(int argc, char* argv[])
{
	if (argc != 1 && argc != 3)
	{
		cout << "Usage: SimpleSmppTest [host] [port]" << endl;
		exit(-1);
	}
	string smscHost = "smsc";
	int smscPort = 15971;
	if (argc == 3)
	{
		smscHost = argv[1];
		smscPort = atoi(argv[2]);
	}
    using_history();
	executeTest(smscHost, smscPort);
}

