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
		PduData* pduData = it->second;
		disposePdu(pduData->pdu);
		delete pduData;
	}
}

void PduRegistry::putPdu(PduData& data)
{
	__require__(data.pdu && data.pdu->get_sequenceNumber());
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

void PduRegistry::updateSmsId(PduData* pduData)
{
	__require__(pduData);
	if (pduData->smsId)
	{
		idMap[pduData->smsId] = pduData;
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
	__trace2__("PduRegistry::removePdu(): sequenceNumber = %u",
		pduData->pdu->get_sequenceNumber());
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
	disposePdu(pduData->pdu);
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

int PduRegistry::size()
{
	return seqNumMap.size();
}

void PduRegistry::dump(FILE* log)
{
	using namespace smsc::smpp;
	using namespace smsc::smpp::SmppCommandSet;
	fprintf(log, "PduRegistry::dump()\n");
	for (SeqNumMap::iterator it = seqNumMap.begin(); it != seqNumMap.end(); it++)
	{
		PduData* data = it->second;
		static const char* fmt = "%Y-%m-%d %H:%M:%S";
		char submitTime[20];
		char waitTime[20];
		char validTime[20];
		strftime(submitTime, 20, fmt, localtime(&data->submitTime));
		strftime(waitTime, 20, fmt, localtime(&data->waitTime));
		strftime(validTime, 20, fmt, localtime(&data->validTime));
		fprintf(log, "pduData[%u] = {smsId = %u, seqNum = %u, msgRef = %u",
			it->first, (uint32_t) data->smsId,
			(uint32_t) data->pdu->get_sequenceNumber(), (uint32_t) data->msgRef);
		fprintf(log, ", submitTime = %s, waitTime = %s, validTime = %s",
			submitTime, waitTime, validTime);
		fprintf(log, ", responseFlag = %d, deliveryFlag = %d",
			(int) data->responseFlag, (int) data->deliveryFlag);
		fprintf(log, ", deliveryReceiptFlag = %d, intermediateNotificationFlag = %d",
			(int) data->deliveryReceiptFlag, (int) data->intermediateNotificationFlag);
		fprintf(log, ", pdu = 0x%x, replacePdu = 0x%x}\n", data->pdu, data->replacePdu);
		switch (data->pdu->get_commandId())
		{
			case SUBMIT_SM:
				(reinterpret_cast<PduSubmitSm*>(data->pdu))->dump(log, 1);
				break;
			default:
				data->pdu->dump(log, 1);
		}
	}
}

}
}
}

