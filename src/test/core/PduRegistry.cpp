#include "PduRegistry.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

void PduRegistry::putPdu(PduData& data)
{
	__require__(data.pdu);
	PduData* pduData = new PduData(data);
	idMap[pduData->smsId] = pduData;
	seqNumMap[pduData->pdu->get_sequenceNumber()] = pduData;
	msgRefMap[pduData->msgRef] = pduData;
	waitTimeMap.insert(WaitTimeMapPair(pduData->waitTime, pduData));
}

PduData* PduRegistry::getPdu(uint16_t msgRef)
{
	MsgRefMap::const_iterator it = msgRefMap.find(msgRef);
	if (it != msgRefMap.end())
	{
		return it->second;
	}
	return NULL;
}

PduData* PduRegistry::getPdu(uint32_t seqNumber)
{
	SeqNumMap::const_iterator it = seqNumMap.find(seqNumber);
	if (it != seqNumMap.end())
	{
		return it->second;
	}
	return NULL;
}

PduData* PduRegistry::getPdu(const SMSId smsId)
{
	SmsIdMap::const_iterator it = idMap.find(smsId);
	if (it != idMap.end())
	{
		return it->second;
	}
	return NULL;
}

bool PduRegistry::removePdu(const PduData& pduData)
{
	__require__(pduData.pdu);
	idMap.erase(pduData.smsId);
	seqNumMap.erase(pduData.pdu->get_sequenceNumber());
	msgRefMap.erase(pduData.msgRef);
	pair<WaitTimeMap::iterator, WaitTimeMap::iterator> p =
		waitTimeMap.equal_range(pduData.waitTime);
	for (; p.first != p.second; p.first++)
	{
		PduData* data = p.first->second;
		if (data->pdu->get_sequenceNumber() == 
			pduData.pdu->get_sequenceNumber() &&
			data->msgRef == pduData.msgRef)
		{
			waitTimeMap.erase(p.first);
			break;
		}
	}
	delete pduData.pdu;
	return true;
}

vector<PduData*> PduRegistry::getOverduePdu(time_t waitTime)
{
	vector<PduData*> res;
	WaitTimeMap::iterator it = waitTimeMap.lower_bound(waitTime);
	for (; it != waitTimeMap.end(); it--)
	{
		if (it->second->waitTime <= waitTime)
		{
			res.push_back(it->second);
		}
	}
	return res;
}

PduData* PduRegistry::getFirstPendingSubmitSmPdu(time_t waitTime)
{
    WaitTimeMap::iterator it = waitTimeMap.lower_bound(waitTime);
	return (it == waitTimeMap.end() ? NULL : it->second);
}

}
}
}

