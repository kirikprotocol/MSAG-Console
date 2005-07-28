#ifndef __SMSC_CLUSTER_COMMANDS__
#define __SMSC_CLUSTER_COMMANDS__

#include <sys/types.h>
#include <netinet/in.h>
#include <string>
#include "smeman/smeinfo.h"
#include "sms/sms_const.h"
#include <vector>
#include "acls/interfaces.h"

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif

const int SMEID_LENGTH = smsc::sms::MAX_SMESYSID_TYPE_LENGTH + 1;

namespace smsc { namespace cluster 
{
    typedef enum {
        APPLYROUTES_CMD,
        APPLYALIASES_CMD,
        APPLYRESCHEDULE_CMD,
        APPLYSMSCCONFIG_CMD,
        APPLYSERVICES_CMD,
        APPLYLOCALRESOURCE_CMD,
        PROFILEUPDATE_CMD,
        PROFILEDELETE_CMD,
        MSCREGISTRATE_CMD,
        MSCUNREGISTER_CMD,
        MSCBLOCK_CMD,
        MSCCLEAR_CMD,
        SMEADD_CMD,
        SMEREMOVE_CMD,
        SMEUPDATE_CMD,
        ACLREMOVE_CMD,
        ACLCREATE_CMD,
        ACLUPDATEINFO_CMD,
        ACLREMOVEADDRESSES_CMD,
        ACLADDADDRESSES_CMD,
        PRCADDPRINCIPAL_CMD,
        PRCDELETEPRINCIPAL_CMD,
        PRCALTERPRINCIPAL_CMD,
        MEMADDMEMBER_CMD,
        MEMDELETEMEMBER_CMD,
        SBMADDSUBMITER_CMD,
        SBMDELETESUBMITER_CMD,
        DLADD_CMD,
        DLDELETE_CMD,
        DLALTER_CMD
        // TODO: Add other command types...
    } CommandType;

    class Command
    {
    protected:

        CommandType type;
        Command(CommandType _type) : type(_type) {};

    public:
        
        /**
         * Method creates command from buffer.
         * First 2 bytes defines command type.
         * To use from CommandReader
         */ 
        static Command* create(CommandType type, void* buffer, uint32_t len);
        
        virtual ~Command() {}
        
        inline CommandType getType() {
            return type;
        }

        virtual void* serialize(uint32_t& len) = 0;
        virtual bool deserialize(void* buffer, uint32_t len) = 0;
    };

    // #######################  Particular commands declarations #######################  

    class ApplyRoutesCommand : public Command
    {
    private:
        
        

    public:
        
        ApplyRoutesCommand() : Command(APPLYROUTES_CMD) {};
        //ApplyRoutesCommand(int sf) : Command(SAMPLE_CMD), sampleField(sf) {};

        virtual ~ApplyRoutesCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class ApplyAliasesCommand : public Command
    {
    public:
        ApplyAliasesCommand() : Command(APPLYALIASES_CMD) {};

        virtual ~ApplyAliasesCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class ApplyRescheduleCommand : public Command
    {
    public:
        ApplyRescheduleCommand() : Command(APPLYRESCHEDULE_CMD) {};

        virtual ~ApplyRescheduleCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class ApplySmscConfigCommand : public Command
    {
    public:
        ApplySmscConfigCommand() : Command(APPLYSMSCCONFIG_CMD) {};

        virtual ~ApplySmscConfigCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class ApplyServicesCommand : public Command
    {
    public:
        ApplyServicesCommand() : Command(APPLYSERVICES_CMD) {};

        virtual ~ApplyServicesCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class ApplyLocaleResourceCommand : public Command
    {
    public:
        ApplyLocaleResourceCommand() : Command(APPLYLOCALRESOURCE_CMD) {};

        virtual ~ApplyLocaleResourceCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Profile commans ===========

    class ProfileUpdateCommand : public Command
    {
    private:
        uint8_t plan;
        uint8_t type;
        char address[21];
        int codePage;
        int reportOption;
        int hideOption;

        bool hideModifaible;
        bool divertModifaible;
        bool udhContact;
        bool translit;

        bool divertActive;
        bool divertActiveAbsent;
        bool divertActiveBlocked;
        bool divertActiveBarred;
        bool divertActiveCapacity;

        std::string local;
        std::string divert;

    public:
        ProfileUpdateCommand(uint8_t plan_, uint8_t type_, char *address_, int codePage_, int reportOption_,
                                int hideOption_, bool hideModifaible_, bool divertModifaible_, bool udhContact_,
                                bool translit_, bool divertActive_, bool divertActiveAbsent_, bool divertActiveBlocked_,
                                bool divertActiveBarred_, bool divertActiveCapacity_, std::string local_, std::string divert_);
        ProfileUpdateCommand() : Command(PROFILEUPDATE_CMD) {};

        virtual ~ProfileUpdateCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };
    
    class ProfileDeleteCommand : public Command
    {
    private:
        uint8_t plan;
        uint8_t type;
        char address[21];

    public:
        ProfileDeleteCommand(uint8_t plan_, uint8_t type_, const char *address_);
        ProfileDeleteCommand() : Command(PROFILEDELETE_CMD) {};

        virtual ~ProfileDeleteCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Msc Commands ==============

    class MscRegistrateCommand : public Command
    {
    private:
        char mscNum[22];
    public:
        MscRegistrateCommand(const char *mscNum_);
        MscRegistrateCommand() : Command(MSCREGISTRATE_CMD) {};

        virtual ~MscRegistrateCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class MscUnregisterCommand : public Command
    {
    private:
        char mscNum[22];
    public:
        MscUnregisterCommand(const char *mscNum_);
        MscUnregisterCommand() : Command(MSCUNREGISTER_CMD) {};

        virtual ~MscUnregisterCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class MscBlockCommand : public Command
    {
    private:
        char mscNum[22];
    public:
        MscBlockCommand(const char *mscNum_);
        MscBlockCommand() : Command(MSCBLOCK_CMD) {};

        virtual ~MscBlockCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class MscClearCommand : public Command
    {
    private:
        char mscNum[22];
    public:
        MscClearCommand(const char *mscNum_);
        MscClearCommand() : Command(MSCCLEAR_CMD) {};

        virtual ~MscClearCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Sme Commands ==============

    class SmeAddCommand : public Command
    {
    private:
        smsc::smeman::SmeInfo si;
    public:
        SmeAddCommand(smsc::smeman::SmeInfo si_);
        SmeAddCommand() : Command(SMEADD_CMD) {};

        virtual ~SmeAddCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class SmeRemoveCommand : public Command
    {
    private:
        char smeId[SMEID_LENGTH];
    public:
        SmeRemoveCommand(const char *smeId_);
        SmeRemoveCommand() : Command(SMEREMOVE_CMD) {};

        virtual ~SmeRemoveCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class SmeUpdateCommand : public Command
    {
    private:
        smsc::smeman::SmeInfo si;
    public:
        SmeUpdateCommand(smsc::smeman::SmeInfo si_);
        SmeUpdateCommand() : Command(SMEUPDATE_CMD) {};

        virtual ~SmeUpdateCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Acl Commands ==============

    class AclRemoveCommand : public Command
    {
    private:
        smsc::acls::AclIdent aclId;
    public:
        AclRemoveCommand(smsc::acls::AclIdent id);
        AclRemoveCommand() : Command(ACLREMOVE_CMD) {};

        virtual ~AclRemoveCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class AclCreateCommand : public Command
    {
    private:
        std::string name;
        std::string description;
        std::string cache_type;
        bool cache_type_present;
        std::vector<std::string> phones;
    public:
        AclCreateCommand() : Command(ACLCREATE_CMD) {};
        AclCreateCommand(std::string name_, std::string desc_,
                            std::string type_, std::vector<std::string> phones_);

        virtual ~AclCreateCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class AclUpdateInfoCommand : public Command
    {
    private:
        smsc::acls::AclIdent aclId;
        std::string name;
        std::string description;
        std::string cache_type;
    public:
        AclUpdateInfoCommand(smsc::acls::AclIdent id, std::string name_, std::string desc_,
                                    std::string type_);
        AclUpdateInfoCommand() : Command(ACLUPDATEINFO_CMD) {};

        virtual ~AclUpdateInfoCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class AclRemoveAddressesCommand : public Command
    {
    private:
        smsc::acls::AclIdent aclId;
        std::vector<std::string> addresses;
    public:
        AclRemoveAddressesCommand(smsc::acls::AclIdent id, std::vector<std::string> addresses_);
        AclRemoveAddressesCommand() : Command(ACLREMOVEADDRESSES_CMD) {};

        virtual ~AclRemoveAddressesCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class AclAddAddressesCommand : public Command
    {
    private:
        smsc::acls::AclIdent aclId;
        std::vector<std::string> addresses;
    public:
        AclAddAddressesCommand() : Command(ACLADDADDRESSES_CMD) {};
        AclAddAddressesCommand(smsc::acls::AclIdent id, std::vector<std::string> addr);

        virtual ~AclAddAddressesCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Prc Commands ==============

    class PrcAddPrincipalCommand : public Command
    {
    private:
        int maxLists;
        int maxElements;
        std::string address;
    public:
        PrcAddPrincipalCommand(int maxLists_, int maxElements_, std::string address_);
        PrcAddPrincipalCommand() : Command(PRCADDPRINCIPAL_CMD) {};

        virtual ~PrcAddPrincipalCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class PrcDeletePrincipalCommand : public Command
    {
    private:
        std::string address;
    public:
        PrcDeletePrincipalCommand(std::string address_);
        PrcDeletePrincipalCommand() : Command(PRCDELETEPRINCIPAL_CMD) {};

        virtual ~PrcDeletePrincipalCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class PrcAlterPrincipalCommand : public Command
    {
    private:
        int maxLists;
        int maxElements;
        std::string addresses;
    public:
        PrcAlterPrincipalCommand(int maxLists_, int maxElements_, std::string addresses_);
        PrcAlterPrincipalCommand() : Command(PRCALTERPRINCIPAL_CMD) {};

        virtual ~PrcAlterPrincipalCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Mem Commands ==============

    class MemAddMemberCommand : public Command
    {
    private:
        std::string dlname;
        std::string address;
    public:
        MemAddMemberCommand(std::string dlname_, std::string address_);
        MemAddMemberCommand() : Command(MEMADDMEMBER_CMD) {};

        virtual ~MemAddMemberCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class MemDeleteMemberCommand : public Command
    {
    private:
        std::string dlname;
        std::string address;
    public:
        MemDeleteMemberCommand(std::string dlname_, std::string address_);
        MemDeleteMemberCommand() : Command(MEMDELETEMEMBER_CMD) {};

        virtual ~MemDeleteMemberCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== SbmCommands ===============

    class SbmAddSubmiterCommand : public Command
    {
    private:
        std::string dlname;
        std::string address;
    public:
        SbmAddSubmiterCommand(std::string dlname_, std::string address_);
        SbmAddSubmiterCommand() : Command(SBMADDSUBMITER_CMD) {};

        virtual ~SbmAddSubmiterCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class SbmDeleteSubmiterCommand : public Command
    {
    private:
        std::string dlname;
        std::string address;
    public:
        SbmDeleteSubmiterCommand(std::string dlname_, std::string address_);
        SbmDeleteSubmiterCommand() : Command(SBMDELETESUBMITER_CMD) {};

        virtual ~SbmDeleteSubmiterCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Dl Commands ===============

    class DlAddCommand : public Command
    {
    private:
        int maxElements;
        std::string dlname;
        std::string owner;
    public:
        DlAddCommand(int maxElements_, std::string dlname_, std::string owner_);
        DlAddCommand() : Command(DLADD_CMD) {};

        virtual ~DlAddCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class DlDeleteCommand : public Command
    {
    private:
        std::string dlname;
    public:
        DlDeleteCommand(std::string dlname_);
        DlDeleteCommand() : Command(DLDELETE_CMD) {};

        virtual ~DlDeleteCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class DlAlterCommand : public Command
    {
    private:
        int maxElements;
        std::string dlname;
    public:
        DlAlterCommand(int maxElements_, std::string dlname_);
        DlAlterCommand() : Command(DLALTER_CMD) {};

        virtual ~DlAlterCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };









    // TODO: Add more commands ...
}}

#endif // __SMSC_CLUSTER_COMMANDS__

