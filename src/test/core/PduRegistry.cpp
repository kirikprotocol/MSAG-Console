#include "PduRegistry.hpp"
#include "util/debug.h"
#include <sstream>

namespace smsc {
namespace test {
namespace core {

using namespace std;

PduRegistry::~PduRegistry()
{
	__trace2__("~PduRegistry(): size = %d", checkTimeMap.size());
	clear();
}

void PduRegistry::registerMonitor(PduMonitor* monitor)
{
	__require__(monitor);
	/*
	if (!monitor->getCheckTime())
	{
		__trace2__("monitor registration rejected: pduReg = %p, %s", this, monitor->str().c_str());
		return;
	}
	*/
	//регистрирую все мониторы, включая checkTime = 0
	switch (monitor->getType())
	{
		case RESPONSE_MONITOR:
			{
				ResponseMonitor* m = dynamic_cast<ResponseMonitor*>(monitor);
				__require__(m);
				SeqNumMap::const_iterator it = seqNumMap.find(m->sequenceNumber);
				__require__(it == seqNumMap.end());
				seqNumMap[m->sequenceNumber] = m;
			}
			break;
		case DELIVERY_MONITOR:
		case DELIVERY_RECEIPT_MONITOR:
		case INTERMEDIATE_NOTIFICATION_MONITOR:
		case SME_ACK_MONITOR:
			{
				MsgRefKey msgRefKey = MsgRefKey(monitor->pduData->msgRef,
					monitor->getId());
				MsgRefMap::const_iterator it = msgRefMap.find(msgRefKey);
				__require__(it == msgRefMap.end());
				msgRefMap[msgRefKey] = monitor;
			}
			break;
		default:
			__unreachable__("Invalid monitor type");
	}
	TimeKey timeKey(monitor->getCheckTime(), monitor->getId());
	TimeMap::const_iterator it = checkTimeMap.find(timeKey);
	__require__(it == checkTimeMap.end());
	checkTimeMap[timeKey] = monitor;
	__trace2__("monitor registered: pduReg = %p, monitor = %s",
		this, monitor->str().c_str());
}

void PduRegistry::clear()
{
	__trace2__("PduRegistry::clear(): pduReg = %x", this);
	for (TimeMap::iterator it = checkTimeMap.begin(); it != checkTimeMap.end(); it++)
	{
		delete it->second;
	}
	seqNumMap.clear();
	msgRefMap.clear();
	checkTimeMap.clear();
}

PduMonitor* PduRegistry::getMonitor(uint16_t msgRef, MonitorType type) const
{
	MsgRefMap::const_iterator it = msgRefMap.lower_bound(MsgRefKey(msgRef, 0));
	MsgRefMap::const_iterator end = msgRefMap.upper_bound(MsgRefKey(msgRef, UINT_MAX));
	PduMonitor* monitor = NULL;
	for (; it != end; it++)
	{
		PduMonitor* m = it->second;
		__require__(m);
		if (m->getType() == type)
		{
			__require__(!monitor); //едиственный монитор для msgRef
			monitor = m;
		}
	}
	return monitor;
}

ResponseMonitor* PduRegistry::getResponseMonitor(uint32_t seqNum) const
{
	SeqNumMap::const_iterator it = seqNumMap.find(seqNum);
	return (it == seqNumMap.end() ? NULL : it->second);
}

DeliveryMonitor* PduRegistry::getDeliveryMonitor(uint16_t msgRef) const
{
	PduMonitor* m = getMonitor(msgRef, DELIVERY_MONITOR);
	return (m ? dynamic_cast<DeliveryMonitor*>(m) : NULL);
}

DeliveryReceiptMonitor* PduRegistry::getDeliveryReceiptMonitor(uint16_t msgRef) const
{
	PduMonitor* m = getMonitor(msgRef, DELIVERY_RECEIPT_MONITOR);
	return (m ? dynamic_cast<DeliveryReceiptMonitor*>(m) : NULL);
}

SmeAckMonitor* PduRegistry::getSmeAckMonitor(uint16_t msgRef) const
{
	PduMonitor* m = getMonitor(msgRef, SME_ACK_MONITOR);
	return (m ? dynamic_cast<SmeAckMonitor*>(m) : NULL);
}

void PduRegistry::removeMonitor(PduMonitor* monitor)
{
	__require__(monitor);
	switch (monitor->getType())
	{
		case RESPONSE_MONITOR:
			{
				ResponseMonitor* m = dynamic_cast<ResponseMonitor*>(monitor);
				__require__(m);
				int res = seqNumMap.erase(m->sequenceNumber);
				__require__(res);
			}
			break;
		case DELIVERY_MONITOR:
		case DELIVERY_RECEIPT_MONITOR:
		case INTERMEDIATE_NOTIFICATION_MONITOR:
		case SME_ACK_MONITOR:
			{
				int res = msgRefMap.erase(MsgRefKey(monitor->pduData->msgRef,
					monitor->getId()));
				__require__(res);
			}
			break;
		default:
			__unreachable__("Invalid monitor type");

	}
	int res = checkTimeMap.erase(TimeKey(monitor->getCheckTime(), monitor->getId()));
	__require__(res);
	//delete monitor;
	__trace2__("monitor unregistered: pduReg = %p, monitor = %s",
		this, monitor->str().c_str());
}

PduRegistry::PduMonitorIterator* PduRegistry::getMonitors(time_t t1, time_t t2) const
{
	TimeMap::const_iterator it1 = checkTimeMap.lower_bound(TimeKey(t1, 0));
	TimeMap::const_iterator it2 = checkTimeMap.lower_bound(TimeKey(t2, ULONG_MAX));
	return new PduMonitorIterator(it1, it2);
}

int PduRegistry::size() const
{
	return checkTimeMap.size();
}

void PduRegistry::dump(FILE* log) const
{
	/*
	using namespace smsc::smpp;
	using namespace smsc::smpp::SmppCommandSet;
	fprintf(log, "PduRegistry::dump()\n");
	for (SeqNumMap::const_iterator it = seqNumMap.begin(); it != seqNumMap.end(); it++)
	{
		PduData* data = it->second;
		static const char* fmt = "%Y-%m-%d %H:%M:%S";
		char submitTime[20];
		char waitTime[20];
		char validTime[20];
		tm t;
		strftime(submitTime, 20, fmt, localtime_r(&data->submitTime, &t));
		strftime(waitTime, 20, fmt, localtime_r(&data->waitTime, &t));
		strftime(validTime, 20, fmt, localtime_r(&data->validTime, &t));
		fprintf(log, "pduData[%u] = {%s}", it->first, toString(data));
		switch (data->pdu->get_commandId())
		{
			case SUBMIT_SM:
				(reinterpret_cast<PduSubmitSm*>(data->pdu))->dump(log, 1);
				break;
			default:
				data->pdu->dump(log, 1);
		}
	}
	*/
}

}
}
}

