/* $Id$ */

#ifndef SCAG_PERS_CLIENT
#define SCAG_PERS_CLIENT

#include "logger/Logger.h"
#include "SerialBuffer.h"
#include "Property.h"
#include "core/network/Socket.hpp"

namespace scag { namespace pers { namespace client {

using smsc::core::network::Socket;

enum ProfileType{
	PT_ABONENT = 1,
	PT_OPERATOR,
	PT_PROVIDER,
	PT_SERVICE
};

enum PersCmd{
	PC_DEL = 1,
	PC_SET,
	PC_GET
};

using smsc::logger::Logger;
using scag::pers;

class PersClient {
public:
    PersClient() : connected(false), log(Logger::getInstance("client")) {};
    ~PersClient() {};

	void init(const char *_host, int _port, int timeout);

	void SetProperty(ProfileType pt, const char* key, Property *prop);
	void SetProperty(ProfileType pt, uint32_t key, Property *prop);

	Property* GetProperty(ProfileType pt, const char* key, const char *property_name);
	Property* GetProperty(ProfileType pt, uint32_t key, const char *property_name);

	void DelProperty(ProfileType pt, const char* key, const char *property_name);
	void DelProperty(ProfileType pt, uint32_t key, const char *property_name);

protected:
	void SetPacketSize();
	void FillStringHead(PersCmd pc, ProfileType pt, const char *key);
	void FillIntHead(PersCmd pc, ProfileType pt, uint32_t key);

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

