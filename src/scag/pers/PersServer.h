/* $Id$ */

#ifndef SCAG_PERS_COMMAND_DISPATCHER
#define SCAG_PERS_COMMAND_DISPATCHER

#include "logger/Logger.h"
#include "SerialBuffer.h"
#include "ProfileStore.h"
#include "Types.h"
#include "PersSocketServer.h"

namespace scag { namespace pers {

using smsc::logger::Logger;

class PersServer : public PersSocketServer {
    StringProfileStore* AbonentStore;

public:
    PersServer(const char* persHost_, int persPort_, int maxClientCount_, int timeout_, int transactTimeout_,
                StringProfileStore *abonent, IntProfileStore *service, IntProfileStore *oper,
                IntProfileStore *provider);
    virtual ~PersServer() {};
    virtual bool processPacket(ConnectionContext& ctx);

protected:
    typedef struct {
        ProfileType pt;
        IntProfileStore* store;
    } IntStore;

	void execCommand(SerialBuffer& isb, SerialBuffer& osb);
    IntProfileStore* findStore(ProfileType pt);
    Profile* getProfile(const string& key);
   string getProfileKey(const string& key) const;
    Profile* createProfile(AbntAddr& addr);
    StringProfileStore* getAbonentStore() { return AbonentStore; };
    void SendResponse(SerialBuffer& sb, PersServerResponseType r);
    void SetPacketSize(SerialBuffer& sb);
    void DelCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb);
    void GetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb);
    void SetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb);
    void IncCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb);
    void IncModCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, int mod, SerialBuffer& osb);

protected:
    Logger * plog;
#define INT_STORE_CNT 3
    IntStore int_store[INT_STORE_CNT];
};

}}

#endif

