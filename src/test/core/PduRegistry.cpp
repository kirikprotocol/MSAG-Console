#include "PduRegistry.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

PduRegistry::~PduRegistry()
{
	__trace2__("~PduRegistry(): size = %d", seqNumMap.size());
	for (SeqNumMap::iterator it = seqNumMap.begin(); it != seqNumMap.end(); it++)
	{
		delete it->second->pdu;
		delete it->second;
	}
}

void PduRegistry::putPdu(PduData& data)
{
	__require__(data.pdu);
	PduData* pduData = new PduData(data);
	seqNumMap[pduData->pdu->get_sequenceNumber()] = pduData;
	if (pduData->smsId)
	{
		idMap[pduData->smsId] = pduData;
	}
	if (pduData->msgRef)
	{
		msgRefMap[pduData->msgRef] = pduData;
	}
	if (pduData->submitTime)
	{
		submitTimeMap[TimeKey(pduData->submitTime, pduData->pdu->get_sequenceNumber())] = pduData;
	}
	if (pduData->waitTime)
	{
		waitTimeMap[TimeKey(pduData->waitTime, pduData->pdu->get_sequenceNumber())] = pduData;
	}
	if (pduData->validTime)
	{
		validTimeMap[TimeKey(pduData->validTime, pduData->pdu->get_sequenceNumber())] = pduData;
	}
}

PduData* PduRegistry::getPdu(uint16_t msgRef)
{
	MsgRefMap::const_iterator it = msgRefMap.find(msgRef);
	return (it == msgRefMap.end() ? NULL : it->second);
}

PduData* PduRegistry::getPdu(uint32_t seqNumber)
{
	SeqNumMap::const_iterator it = seqNumMap.find(seqNumber);
	return (it == seqNumMap.end() ? NULL : it->second);
}

PduData* PduRegistry::getPdu(const SMSId smsId)
{
	SmsIdMap::const_iterator it = idMap.find(smsId);
	return (it != idMap.end() ? NULL : it->second);
}

bool PduRegistry::removePdu(PduData* pduData)
{
	__require__(pduData && pduData->pdu);
	seqNumMap.erase(pduData->pdu->get_sequenceNumber());
	if (pduData->smsId)
	{
		idMap.erase(pduData->smsId);
	}
	if (pduData->msgRef)
	{
		msgRefMap.erase(pduData->msgRef);
	}
	if (pduData->submitTime)
	{
		submitTimeMap.erase(TimeKey(pduData->submitTime, pduData->pdu->get_sequenceNumber()));
	}
	if (pduData->waitTime)
	{
		waitTimeMap.erase(TimeKey(pduData->waitTime, pduData->pdu->get_sequenceNumber()));
	}
	if (pduData->validTime)
	{
		validTimeMap.erase(TimeKey(pduData->validTime, pduData->pdu->get_sequenceNumber()));
	}
	delete pduData->pdu;
	delete pduData;
	return true;
}

PduRegistry::PduDataIterator* PduRegistry::getPduBySubmitTime(time_t t1, time_t t2)
{
	TimeMap::iterator it1 = submitTimeMap.lower_bound(TimeKey(t1, 0));
	TimeMap::iterator it2 = submitTimeMap.lower_bound(TimeKey(t2, ULONG_MAX));
	return new PduDataIterator(it1, it2);
}

PduRegistry::PduDataIterator* PduRegistry::getPduByWaitTime(time_t t1, time_t t2)
{
	TimeMap::iterator it1 = waitTimeMap.lower_bound(TimeKey(t1, 0));
	TimeMap::iterator it2 = waitTimeMap.lower_bound(TimeKey(t2, ULONG_MAX));
	return new PduDataIterator(it1, it2);
}

PduRegistry::PduDataIterator* PduRegistry::getPduByValidTime(time_t t1, time_t t2)
{
	TimeMap::iterator it1 = validTimeMap.lower_bound(TimeKey(t1, 0));
	TimeMap::iterator it2 = validTimeMap.lower_bound(TimeKey(t2, ULONG_MAX));
	return new PduDataIterator(it1, it2);
}

}
}
}

