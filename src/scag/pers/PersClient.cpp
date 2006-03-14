/* $Id$ */

#include "PersClient.h"

namespace scag { namespace pers { namespace client {

using smsc::util::Exception;
using smsc::logger::Logger;
using scag::pers;

void PersClient::SetProperty(ProfileType pt, const char* key, Property& prop) throw(PersClientException)
{
	if(pt != PT_ABONENT)
		throw PersClientException(INVALID_KEY);

	MutexGuard mt(mtx);

	FillStringHead(PC_SET, pt, key);
	prop.Serialize(sb);
	SetPacketSize();
	SendPacket();
	ReadPacket();
	if(GetServerResponse() != RESPONSE_OK)
		throw PersClientException(SERVER_ERROR);
}
void PersClient::SetProperty(ProfileType pt, uint32_t key, Property& prop) throw(PersClientException)
{
	if(pt == PT_ABONENT)
		throw PersClientException(INVALID_KEY);

	MutexGuard mt(mtx);

	FillIntHead(PC_SET, pt, key);
	prop.Serialize(sb);
	SetPacketSize();
	SendPacket();
	ReadPacket();
	if(GetServerResponse() != RESPONSE_OK)
		throw PersClientException(SERVER_ERROR);
}

void PersClient::GetProperty(ProfileType pt, const char* key, const char *property_name, Property& prop) throw(PersClientException)
{
	if(pt != PT_ABONENT)
		throw PersClientException(INVALID_KEY);


	MutexGuard mt(mtx);

	FillStringHead(PC_GET, pt, key);
	AppendString(property_name);
	SetPacketSize();
	SendPacket();
	ReadPacket();
	ParseProperty(prop);
}
void PersClient::GetProperty(ProfileType pt, uint32_t key, const char *property_name, Property& prop) throw(PersClientException)
{
	if(pt == PT_ABONENT)
		throw PersClientException(INVALID_KEY);

	MutexGuard mt(mtx);

	FillIntHead(PC_GET, pt, key);
	AppendString(property_name);
	SetPacketSize();
	SendPacket();
	ReadPacket();
	ParseProperty(prop);
}

void PersClient::DelProperty(ProfileType pt, const char* key, const char *property_name) throw(PersClientException)
{
	if(pt != PT_ABONENT)
		throw PersClientException(INVALID_KEY);

	MutexGuard mt(mtx);

	FillStringHead(PC_DEL, pt, key);
	AppendString(property_name);
	SetPacketSize();
	SendPacket();
	ReadPacket();
	if(GetServerResponse() == RESPONSE_ERROR)
		throw PersClientException(SERVER_ERROR);
}
void PersClient::DelProperty(ProfileType pt, uint32_t key, const char *property_name) throw(PersClientException)
{
	if(pt == PT_ABONENT)
		throw PersClientException(INVALID_KEY);

	MutexGuard mt(mtx);

	FillIntHead(PC_DEL, pt, key);
	AppendString(property_name);
	SetPacketSize();
	SendPacket();
	ReadPacket();
	if(GetServerResponse() == RESPONSE_ERROR)
		throw PersClientException(SERVER_ERROR);
}

void PersClient::init(const char *_host, int _port, int _timeout) throw(PersClientException)
{
	host = _host;
	port = _port;
	timeout = _timeout;
	init();
}


void PersClient::init()
{
	char resp[3];
	if(connected)
		return;
	if(sock.Init(host.c_str(), port, timeout) == -1)
		throw PersClientException(CANT_CONNECT);
	if(sock.Connect() == -1)
		throw PersClientException(CANT_CONNECT);
	if(sock.Read(resp, 2) != 2)
	{
		sock.Close();
		throw PersClientException(CANT_CONNECT);
	}
	resp[2] = 0;
	if(!strcmp(resp, "SB"))
	{
		sock.Close();
		throw PersClientException(SERVER_BUSY);
	} else if(strcmp(resp, "OK"))
	{
		sock.Close();
		throw PersClientException(UNKNOWN_RESPONSE);
	}
	connected = true;
}

void PersClient::ReadAllTO(char* buf, uint32_t sz)
{
	uint32_t cnt, rd = 0;

	while(sz)
	{
		if(sock.canRead(0) <= 0)
			throw PersClientException(TIMEOUT);

		cnt = sock.Read(buf + rd, sz);
		if(cnt <= 0)
			throw PersClientException(READ_FAILED);

		rd += cnt;
		sz -= cnt;
	}
}

void PersClient::WriteAllTO(char* buf, uint32_t sz)
{
	uint32_t cnt, wr = 0;

	while(sz)
	{
		if(sock.canWrite(0) <= 0)
			throw PersClientException(TIMEOUT);

		cnt = sock.Write(buf + wr, sz);
		if(cnt <= 0)
			throw PersClientException(SEND_FAILED);

		wr += cnt;
		sz -= cnt;
	}
}

void PersClient::SendPacket()
{
	init();
	if(sock.WriteAll(sb.get(), sb.GetSize()) == -1)
	{
		connected = false;
		sock.Close();
		throw PersClientException(SEND_FAILED);
	}
}

void PersClient::ReadPacket()
{
	char tmp_buf[1024];
	uint32_t sz;

	if(!connected)
		throw PersClientException(NOT_CONNECTED);

	sb.Empty();
	ReadAllTO(tmp_buf, sizeof(uint32_t));
	sb.Append(tmp_buf, sizeof(uint32_t));
	sb.SetPos(0);
	sb.Read((char*)&sz, sizeof(uint32_t));
	sb.SetPos(sizeof(uint32_t));

	while(sz > 1024)
	{
		ReadAllTO(tmp_buf, 1024);
		sb.Append(tmp_buf, 1024);
	}
	if(sz)
	{
		ReadAllTO(tmp_buf, sz);
		sb.Append(tmp_buf, sz);
	}
}

void PersClient::SetPacketSize()
{
	uint32_t len = sb.GetPos();
	sb.SetPos(0);
	sb.Append((char*)&len, sizeof(len));
	sb.SetPos(len);
}

uint32_t PersClient::GetPacketSize()
{
	uint32_t sz;
	sb.SetPos(0);
	sb.Read((char*)&sz, sizeof(sz));
	return sz;
}

PersServerResponseType PersClient::GetServerResponse()
{
	uint8_t t;
	if(GetPacketSize() < sizeof(uint32_t) + 1)
		throw PersClientException(BAD_RESPONSE);
	sb.SetPos(sizeof(uint32_t));
	sb.Read((char*)&t, sizeof(t));
	return (PersServerResponseType)t;
}

void PersClient::AppendString(const char *str)
{
	uint8_t t = strlen(str);
	sb.Append((char*)&t, 1);
	sb.Append(str, t);
}

void PersClient::FillIntHead(PersCmd pc, ProfileType pt, uint32_t key)
{
	uint8_t t;
	sb.SetPos(4);
	t = pc;
	sb.Append((char*)&t, 1);
	t = pt;
	sb.Append((char*)&t, 1);
	sb.Append((char*)&key, sizeof(key));
}

void PersClient::FillStringHead(PersCmd pc, ProfileType pt, const char *key)
{
	uint8_t t;
	sb.SetPos(4);
	t = pc;
	sb.Append((char*)&t, 1);
	t = pt;
	sb.Append((char*)&t, 1);
	AppendString(key);
}

void PersClient::ParseProperty(Property& prop)
{
	PersServerResponseType ss = GetServerResponse();
	if(ss == RESPONSE_PROPERTY_NOT_FOUND)
		throw PersClientException(PROPERTY_NOT_FOUND);

	if(ss != RESPONSE_OK)
		throw PersClientException(SERVER_ERROR);

	sb.SetPos(sizeof(uint32_t) + 1);
	try{
		prop.Deserialize(sb);	
	} catch(SerialBufferOutOfBounds &e)
	{
		throw PersClientException(BAD_RESPONSE);
	}
}

}}}
