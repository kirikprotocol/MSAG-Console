/* $Id$ */

#ifndef SCAG_PERS_COMMAND_DISPATCHER
#define SCAG_PERS_COMMAND_DISPATCHER

#include "logger/Logger.h"
#include "SerialBuffer.h"
#include "ProfileStore.h"
#include "Types.h"

namespace scag { namespace pers {

using smsc::logger::Logger;

class CommandDispatcher {
    StringProfileStore* AbonentStore;
public:
    CommandDispatcher(StringProfileStore *abonent, IntProfileStore *service, IntProfileStore *oper, IntProfileStore *provider);
    ~CommandDispatcher() {};
    void Execute(SerialBuffer* sb);
protected:
    typedef struct {
        ProfileType pt;
        IntProfileStore* store;
    } IntStore;

    IntProfileStore* findStore(ProfileType pt);
    void SendResponse(SerialBuffer *sb, PersServerResponseType r);
    void SetPacketSize(SerialBuffer *sb);
    void DelCmdHandler(ProfileType pt, uint32_t int_key, std::string& str_key, std::string& name, SerialBuffer *sb);
    void GetCmdHandler(ProfileType pt, uint32_t int_key, std::string& str_key, std::string& name, SerialBuffer *sb);
    void SetCmdHandler(ProfileType pt, uint32_t int_key, std::string& str_key, Property& prop, SerialBuffer *sb);
    void IncCmdHandler(ProfileType pt, uint32_t int_key, std::string& str_key, Property& prop, SerialBuffer *sb);
    void IncModCmdHandler(ProfileType pt, uint32_t int_key, std::string& str_key, Property& prop, int mod, SerialBuffer *sb);
    Logger * log;
#define INT_STORE_CNT 3
    IntStore int_store[INT_STORE_CNT];
};

}}

#endif

