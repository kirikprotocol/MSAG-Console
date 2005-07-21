#ifndef __SMSC_CLUSTER_COMMANDS__
#define __SMSC_CLUSTER_COMMANDS__

#include <sys/types.h>
#include <netinet/in.h>

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif

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
    public:
        ProfileUpdateCommand() : Command(PROFILEUPDATE_CMD) {};

        virtual ~ProfileUpdateCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };
    
    class ProfileDeleteCommand : public Command
    {
    public:
        ProfileDeleteCommand() : Command(PROFILEDELETE_CMD) {};

        virtual ~ProfileDeleteCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Msc Commands ==============

    class MscRegistrateCommand : public Command
    {
    public:
        MscRegistrateCommand() : Command(MSCREGISTRATE_CMD) {};

        virtual ~MscRegistrateCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class MscUnregisterCommand : public Command
    {
    public:
        MscUnregisterCommand() : Command(MSCUNREGISTER_CMD) {};

        virtual ~MscUnregisterCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class MscBlockCommand : public Command
    {
    public:
        MscBlockCommand() : Command(MSCBLOCK_CMD) {};

        virtual ~MscBlockCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class MscClearCommand : public Command
    {
    public:
        MscClearCommand() : Command(MSCCLEAR_CMD) {};

        virtual ~MscClearCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Sme Commands ==============

    class SmeAddCommand : public Command
    {
    public:
        SmeAddCommand() : Command(SMEADD_CMD) {};

        virtual ~SmeAddCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class SmeRemoveCommand : public Command
    {
    public:
        SmeRemoveCommand() : Command(SMEREMOVE_CMD) {};

        virtual ~SmeRemoveCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class SmeUpdateCommand : public Command
    {
    public:
        SmeUpdateCommand() : Command(SMEUPDATE_CMD) {};

        virtual ~SmeUpdateCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Acl Commands ==============

    class AclRemoveCommand : public Command
    {
    public:
        AclRemoveCommand() : Command(ACLREMOVE_CMD) {};

        virtual ~AclRemoveCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class AclCreateCommand : public Command
    {
    public:
        AclCreateCommand() : Command(ACLCREATE_CMD) {};

        virtual ~AclCreateCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class AclUpdateInfoCommand : public Command
    {
    public:
        AclUpdateInfoCommand() : Command(ACLUPDATEINFO_CMD) {};

        virtual ~AclUpdateInfoCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class AclRemoveAddressesCommand : public Command
    {
    public:
        AclRemoveAddressesCommand() : Command(ACLREMOVEADDRESSES_CMD) {};

        virtual ~AclRemoveAddressesCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class AclAddAddressesCommand : public Command
    {
    public:
        AclAddAddressesCommand() : Command(ACLADDADDRESSES_CMD) {};

        virtual ~AclAddAddressesCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Prc Commands ==============

    class PrcAddPrincipalCommand : public Command
    {
    public:
        PrcAddPrincipalCommand() : Command(PRCADDPRINCIPAL_CMD) {};

        virtual ~PrcAddPrincipalCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class PrcDeletePrincipalCommand : public Command
    {
    public:
        PrcDeletePrincipalCommand() : Command(PRCDELETEPRINCIPAL_CMD) {};

        virtual ~PrcDeletePrincipalCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class PrcAlterPrincipalCommand : public Command
    {
    public:
        PrcAlterPrincipalCommand() : Command(PRCALTERPRINCIPAL_CMD) {};

        virtual ~PrcAlterPrincipalCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Mem Commands ==============

    class MemAddMemberCommand : public Command
    {
    public:
        MemAddMemberCommand() : Command(MEMADDMEMBER_CMD) {};

        virtual ~MemAddMemberCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class MemDeleteMemberCommand : public Command
    {
    public:
        MemDeleteMemberCommand() : Command(MEMDELETEMEMBER_CMD) {};

        virtual ~MemDeleteMemberCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== SbmCommands ===============

    class SbmAddSubmiterCommand : public Command
    {
    public:
        SbmAddSubmiterCommand() : Command(SBMADDSUBMITER_CMD) {};

        virtual ~SbmAddSubmiterCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class SbmDeleteSubmiterCommand : public Command
    {
    public:
        SbmDeleteSubmiterCommand() : Command(SBMDELETESUBMITER_CMD) {};

        virtual ~SbmDeleteSubmiterCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Dl Commands ===============

    class DlAddCommand : public Command
    {
    public:
        DlAddCommand() : Command(DLADD_CMD) {};

        virtual ~DlAddCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class DlDeleteCommand : public Command
    {
    public:
        DlDeleteCommand() : Command(DLDELETE_CMD) {};

        virtual ~DlDeleteCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class DlAlterCommand : public Command
    {
    public:
        DlAlterCommand() : Command(DLALTER_CMD) {};

        virtual ~DlAlterCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };









    // TODO: Add more commands ...
}}

#endif // __SMSC_CLUSTER_COMMANDS__

