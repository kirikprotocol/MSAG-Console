#include "PduRegistry.hpp"
#include "util/debug.h"
#include <sstream>

namespace smsc {
namespace test {
namespace core {

using namespace std;

PduRegistry::~PduRegistry()
{
	__trace2__("~PduRegistry(): size = %d", msgRefMap.size());
	clear();
}

const char* PduRegistry::toString(PduData* pduData) const
{
	ostringstream s;
	s << "pduReg = " << (void*) this <<
		", smsId = " << pduData->smsId <<
		", seqNum = " << pduData->pdu->get_sequenceNumber() <<
		", msgRef = " << pduData->msgRef <<
		", pdu = " << (void*) pduData->pdu <<
		", deliveryStatus = " << pduData->deliveryStatus <<
		", replacePdu = " << (void*) pduData->replacePdu <<
		", replacedByPdu = " << (void*) pduData->replacedByPdu <<
		", submitTime = " << pduData->submitTime <<
		", waitTime = " << pduData->waitTime <<
		", validTime = " << pduData->validTime <<
		", responseFlag = " << pduData->responseFlag <<
		", deliveryFlag = " << (int) pduData->deliveryFlag <<
		", deliveryReceiptFlag = " << (int) pduData->deliveryReceiptFlag <<
		", intermediateNotificationFlag = " << pduData->intermediateNotificationFlag;
	return s.str().c_str();
}

void PduRegistry::registerPdu(PduData* pduData)
{
	__require__(pduData && pduData->pdu);
	__trace2__("PduRegistry::registerPdu(): %s", toString(pduData));
	if (pduData->smsId.length())
	{
		idMap[pduData->smsId] = pduData;
	}
	if (pduData->pdu->get_sequenceNumber())
	{
		seqNumMap[pduData->pdu->get_sequenceNumber()] = pduData;
	}
	if (pduData->msgRef)
	{
		msgRefMap[MsgRefKey(pduData->msgRef, pduData->id)] = pduData;
	}
	if (pduData->submitTime)
	{
		submitTimeMap[TimeKey(pduData->submitTime, pduData->id)] = pduData;
	}
	if (pduData->waitTime)
	{
		waitTimeMap[TimeKey(pduData->waitTime, pduData->id)] = pduData;
	}
	if (pduData->validTime)
	{
		validTimeMap[TimeKey(pduData->validTime, pduData->id)] = pduData;
	}
}

void PduRegistry::updatePdu(PduData* pduData)
{
	__require__(pduData && pduData->pdu);
	__trace2__("PduRegistry::updatePdu(): %s", toString(pduData));
	//smsId и sequenceNumber становятся известны позже из submit_sm_resp
	//для того же самого smsId могут меняются pduData при replace_sm и при откате replace_sm
	if (pduData->smsId.length())
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

vector<PduData*> PduRegistry::getPdu(uint16_t msgRef) const
{
	vector<PduData*> res;
	MsgRefMap::const_iterator it = msgRefMap.lower_bound(MsgRefKey(msgRef, 0));
	MsgRefMap::const_iterator end = msgRefMap.upper_bound(MsgRefKey(msgRef, UINT_MAX));
	for (; it != end; it++)
	{
		res.push_back(it->second);
	}
	return res;
}

PduData* PduRegistry::getPdu(uint32_t seqNumber) const
{
	SeqNumMap::const_iterator it = seqNumMap.find(seqNumber);
	return (it == seqNumMap.end() ? NULL : it->second);
}

PduData* PduRegistry::getPdu(const string& smsId) const
{
	SmsIdMap::const_iterator it = idMap.find(smsId);
	return (it != idMap.end() ? NULL : it->second);
}

void PduRegistry::removePdu(PduData* pduData)
{
	__require__(pduData && pduData->pdu);
	__trace2__("PduRegistry::removePdu(): %s", toString(pduData));
	if (pduData->smsId.length())
	{
		int res = idMap.erase(pduData->smsId);
		__require__(res || pduData->replacedByPdu || pduData->deliveryStatus);
	}
	if (pduData->msgRef)
	{
		int res = msgRefMap.erase(MsgRefKey(pduData->msgRef, pduData->id));
		__require__(res);
	}
	if (pduData->pdu->get_sequenceNumber())
	{
		int res = seqNumMap.erase(pduData->pdu->get_sequenceNumber());
		__require__(res);
	}
	if (pduData->submitTime)
	{
		int res = submitTimeMap.erase(TimeKey(pduData->submitTime, pduData->id));
		__require__(res);
	}
	if (pduData->waitTime)
	{
		int res = waitTimeMap.erase(TimeKey(pduData->waitTime, pduData->id));
		__require__(res);
	}
	if (pduData->validTime)
	{
		int res = validTimeMap.erase(TimeKey(pduData->validTime, pduData->id));
		__require__(res);
	}
	//разорвать связь с замещающей и замещаемой pdu
	if (pduData->replacePdu)
	{
		pduData->replacePdu->replacedByPdu = NULL;
	}
	if (pduData->replacedByPdu)
	{
		pduData->replacedByPdu->replacePdu = NULL;
	}
	//удалить
	if (lastRemovedPduData)
	{
		disposePdu(lastRemovedPduData->pdu);
		delete lastRemovedPduData;
	}
	lastRemovedPduData = pduData;
}

PduData* PduRegistry::getLastRemovedPdu() const
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
}

}
}
}

