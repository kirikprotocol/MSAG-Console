#include "sme/SmppBase.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "readline/readline.h"
#include "readline/history.h"
#include <map>
#include <iostream>

using smsc::sms::Address;
using namespace std;
using namespace smsc::sme;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::DataCoding;
using namespace smsc::test::smpp;
using namespace smsc::test::util;

class SimpleSmppListener : public SmppPduEventListener
{
	const string systemId;
	SmppSession* session;
	uint32_t respCommandStatus;

public:
	SimpleSmppListener(const string& _systemId)
		: systemId(_systemId), respCommandStatus(0x0) {}
	void setSession(SmppSession* _session) { session = _session; }
	void setDeliveryResp(uint32_t commandStatus) { respCommandStatus = commandStatus; }
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
	void bind(int mode, const string& systemId, const string& passwd);
	void select(const string& systemId);
	const string& getSelected() { return selectedId; }
	void list();
	void unbind();
	void submit(const string& src, const string& dest,
		uint8_t registeredDelivery, bool ussd);
	void setDeliveryResp(uint32_t commandStatus);
};

#define __check_exception__ \
	catch (Exception& e) { cout << "exception: " << e.what() << endl; } \
	catch (...) { cout << "unknown exception" << endl; }

void SimpleSmppTest::bind(int mode, const string& systemId,
	const string& passwd)
{
	SmeConfig config;
	config.host = host;
	config.port = port;
	config.sid = systemId;
	config.timeOut = 8;
	config.password = passwd;
	__trace2__("bind(): host = %s, port = %d, systemId = %s, password = %s",
		host.c_str(), port, systemId.c_str(), passwd.c_str());
	try
	{
		SessionData* data = new SessionData();
		data->listener = new SimpleSmppListener(systemId);
		data->session = new SmppSession(config, data->listener);
		data->listener->setSession(data->session);
		data->session->connect(mode);
		sessions[systemId] = data;
		cout << "connected successfully" << endl;
		select(systemId);
	}
	catch (SmppConnectException& e)
	{
		cout << "connect exception: " << e.what() << endl;
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

void SimpleSmppTest::submit(const string& src, const string& dest,
	uint8_t registeredDelivery, bool ussd)
{
	static uint16_t msgRef = 1;
	__check_selected__;
	try
	{
		PduSubmitSm pdu;
		SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, true, false,
			OPT_ALL & ~OPT_USSD_SERVICE_OP);
		PduAddress addr;
		const Address srcAddr(src.c_str());
		const Address destAddr(dest.c_str());
		pdu.get_message().set_source(*SmppUtil::convert(srcAddr, &addr));
		pdu.get_message().set_dest(*SmppUtil::convert(destAddr, &addr));
		pdu.get_message().set_registredDelivery(registeredDelivery);
		//неменденная доставка
		pdu.get_message().set_scheduleDeliveryTime("");
		pdu.get_message().set_validityPeriod("");
		pdu.get_optional().set_userMessageReference(++msgRef);
		if (ussd)
		{
			pdu.get_optional().set_ussdServiceOp(1);
		}
		PduSubmitSmResp* respPdu = data->session->getSyncTransmitter()->submit(pdu);
		dumpPdu("submit", selectedId, reinterpret_cast<SmppHeader*>(&pdu));
		uint8_t dc = pdu.get_message().get_dataCoding();
		cout << "submit: sme = " << selectedId <<
			", seqNum = " << dec << pdu.get_header().get_sequenceNumber() <<
			", commandStatus = 0x" << hex << respPdu->get_header().get_commandStatus() <<
			", msgRef = " << dec << (int) pdu.get_optional().get_userMessageReference() <<
			", ussd = " << (ussd ? "yes" : "no") <<
			", dc = " << (int) dc;
		if (dc == DEFAULT || dc == UCS2 || dc == SMSC7BIT)
		{
			bool udhi = pdu.get_message().get_esmClass() & ESM_CLASS_UDHI_INDICATOR;
			int udlLen = !udhi ? 0 :
				1 + (unsigned char) *pdu.get_message().get_shortMessage();
			const char* text = pdu.get_message().get_shortMessage() + udlLen;
			int len = pdu.get_message().size_shortMessage() - udlLen;
			cout << ", udhi = " << (udhi ? "yes" : "no") <<
				", sm = " << decode(text, len, dc, false);
		}
		cout << endl;
	}
	__check_exception__
}

void SimpleSmppTest::setDeliveryResp(uint32_t commandStatus)
{
	__check_selected__;
	data->listener->setDeliveryResp(commandStatus);
}

void SimpleSmppListener::handleEvent(SmppHeader* pdu)
{
	__require__(pdu);
	dumpPdu("handleEvent", systemId, pdu);
	switch (pdu->get_commandId())
	{
		case GENERIC_NACK:
			cout << "handle generick_nack: sme = " << systemId <<
				", seqNum = " << dec << pdu->get_sequenceNumber() <<
				", commandStatus = 0x" << hex << pdu->get_commandStatus() << endl;
			break;
		case SUBMIT_SM_RESP:
			cout << "handle submit_sm_resp: sme = " << systemId <<
				", seqNum = " << dec << pdu->get_sequenceNumber() <<
				", commandStatus = 0x" << hex << pdu->get_commandStatus() << endl;
			break;
		case DELIVERY_SM:
			{
				PduDeliverySm* p = reinterpret_cast<PduDeliverySm*>(pdu);
				__require__(p->get_optional().has_userMessageReference());
				uint8_t dc = p->get_message().get_dataCoding();
				uint16_t msgRef = p->get_optional().get_userMessageReference();
				cout << "handle deliver_sm: sme = " << systemId <<
					", msgRef = " << dec << (int) msgRef <<
					", dc = " << (int) dc;
				if (dc == DEFAULT || dc == UCS2 || dc == SMSC7BIT)
				{
					bool udhi = p->get_message().get_esmClass() & ESM_CLASS_UDHI_INDICATOR;
					int udlLen = !udhi ? 0 :
						1 + (unsigned char) *p->get_message().get_shortMessage();
					const char* text = p->get_message().get_shortMessage() + udlLen;
					int len = p->get_message().size_shortMessage() - udlLen;
					cout << ", udhi = " << (udhi ? "yes" : "no") <<
						", sm = " << decode(text, len, dc, false);
				}
				cout << endl;
				PduDeliverySmResp respPdu;
				respPdu.get_header().set_commandStatus(respCommandStatus);
				respPdu.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
				session->getAsyncTransmitter()->sendDeliverySmResp(respPdu);
				dumpPdu("send resp", systemId, reinterpret_cast<SmppHeader*>(&respPdu));
				cout << "deliver resp: sme = " << systemId <<
					", seqNum = " << dec << respPdu.get_header().get_sequenceNumber() <<
					", commandStatus = 0x" << hex << respPdu.get_header().get_commandStatus() << endl;
			}
			break;
		default:
			cout << "handle pdu: sme = " << systemId <<
				", commandId = 0x" << hex << pdu->get_commandId() << endl;
	}
}

void SimpleSmppListener::handleError(int errorCode)
{
	cout << "handle error: sme = " << systemId <<
		", errorCode = " << dec << errorCode << endl;
}

void executeTest(const string& smscHost, int smscPort)
{
	SimpleSmppTest test(smscHost, smscPort);
	//обработка команд консоли
	bool help = true;
	while(true)
	{
		//хелп
		if (help)
		{
			help = false;
			cout << "bind <mode> <systemId> <password> - bind sme" << endl;
			cout << "select <systemId> - select sme" << endl;
			cout << "list - list sessions" << endl;
			cout << "unbind - unbind selected sme" << endl;
			cout << "submit <src> <dest> [report=none|full|failure] [ussd=no|yes] - submit sms" << endl;
			cout << "set resp [commandStatus=0] - set delivery_sm_resp command status" << endl;
			cout << "quit - quit test" << endl;
			continue;
		}
		string prompt = test.getSelected() + ">";
		const char* cmdLine = readline(prompt.c_str());
		if (!cmdLine)
		{
			return;
		}
		add_history(cmdLine);
		istringstream is(cmdLine);
		string cmd;
		is >> cmd;
		if (cmd == "bind")
		{
			string strMode;
			string systemId;
			string passwd;
			is >> strMode;
			is >> systemId;
			is >> passwd;
			if (strMode == "rx" && systemId.length())
			{
				test.bind(BindType::Receiver, systemId, passwd);
				continue;
			}
			else if (strMode == "tx" && systemId.length())
			{
				test.bind(BindType::Transmitter, systemId, passwd);
				continue;
			}
			else if (strMode == "trx" && systemId.length())
			{
				test.bind(BindType::Transceiver, systemId, passwd);
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
			string src, dest, report, ussd;
			is >> src;
			is >> dest;
			is >> report;
			is >> ussd;
			uint8_t registeredDelivery;
			bool ussdFlag;
			//report
			if (report == "" || report == "none")
			{
				registeredDelivery = NO_SMSC_DELIVERY_RECEIPT;
			}
			else if (report == "full")
			{
				registeredDelivery = FINAL_SMSC_DELIVERY_RECEIPT;
			}
			else if (report == "failure")
			{
				registeredDelivery = FAILURE_SMSC_DELIVERY_RECEIPT;
			}
			else
			{
				cout << "invalid report option" << endl;
				continue;
			}
			//ussd
			if (ussd == "" || ussd == "no")
			{
				ussdFlag = false;
			}
			else if (ussd == "yes")
			{
				ussdFlag = true;
			}
			else
			{
				cout << "invalid ussd flag" << endl;
				continue;
			}
			test.submit(src, dest, registeredDelivery, ussdFlag);
			continue;
		}
		else if (cmd == "set")
		{
			is >> cmd;
			if (cmd == "resp")
			{
				uint32_t commandStatus = 0x0;
				is >> commandStatus;
				test.setDeliveryResp(commandStatus);
				continue;
			}
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

