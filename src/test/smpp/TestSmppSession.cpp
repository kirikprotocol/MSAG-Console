#include "TestSmppSession.hpp"

namespace smsc {
namespace test {
namespace smpp {

using namespace smsc::sme;
using namespace smsc::smpp;

//not synchronized
void TestSmppSession::connect(const char* host, int port, int timeout)
{
	if (!closed) return;
	if (socket.Init(host, port, timeout) == -1)
		throw SmppConnectException(SmppConnectException::Reason::networkResolve);
	if (socket.Connect() == -1)
		throw SmppConnectException(SmppConnectException::Reason::networkConnect);
	reader.Start();
	closed = false;
}

//not synchronized
void TestSmppSession::close()
{
    if (closed) return;
    reader.Stop();
    socket.Close();
    reader.WaitFor();
    closed = true;
}

int TestSmppSession::getNextSeq()
{
	MutexGuard g(cntMutex);
	return ++seqCounter;
}

PduBuffer TestSmppSession::getBytes(SmppHeader* pdu)
{
	__require__(pdu);
	int sz = calcSmppPacketLength(pdu);
	PduBuffer pb;
	pb.buf = new char[sz];
	pb.size = sz;
	SmppStream s;
	assignStreamWith(&s, pb.buf, sz, false);
	if (!fillSmppPdu(&s, pdu)) throw Exception("Failed to fill smpp packet");
	return pb;
}

void TestSmppSession::sendBytes(PduBuffer& pb)
{
	__require__(!closed);
	__require__(pb.buf && pb.size);
	int count = 0;
	__trace2__("writer:sending buffer %p, %d", pb.buf, pb.size);
	MutexGuard mguard(lockMutex);
	do
	{
		int wr = socket.Write(pb.buf + count, pb.size - count);
		if (wr <= 0)
		{
			__trace2__("socket write error: wr = %d", wr);
			throw Exception("Failed to send smpp packet");
		}
		count += wr;
	}
	while (count != pb.size);
	delete pb.buf;
}

void TestSmppSession::sendPdu(SmppHeader* pdu)
{
	__require__(!closed);
	PduBuffer pb = getBytes(pdu);
	sendBytes(pb);
}

}
}
}

