#include "SmppPduSender.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::core::synchronization::MutexGuard;

bool operator< (timeb t1, timeb t2)
{
	return (t1.time < t2.time || (t1.time == t2.time && t1.millitm < t2.millitm));
}

timeb operator+ (timeb t, int millitm)
{
	__require__(millitm > 0);
	millitm += t.millitm;
	t.time += millitm / 1000;
	t.millitm = millitm % 1000;
	return t;
}

int operator- (timeb t1, timeb t2)
{
	return (1000 * (t1.time - t2.time) + (t1.millitm - t2.millitm));
}

SmppPduSender::~SmppPduSender()
{
	MutexGuard mguard(mutex);
	for (Tasks::iterator it = tasks.begin(); it != tasks.end(); it++)
	{
		PduTask* task = it->second;
		__require__(task);
		delete task;
	}
	tasks.clear();
}

int SmppPduSender::Execute()
{
	while (!isStopping)
	{
		timeb t;
		ftime(&t);
		Tasks::iterator it = tasks.begin();
		while (it != tasks.end() && it->first < t)
		{
			it->second->sendPdu();
			delete (it)->second;
			tasks.erase(it++);
		}
		int timeout = 1000;
		if (it != tasks.end() && it->first < t + timeout)
		{
			timeout = it->first - t;
			__require__(timeout >= 0);
		}
		evt.Wait(timeout);
	}
	return 0;
}

void SmppPduSender::schedulePdu(PduTask* pduTask, int delay)
{
	__require__(pduTask);
	MutexGuard mguard(mutex);
	timeb t;
    ftime(&t);
	t = t + delay;
	tasks.insert(Tasks::value_type(t, pduTask));
}

}
}
}

