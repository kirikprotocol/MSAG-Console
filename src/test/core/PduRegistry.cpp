#include "PduRegistry.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

PduRegistry::~PduRegistry()
{
	__trace2__("~PduRegistry(): size = %d", msgRefMap.size());
	clear();
}

void PduRegistry::registerPdu(PduData* pduData)
{
	__require__(pduData && pduData->pdu);
	__trace2__("PduRegistry::registerPdu(): pduReg = 0x%x, msgRef = %d, waitTime = %d, validTime = %d, seqNum = %u",
		this, pduData->msgRef, pduData->waitTime, pduData->validTime, pduData->pdu->get_sequenceNumber());
	MsgRefMap::const_iterator it = msgRefMap.find(msgRef);
	__require__(msgRef && it == msgRefMap.end()); //registerPdu() первый раз для данной pdu
	if (pduData->smsId)
	{
		idMap[pduData->smsId] = pduData;
	}
	if (pduData->pdu->get_sequenceNumber())
	{
		seqNumMap[pduData->pdu->get_sequenceNumber()] = pduData;
	}
	if (pduData->msgRef)
	{
		msgRefMap[pduData->msgRef] = pduData;
	}
	if (pduData->submitTime)
	{
		submitTimeMap[TimeKey(pduData->submitTime, pduData->msgRef)] = pduData;
	}
	if (pduData->waitTime)
	{
		waitTimeMap[TimeKey(pduData->waitTime, pduData->msgRef)] = pduData;
	}
	if (pduData->validTime)
	{
		validTimeMap[TimeKey(pduData->validTime, pduData->msgRef)] = pduData;
	}
}

void PduRegistry::updatePdu(PduData* pduData)
{
	__require__(pduData && pduData->pdu);
	__trace2__("PduRegistry::updatePdu(): pduReg = 0x%x, msgRef = %d, seqNum = %u, smsId = %u, pdu = 0x%x, replacePdu = 0x%x, replacedByPdu = 0x%x",
		this, pduData->msgRef, pduData->pdu->get_sequenceNumber(), (uint32_t) pduData->smsId, pduData->pdu, pduData->replacePdu, pduData->replacedByPdu);
	if (pduData->smsId)
	{
		idMap[pduData->smsId] = pduData;
	}
	if (pduData->pdu->get_sequenceNumber())
	{
		seqNumMap[pduData->pdu->get_sequenceNumber()] = pduData;
	}
}

void PduRegistry::clear()
{
	__trace2__("PduRegistry::clear(): pduReg = 0x%x", this);
	if (lastRemovedPduData)
	{
		disposePdu(lastRemovedPduData->pdu);
		delete lastRemovedPduData;
		lastRemovedPduData = NULL;
	}
	for (MsgRefMap::iterator it = msgRefMap.begin(); it != msgRefMap.end(); it++)
	{
		PduData* pduData = it->second;
		disposePdu(pduData->pdu);
		delete pduData;
	}
	idMap.clear();
	seqNumMap.clear();
	msgRefMap.clear();
	submitTimeMap.clear();
	waitTimeMap.clear();
	validTimeMap.clear();
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

void PduRegistry::removePdu(PduData* pduData)
{
	__require__(pduData && pduData->pdu);
	__trace2__("PduRegistry::removePdu(): pduReg = 0x%x, msgRef = %d, seqNum = %u, smsId = %u, pdu = 0x%x, replacePdu = 0x%x, replacedByPdu = 0x%x",
		this, pduData->msgRef, pduData->pdu->get_sequenceNumber(), (uint32_t) pduData->smsId, pduData->pdu, pduData->replacePdu, pduData->replacedByPdu);
	//разорвать связь с замещающей и замещаемой pdu
	if (pduData->replacePdu)
	{
		pduData->replacePdu->replacedByPdu = NULL;
	}
	if (pduData->replacedByPdu)
	{
		pduData->replacedByPdu->replacePdu = NULL;
	}
	if (pduData->smsId)
	{
		int res = idMap.erase(pduData->smsId);
		__require__(res);
	}
	if (pduData->msgRef)
	{
		int res = msgRefMap.erase(pduData->msgRef);
		__require__(res);
	}
	if (pduData->pdu->get_sequenceNumber())
	{
		int res = seqNumMap.erase(pduData->pdu->get_sequenceNumber());
		__require__(res);
	}
	if (pduData->submitTime)
	{
		int res = submitTimeMap.erase(TimeKey(pduData->submitTime, pduData->msgRef));
		__require__(res);
	}
	if (pduData->waitTime)
	{
		int res = waitTimeMap.erase(TimeKey(pduData->waitTime, pduData->msgRef));
		__require__(res);
	}
	if (pduData->validTime)
	{
		int res = validTimeMap.erase(TimeKey(pduData->validTime, pduData->msgRef));
		__require__(res);
	}
	if (lastRemovedPduData)
	{
		disposePdu(lastRemovedPduData->pdu);
		delete lastRemovedPduData;
	}
	lastRemovedPduData = pduData;
}

PduData* PduRegistry::getLastRemovedPdu()
{
	return lastRemovedPduData;
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

int PduRegistry::size() const
{
	return seqNumMap.size();
}

void PduRegistry::dump(FILE* log) const
{
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
		strftime(submitTime, 20, fmt, localtime(&data->submitTime));
		strftime(waitTime, 20, fmt, localtime(&data->waitTime));
		strftime(validTime, 20, fmt, localtime(&data->validTime));
		fprintf(log, "pduData[%u] = {smsId = %u, seqNum = %u, msgRef = %u",
			it->first, (uint32_t) data->smsId,
			(uint32_t) data->pdu->get_sequenceNumber(), (uint32_t) data->msgRef);
		fprintf(log, ", submitTime = %s, waitTime = %s, validTime = %s",
			submitTime, waitTime, validTime);
		fprintf(log, ", responseFlag = %d, deliveryFlag = %d",
			data->responseFlag, (int) data->deliveryFlag);
		fprintf(log, ", deliveryReceiptFlag = %d, intermediateNotificationFlag = %d",
			(int) data->deliveryReceiptFlag, data->intermediateNotificationFlag);
		fprintf(log, ", pdu = 0x%x, replacePdu = 0x%x, replacedByPdu = 0x%x}\n",
			data->pdu, data->replacePdu, data->replacedByPdu);
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

