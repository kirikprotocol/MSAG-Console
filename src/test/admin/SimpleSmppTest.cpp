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

class SimpleSmppTest : public SmppPduEventListener
{
	const string host;
	const int port;
	typedef map<const string, SmppSession*> Sessions;
	Sessions sessions;
	string selectedId;

public:
	SimpleSmppTest(const string& _host, int _port)
		: host(host), port(port) {}
	void bind(const string& systemId, const string& passwd);
	void select(const string& systemId);
	void unbind();
	void submit(const string& src, const string& dest);
	virtual void handleEvent(SmppHeader* pdu);
	virtual void handleError(int errorCode);
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
		SmppSession* sess = new SmppSession(config, this);
		sess->connect();
		sessions[systemId] = sess;
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

#define __check_selected__ \
	Sessions::iterator it = sessions.find(selectedId); \
	SmppSession* selectedSess = it == sessions.end() ? NULL : it->second; \
	if (!selectedSess) { \
		cout << "no session selected" << endl; \
		return; \
	}

void SimpleSmppTest::unbind()
{
	__check_selected__;
	try
	{
		selectedSess->close();
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
		pdu.get_optional().set_userMessageReference(++msgRef);
		PduSubmitSmResp* respPdu = selectedSess->getSyncTransmitter()->submit(pdu);
		cout << "submit: sme = " << selectedId <<
			", seqNum = " << pdu.get_header().get_sequenceNumber() <<
			", commandStatus = " << respPdu->get_header().get_commandStatus() <<
			", msgRef = " << pdu.get_optional().get_userMessageReference() << endl;
	}
	__check_exception__
}

void SimpleSmppTest::handleEvent(SmppHeader* pdu)
{
	__require__(pdu);
	dumpPdu("handleEvent", selectedId, pdu);
	switch (pdu->get_commandId())
	{
		case GENERIC_NACK:
			cout << "handle generick_nack: sme = " << selectedId <<
				", seqNum = " << pdu->get_sequenceNumber() <<
				", commandStatus = " << pdu->get_commandStatus() << endl;
			break;
		case SUBMIT_SM_RESP:
			cout << "handle submit_sm_resp: sme = " << selectedId <<
				", seqNum = " << pdu->get_sequenceNumber() <<
				", commandStatus = " << pdu->get_commandStatus() << endl;
			break;
		case DELIVERY_SM:
			{
				PduDeliverySm* p = reinterpret_cast<PduDeliverySm*>(pdu);
				__require__(p->get_optional().has_userMessageReference());
				uint16_t msgRef = p->get_optional().get_userMessageReference();
				cout << "handle delivery_sm: sme = " << selectedId <<
					", msgRef = " << (int) msgRef << endl;
			}
			break;
		default:
			cout << "handle pdu: commandId = " << pdu->get_commandId() << endl;
	}
}

void SimpleSmppTest::handleError(int errorCode)
{
	cout << "fatal error = " << errorCode << " in sme = " << selectedId << endl;
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

