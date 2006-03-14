/* $Id$ */

#include "CmdDispatcher.h"

namespace scag { namespace pers {

using smsc::util::Exception;
using smsc::logger::Logger;

CommandDispatcher::CommandDispatcher(StringProfileStore *abonent, IntProfileStore *service, IntProfileStore *oper, IntProfileStore *provider):
		log(Logger::getInstance("cmd"))
{
	int_store[0].pt = PT_SERVICE;
	int_store[1].pt = PT_OPERATOR;
	int_store[2].pt = PT_PROVIDER;
	int_store[0].store = service;
	int_store[1].store = oper;
	int_store[2].store = provider;
}

void CommandDispatcher::SendResponse(SerialBuffer *sb, PersServerResponseType r)
{
	uint32_t sz = 5;
	sb->Empty();
	sb->Append((char*)&sz, sizeof(sz));
	uint8_t t = r;
	sb->Append((char*)&t, sizeof(t));	
}

void CommandDispatcher::SetPacketSize(SerialBuffer *sb)
{
	uint32_t sz = sb->GetSize();
	sb->SetPos(0);
	sb->Append((char*)&sz, sizeof(sz));
}

void CommandDispatcher::DelCmdHandler(ProfileType pt, uint32_t int_key, string& str_key, string& name, SerialBuffer *sb)
{
	if(pt == PT_ABONENT)
		AbonentStore->delProperty(StringProfileKey(str_key.c_str()), name.c_str());
	else
		for(int i = 0; i < INT_STORE_CNT; i++)
			if(pt == int_store[i].pt)
				int_store[i].store->delProperty(IntProfileKey(int_key), name.c_str());
	SendResponse(sb, RESPONSE_OK);
}

void CommandDispatcher::GetCmdHandler(ProfileType pt, uint32_t int_key, string& str_key, string& name, SerialBuffer *sb)
{
	Property prop;
	bool exists = false;
	if(pt == PT_ABONENT)
		exists = AbonentStore->getProperty(StringProfileKey(str_key.c_str()), name.c_str(), prop);
	else
		for(int i = 0; i < INT_STORE_CNT; i++)
			if(pt == int_store[i].pt)
				exists = int_store[i].store->getProperty(IntProfileKey(int_key), name.c_str(), prop);
	if(exists)
	{	
		SendResponse(sb, RESPONSE_OK);
		prop.Serialize(*sb);
		SetPacketSize(sb);
	} else
		SendResponse(sb, RESPONSE_PROPERTY_NOT_FOUND);
}

void CommandDispatcher::SetCmdHandler(ProfileType pt, uint32_t int_key, string& str_key, Property& prop, SerialBuffer *sb)
{
	if(pt == PT_ABONENT)
		AbonentStore->setProperty(StringProfileKey(str_key.c_str()), prop);
	else
		for(int i = 0; i < INT_STORE_CNT; i++)
			if(pt == int_store[i].pt)
				int_store[i].store->setProperty(IntProfileKey(int_key), prop);
	SendResponse(sb, RESPONSE_OK);
}

void CommandDispatcher::Execute(SerialBuffer* sb)
{
	PersCmd cmd;
	ProfileType pt;
	uint8_t t;
	uint32_t sz;
	uint32_t int_key;
	string str_key;
	string name;
	Property prop;

	try{
		sb->SetPos(0);
		sb->Read((char*)&sz, sizeof(sz));
		sb->Read((char*)&t, sizeof(t));
		cmd = (PersCmd)t;
		sb->Read((char*)t, sizeof(t));		
		pt = (ProfileType)t;
		if(pt != PT_ABONENT)
			sb->Read((char*)&int_key, sizeof(int_key));
		else
			sb->ReadString(str_key);
		switch(cmd)
		{
			case PC_DEL:
				DelCmdHandler(pt, int_key, str_key, name, sb);
				return;
			case PC_SET:
				prop.Deserialize(*sb);
				SetCmdHandler(pt, int_key, str_key, prop, sb);
				return;
			case PC_GET:
				GetCmdHandler(pt, int_key, str_key, name, sb);
				return;
		}
	}
	catch(SerialBufferOutOfBounds &e)
	{
	}
	SendResponse(sb, RESPONSE_BAD_REQUEST);
}

}}
