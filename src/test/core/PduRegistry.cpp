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

void PduRegistry::registerMonitor(uint32_t seqNum, PduMonitor* monitor)
{
	__require__(monitor);
	SeqNumKey key(seqNum, monitor->getType());
	__require__(seqNumMap.find(key) == seqNumMap.end());
	seqNumMap[key] = monitor;
}

void PduRegistry::registerMonitor(uint16_t msgRef, PduMonitor* monitor)
{
	__require__(monitor);
	//можно зарегистрировать единственный монитор для msgRef
	__require__(msgRefMap.lower_bound(MsgRefKey(msgRef, monitor->getType(), 0)) ==
		msgRefMap.upper_bound(MsgRefKey(msgRef, monitor->getType(), UINT_MAX)));
	MsgRefKey key(msgRef, monitor->getType(), monitor->getId());
	msgRefMap[key] = monitor;
}

void PduRegistry::registerMonitor(time_t t, PduMonitor* monitor)
{
	__require__(monitor);
	TimeKey timeKey(t, monitor->getId());
	TimeMap::const_iterator it = checkTimeMap.find(timeKey);
	__require__(it == checkTimeMap.end());
	checkTimeMap[timeKey] = monitor;
}

#define __register_monitor_by_seq_num__(MonitorClass) \
	{ MonitorClass* m = dynamic_cast<MonitorClass*>(monitor); \
	registerMonitor(m->sequenceNumber, m); }

#define __register_monitor_by_msg_ref__(MonitorClass) \
	{ MonitorClass* m = dynamic_cast<MonitorClass*>(monitor); \
	registerMonitor(m->msgRef, m); }

void PduRegistry::registerMonitor(PduMonitor* monitor)
{
	__require__(monitor);
	__require__(!monitor->isRegistered());
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
			__register_monitor_by_seq_num__(ResponseMonitor);
			break;
		case GENERIC_NACK_MONITOR:
			__register_monitor_by_seq_num__(GenericNackMonitor);
			break;
		case DELIVERY_MONITOR:
			__register_monitor_by_msg_ref__(DeliveryMonitor);
			break;
		case DELIVERY_RECEIPT_MONITOR:
			__register_monitor_by_msg_ref__(DeliveryReceiptMonitor);
			break;
		case INTERMEDIATE_NOTIFICATION_MONITOR:
			__register_monitor_by_msg_ref__(IntermediateNotificationMonitor);
			break;
		case SMS_CANCELLED_NOTIFICATION_MONITOR:
			__register_monitor_by_msg_ref__(SmsCancelledNotificationMonitor);
			break;
		case SME_ACK_MONITOR:
			__register_monitor_by_msg_ref__(SmeAckMonitor);
			break;
		default:
			__unreachable__("Invalid monitor type");
	}
	registerMonitor(monitor->getCheckTime(), monitor);
	monitor->setRegistered(true);
	__trace2__("monitor registered: pduReg = %p, monitor = %s",
		this, monitor->str().c_str());
}

void PduRegistry::clear()
{
	__trace2__("PduRegistry::clear(): pduReg = %x", this);
	for (TimeMap::iterator it = checkTimeMap.begin(); it != checkTimeMap.end(); it++)
	{
		PduMonitor* monitor = it->second;
		monitor->setRegistered(false);
		delete monitor;
	}
	seqNumMap.clear();
	msgRefMap.clear();
	checkTimeMap.clear();
}

PduMonitor* PduRegistry::getMonitor(uint16_t msgRef, MonitorType type) const
{
	MsgRefMap::const_iterator it = msgRefMap.lower_bound(MsgRefKey(msgRef, type, 0));
	MsgRefMap::const_iterator end = msgRefMap.upper_bound(MsgRefKey(msgRef, type, UINT_MAX));
	PduMonitor* monitor = NULL;
	for (; it != end; it++)
	{
		//монитор для msgRef с flag != PDU_NOT_EXPECTED_FLAG будет последним
		__require__(!monitor || monitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
		monitor = it->second;
		__require__(monitor);
	}
	return monitor;
}

PduMonitor* PduRegistry::getMonitor(uint32_t seqNum, MonitorType type) const
{
	SeqNumMap::const_iterator it = seqNumMap.find(SeqNumKey(seqNum, type));
	return (it == seqNumMap.end() ? NULL : it->second);
}

#define __remove_monitor_by_seq_num__(MonitorClass) \
	{ MonitorClass* m = dynamic_cast<MonitorClass*>(monitor); \
	int res = seqNumMap.erase(SeqNumKey(m->sequenceNumber, m->getType())); \
	__require__(res); }

#define __remove_monitor_by_msg_ref__(MonitorClass) \
	{ MonitorClass* m = dynamic_cast<MonitorClass*>(monitor); \
	int res = msgRefMap.erase(MsgRefKey(m->msgRef, m->getType(), m->getId())); \
	__require__(res); }

void PduRegistry::removeMonitor(PduMonitor* monitor)
{
	__require__(monitor);
	__require__(monitor->isRegistered());
	switch (monitor->getType())
	{
		case RESPONSE_MONITOR:
			__remove_monitor_by_seq_num__(ResponseMonitor);
			break;
		case GENERIC_NACK_MONITOR:
			__remove_monitor_by_seq_num__(GenericNackMonitor);
			break;
		case DELIVERY_MONITOR:
			__remove_monitor_by_msg_ref__(DeliveryMonitor);
			break;
		case DELIVERY_RECEIPT_MONITOR:
			__remove_monitor_by_msg_ref__(DeliveryReceiptMonitor);
			break;
		case INTERMEDIATE_NOTIFICATION_MONITOR:
			__remove_monitor_by_msg_ref__(IntermediateNotificationMonitor);
			break;
		case SMS_CANCELLED_NOTIFICATION_MONITOR:
			__remove_monitor_by_msg_ref__(SmsCancelledNotificationMonitor);
			break;
		case SME_ACK_MONITOR:
			__remove_monitor_by_msg_ref__(SmeAckMonitor);
			break;
		default:
			__unreachable__("Invalid monitor type");

	}
	int res = checkTimeMap.erase(TimeKey(monitor->getCheckTime(), monitor->getId()));
	__require__(res);
	monitor->setRegistered(false);
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

