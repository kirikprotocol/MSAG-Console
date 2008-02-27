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

	PersServerResponseType execCommand(SerialBuffer& isb, SerialBuffer& osb);
    IntProfileStore* findStore(ProfileType pt);
    Profile* getProfile(const string& key);
    //void deleteProfile();
   string getProfileKey(const string& key) const;
    Profile* createProfile(AbntAddr& addr);
    StringProfileStore* getAbonentStore() { return AbonentStore; };
    void SendResponse(SerialBuffer& sb, PersServerResponseType r);
    void SetPacketSize(SerialBuffer& sb);
    PersServerResponseType DelCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb);
    PersServerResponseType GetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb);
    PersServerResponseType SetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb);
    PersServerResponseType IncCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb);
    PersServerResponseType IncResultCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb);
    PersServerResponseType IncModCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, int mod, SerialBuffer& osb);

    void rollbackCommands(PersServerResponseType error_code);
    void resetStroragesBackup();
    void setNeedBackup(bool needBackup);

protected:
    Logger * plog;
    bool transactBatch;
#define INT_STORE_CNT 3
    IntStore int_store[INT_STORE_CNT];
};

}}

#endif

