#include "MessageRegistry.hpp"

namespace smsc {
namespace test {
namespace core {

using smsc::sms::AddressValue;

Mutex& MessageRegistry::getMutex(const Address& origAddr)
{
	AddressData& data = addrMap[origAddr];
	return data.mutex;
}

uint16_t MessageRegistry::nextMsgRef(const Address& origAddr)
{
	AddressData& data = addrMap[origAddr];
	return data.msgRef++;
}

void MessageRegistry::putMsg(const Address& origAddr, MsgData& msgData)
{
	AddressData& data = addrMap[origAddr];
	data.messages.push_back(msgData);
}

MsgData* MessageRegistry::getMsg(const Address& origAddr, uint16_t msgRef)
{
	AddressData& data = addrMap[origAddr];
	MsgList& messages = data.messages;
	for (MsgList::iterator it = messages.begin(); it != messages.end(); it++)
	{
		if (it->msgRef == msgRef)
	{
			return &(*it);
		}
	}
	return NULL;
}

MsgData* MessageRegistry::getMsg(const Address& origAddr, const SMSId smsId)
{
	AddressData& data = addrMap[origAddr];
	MsgList& messages = data.messages;
	for (MsgList::iterator it = messages.begin(); it != messages.end(); it++)
	{
		if (it->smsId == smsId)
		{
			return &(*it);
		}
	}
	return NULL;
}

bool MessageRegistry::removeMsg(const Address& origAddr, const SMSId smsId)
{
	AddressData& data = addrMap[origAddr];
	MsgList& messages = data.messages;
	for (MsgList::iterator it = messages.begin(); it != messages.end(); it++)
	{
		if (it->smsId == smsId)
		{
			messages.erase(it);
			return true;
		}
	}
	return false;
}

list<SMSId> MessageRegistry::deleteExpiredMsg(const Address& origAddr,
	time_t waitTime)
{
	list<SMSId> res;
	AddressData& data = addrMap[origAddr];
	MsgList& messages = data.messages;
	for (MsgList::iterator it = messages.begin(); it != messages.end(); it++)
	{
		if (it->waitTime < waitTime)
		{
			res.push_back(it->smsId);
			messages.erase(it--);
			continue;
		}
	}
	return res;
}

}
}
}

