#include "sme/SmppBase.hpp"
#include "util/regexp/RegExp.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "readline/readline.h"
#include "readline/history.h"
#include <map>
#include <iostream>

using smsc::sms::Address;
using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;
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
	bool forceDc;
	uint32_t respCommandStatus;
	int respDelay;

public:
	SimpleSmppListener(const string& _systemId) : systemId(_systemId),
		forceDc(false), respCommandStatus(0x0), respDelay(0) {}
	void setSession(SmppSession* _session) { session = _session; }
	bool getForceDc() { return forceDc; }
	void setForceDc(bool _forceDc) { forceDc = _forceDc; }
	void setDeliveryResp(uint32_t commandStatus, int delay) { respCommandStatus = commandStatus; respDelay = delay; }
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
	void bind(int mode, const string& systemId, const string& passwd, bool forceDc);
	void select(const string& systemId);
	const string& getSelected() { return selectedId; }
	void list();
	void unbind();
	void submit(const string& src, const string& dest,
		uint8_t registeredDelivery, bool ussd);
	void setDeliveryResp(uint32_t commandStatus, int delay);
};

#define __check_exception__ \
	catch (Exception& e) { cout << "exception: " << e.what() << endl; } \
	catch (...) { cout << "unknown exception" << endl; }

void SimpleSmppTest::bind(int mode, const string& systemId,
	const string& passwd, bool forceDc)
{
	cout << "bind: host = " << host << ", port = " << port <<
		", systemId = " << systemId << ", password = " << passwd <<
		", mode = " << mode << ", forceDc = " << (forceDc ? "yes" : "no") << endl;
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
		data->listener->setForceDc(forceDc);
		data->session->connect(mode);
		sessions[systemId] = data;
		cout << "bind successful" << endl;
		select(systemId);
	}
	catch (SmppConnectException& e)
	{
		cout << "bind failed: " << e.what() << endl;
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
		bool forceDc = data->listener->getForceDc();
		PduSubmitSm pdu;
		SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, true, forceDc,
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
			", seqNum = " << dec << pdu.get_header().get_sequenceNumber();
		if (respPdu)
		{
			cout << ", commandStatus = 0x" << hex << respPdu->get_header().get_commandStatus();
		}
		else
		{
			cout << ", response missing";
		}
		cout << ", msgRef = " << dec << (int) pdu.get_optional().get_userMessageReference() <<
			", ussd = " << (ussd ? "yes" : "no") <<
			", dc = " << (int) dc;
		if (forceDc)
		{
			bool simMsg;
			SmppUtil::extractDataCoding(dc, dc, simMsg);
		}
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

void SimpleSmppTest::setDeliveryResp(uint32_t commandStatus, int delay)
{
	__check_selected__;
	data->listener->setDeliveryResp(commandStatus, delay);
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
				if (forceDc)
				{
					bool simMsg;
					SmppUtil::extractDataCoding(dc, dc, simMsg);
				}
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
				sleep(respDelay);
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

bool isCmd(const string& cmd, const string& name)
{
	RegExp regExp;
	char regExpStr[64];
	sprintf(regExpStr, "/^%s(\\s+|$)/", name.c_str());
	int regExpOk = regExp.Compile(regExpStr);
	__require__(regExpOk);
	int matchCount = 5;
	SMatch match[matchCount];
	return regExp.Match(cmd.c_str(), match, matchCount);
}

const string getParam(const string& cmd, const string& name)
{
	RegExp regExp;
	char regExpStr[64];
	sprintf(regExpStr, "/.*%s\\s*=\\s*(\\w+)/", name.c_str());
	int regExpOk = regExp.Compile(regExpStr);
	__require__(regExpOk);
	int matchCount = 5;
	SMatch match[matchCount];
	if (regExp.Match(cmd.c_str(), match, matchCount))
	{
		__require__(matchCount > 1);
		int len = match[1].end - match[1].start;
		__require__(len >= 0);
		char tmp[len + 1];
		memcpy(tmp, cmd.c_str() + match[1].start, len);
		tmp[len] = 0;
		return tmp;
	}
	return "";
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
			cout << "bind <id=smeId> [passwd=password] [mode=trx|rx|tx|] [forceDc=no|yes]- bind sme" << endl;
			cout << "select <id=smeId> - select sme" << endl;
			cout << "list - list sessions" << endl;
			cout << "unbind - unbind selected sme" << endl;
			cout << "submit <src=srcAddr> <dest=destAddr> [report=none|full|failure] [ussd=no|yes] - submit sms" << endl;
			cout << "set resp [status=0] [delay=0] - set delivery_sm_resp command status and delay" << endl;
			cout << "quit - quit test" << endl;
			continue;
		}
		string prompt = test.getSelected() + ">";
		const char* cmd = readline(prompt.c_str());
		if (!cmd)
		{
			return;
		}
		add_history(cmd);
		if (isCmd(cmd, "bind"))
		{
			string systemId = getParam(cmd, "id");
			string passwd = getParam(cmd, "passwd");
			string strMode = getParam(cmd, "mode");
			string strForceDc = getParam(cmd, "forceDc");
			int mode = BindType::Transceiver;
			if (strMode == "rx")
			{
				mode = BindType::Receiver;
			}
			else if (strMode == "tx")
			{
				mode = BindType::Transmitter;
			}
			bool forceDc = false;
			if (strForceDc == "yes")
			{
				forceDc = true;
			}
			if (systemId.length())
			{
				test.bind(mode, systemId, passwd, forceDc);
				continue;
			}
		}
		else if (isCmd(cmd, "select"))
		{
			string systemId = getParam(cmd, "id");
			if (systemId.length())
			{
				test.select(systemId);
				continue;
			}
		}
		else if (isCmd(cmd, "list"))
		{
			test.list();
			continue;
		}
		else if (isCmd(cmd, "unbind"))
		{
			test.unbind();
			continue;
		}
		else if (isCmd(cmd, "submit"))
		{
			string src = getParam(cmd, "src");
			string dest = getParam(cmd, "dest");
			string report = getParam(cmd, "report");
			string ussd = getParam(cmd, "ussd");
			//report
			uint8_t registeredDelivery;
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
			bool ussdFlag;
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
		else if (isCmd(cmd, "set resp"))
		{
			uint32_t commandStatus = atoi(getParam(cmd, "status").c_str());
			int delay = atoi(getParam(cmd, "delay").c_str());
			test.setDeliveryResp(commandStatus, delay);
			continue;
		}
		else if (isCmd(cmd, "quit"))
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
	RegExp::InitLocale();
	executeTest(smscHost, smscPort);
}

