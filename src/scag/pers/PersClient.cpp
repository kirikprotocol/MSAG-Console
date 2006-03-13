#include "PersClient.h"

namespace scag { namespace pers { namespace client {

using smsc::util::Exception;
using smsc::logger::Logger;
using scag::pers;

void PersClient::init(const char *_host, int _port, int _timeout)
{
	host = _host;
	port = _port;
	timeout = _timeout;
	if(sock.Init(_host, _port, _timeout) == -1)
		return;
	if(sock.Connect() == -1)
		return;
	connected = true;
}

void PersClient::SetPacketSize()
{
	uint32_t len = sb.GetPos();
	sb.SetPos(0);
	sb.Append((char*)&len, sizeof(uint32_t));
	sb.SetPos(len);
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
	t = strlen(key);
	sb.Append((char*)&t, 1);
	sb.Append(key, t);
}

void PersClient::SetProperty(ProfileType pt, const char* key, Property *prop)
{
	MutexGuard mt(mtx);

	FillStringHead(PC_SET, pt, key);

	prop->Serialize(sb);

	SetPacketSize();
}
void PersClient::SetProperty(ProfileType pt, uint32_t key, Property *prop)
{
	MutexGuard mt(mtx);

	FillIntHead(PC_SET, pt, key);

	prop->Serialize(sb);

	SetPacketSize();
}

Property* PersClient::GetProperty(ProfileType pt, const char* key, const char *property_name)
{
	MutexGuard mt(mtx);

	FillStringHead(PC_GET, pt, key);

	uint8_t t = strlen(property_name);
	sb.Append((char*)&t, 1);
	sb.Append(property_name, t);

	SetPacketSize();

	return NULL;
}
Property* PersClient::GetProperty(ProfileType pt, uint32_t key, const char *property_name)
{
	MutexGuard mt(mtx);

	FillIntHead(PC_GET, pt, key);

	uint8_t t = strlen(property_name);
	sb.Append((char*)&t, 1);
	sb.Append(property_name, t);

	SetPacketSize();

	return NULL;
}

void PersClient::DelProperty(ProfileType pt, const char* key, const char *property_name)
{
	MutexGuard mt(mtx);

	FillStringHead(PC_DEL, pt, key);

	uint8_t t = strlen(property_name);
	sb.Append((char*)&t, 1);
	sb.Append(property_name, t);

	SetPacketSize();
}
void PersClient::DelProperty(ProfileType pt, uint32_t key, const char *property_name)
{
	MutexGuard mt(mtx);

	FillIntHead(PC_DEL, pt, key);

	uint8_t t = strlen(property_name);
	sb.Append((char*)&t, 1);
	sb.Append(property_name, t);

	SetPacketSize();
}

}}}
