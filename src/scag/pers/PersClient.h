/* $Id$ */

#ifndef SCAG_PERS_CLIENT_H
#define SCAG_PERS_CLIENT_H

#include "logger/Logger.h"
#include "SerialBuffer.h"
#include "Property.h"
#include "core/network/Socket.hpp"
#include "Types.h"

namespace scag { namespace pers { namespace client {

using smsc::core::network::Socket;

using smsc::logger::Logger;
using scag::pers;

enum PersClientExceptionType{
	CANT_CONNECT,
	SERVER_BUSY,
	UNKNOWN_RESPONSE,
	SEND_FAILED,
	READ_FAILED,
	TIMEOUT,
	NOT_CONNECTED,
	BAD_RESPONSE,
	SERVER_ERROR,
	PROPERTY_NOT_FOUND,
	INVALID_KEY
};

	static const char* strs[] = {
		"Cant connect to persserver",
		"Server busy",
		"Unknown server response",
		"Send failed",
		"Read failed",
		"Read/write timeout",
		"Not connected",
		"Bad response",
		"Server error",
		"Property not found",
		"Invalid key"
	};

class PersClientException{
protected:
	PersClientExceptionType et;
public:
	PersClientException(PersClientExceptionType e) { et = e; };
	PersClientExceptionType getType() { return et; };
	const char* what() const { return strs[et]; };
};

class PersClient {
public:
    PersClient(): connected(false) {};
    ~PersClient() { if(connected) sock.Close(); };

	static PersClient& Instance();
	static void Init(const char *_host, int _port, int timeout);// throw(PersClientException);

	void SetProperty(ProfileType pt, const char* key, Property& prop);// throw(PersClientException);
	void SetProperty(ProfileType pt, uint32_t key, Property& prop);// throw(PersClientException);

	void GetProperty(ProfileType pt, const char* key, const char *property_name, Property& prop);// throw(PersClientException);
	void GetProperty(ProfileType pt, uint32_t key, const char *property_name, Property& prop);// throw(PersClientException);

	void DelProperty(ProfileType pt, const char* key, const char *property_name);// throw(PersClientException);
	void DelProperty(ProfileType pt, uint32_t key, const char *property_name);// throw(PersClientException);

	void IncProperty(ProfileType pt, const char* key, const char *property_name, int32_t inc);// throw(PersClientException);
	void IncProperty(ProfileType pt, uint32_t key, const char *property_name, int32_t inc);// throw(PersClientException);

protected:
	static bool  inited;
	static Mutex initLock;

	void init();
	void init_internal(const char *_host, int _port, int timeout); //throw(PersClientException);
	void SetPacketSize();
	void FillStringHead(PersCmd pc, ProfileType pt, const char *key);
	void FillIntHead(PersCmd pc, ProfileType pt, uint32_t key);
	void AppendString(const char *str);
	void SendPacket();
	void ReadPacket();
	void WriteAllTO(char* buf, uint32_t sz);
	void ReadAllTO(char* buf, uint32_t sz);
	uint32_t GetPacketSize();
	PersServerResponseType GetServerResponse();
	void ParseProperty(Property& prop);

	string host;
	int port;
	int timeout;
	Socket sock;
	bool connected;
    Logger * log;
	Mutex mtx;
	SerialBuffer sb;
};

}}}

#endif

