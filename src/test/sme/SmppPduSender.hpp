#ifndef TEST_SME_SMPP_PDU_SENDER
#define TEST_SME_SMPP_PDU_SENDER

#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"
#include <map>
#include <sys/timeb.h>

namespace smsc {
namespace test {
namespace sme {

using smsc::core::threads::ThreadedTask;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::Event;
using std::multimap;

struct PduTask
{
	virtual void sendPdu() = NULL;
};

bool operator< (timeb t1, timeb t2);
timeb operator+ (timeb t, int millitm);
int operator- (timeb t1, timeb t2);

class SmppPduSender : public ThreadedTask
{
	struct comp
	{
		bool operator() (timeb t1, timeb t2) { return t1 < t2; }
	};
	typedef multimap<timeb, PduTask*, comp> Tasks;
	Tasks tasks;
	Mutex mutex;
	Event evt;

public:
	~SmppPduSender();
	virtual int Execute();
	virtual const char* taskName() { return "SmppPduSender"; }
	void schedulePdu(PduTask* pduTask, int delay);
};

}
}
}

#endif /* TEST_SME_SMPP_PDU_SENDER */
