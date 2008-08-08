/* $Id$ */

#ifndef SCAG_PERS_COMMAND_DISPATCHER
#define SCAG_PERS_COMMAND_DISPATCHER

#include "logger/Logger.h"
#include "scag/util/storage/SerialBuffer.h"
#include "ProfileStore.h"
#include "Types.h"

namespace scag { namespace pers {

using smsc::logger::Logger;

using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;

class CommandDispatcher {
    StringProfileStore* AbonentStore;
public:
    CommandDispatcher(StringProfileStore *abonent, IntProfileStore *service, IntProfileStore *oper, IntProfileStore *provider);
    ~CommandDispatcher() {};
    void Execute(SerialBuffer& isb, SerialBuffer& osb);
protected:
    typedef struct {
        ProfileType pt;
        IntProfileStore* store;
    } IntStore;

	void execCommand(SerialBuffer& isb, SerialBuffer& osb);
    IntProfileStore* findStore(ProfileType pt);
    void SendResponse(SerialBuffer& sb, PersServerResponseType r);
    void SetPacketSize(SerialBuffer& sb);
    void DelCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb);
    void GetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb);
    void SetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb);
    void IncCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb);
    void IncResultCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb);
    void IncModCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, int mod, SerialBuffer& osb);
    Logger * log;
#define INT_STORE_CNT 3
    IntStore int_store[INT_STORE_CNT];
};

}}

#endif

