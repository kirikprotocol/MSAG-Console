#ifndef TEST_SMPP_TEST_SMPP_SESSION
#define TEST_SMPP_TEST_SMPP_SESSION

#include "sme/SmppBase.hpp"

namespace smsc {
namespace test {
namespace smpp {

using smsc::core::network::Socket;
using smsc::core::synchronization::Mutex;
using smsc::sme::SmeConfig;
using smsc::sme::SmppPduEventListener;
using smsc::sme::SmppReader;
using smsc::smpp::SmppHeader;

struct PduBuffer
{
	char* buf;
	int size;
};

class TestSmppSession
{
	Socket socket;
	SmppPduEventListener* listener;
	SmppReader reader;
	Mutex cntMutex;
	Mutex lockMutex;
	int seqCounter;
	bool closed;

public:
	TestSmppSession(SmppPduEventListener* lst)
	: listener(lst), reader(lst, &socket), seqCounter(0), closed(true) {}
	~TestSmppSession() { close(); }
	
	void connect(const char* host, int port, int timeout);
	void close();
	int getNextSeq();

	PduBuffer getBytes(SmppHeader* pdu);
	void sendBytes(PduBuffer& pb);
	void sendPdu(SmppHeader* pdu);
};

}
}
}

#endif /* TEST_SMPP_TEST_SMPP_SESSION */
