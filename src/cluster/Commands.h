#ifndef __SMSC_CLUSTER_COMMANDS__
#define __SMSC_CLUSTER_COMMANDS__

#include <sys/types.h>
#include <netinet/in.h>
#include <string>
#include "smeman/smeinfo.h"
#include "sms/sms.h"
#include <vector>
#include "acls/interfaces.h"
#include "core/buffers/File.hpp"
#include "profiler/profiler-types.hpp"

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif



namespace smsc { namespace cluster
{

    const int SMEID_LENGTH = smsc::sms::MAX_SMESYSID_TYPE_LENGTH + 1;

    typedef enum {
        APPLYROUTES_CMD =           0x00000000,
        APPLYALIASES_CMD =          0x00000001,
        APPLYRESCHEDULE_CMD =       0x00000002,
        PROFILEUPDATE_CMD =         0x00010000,
        PROFILEDELETE_CMD =         0x00010001,
        MSCREGISTRATE_CMD =         0x00020000,
        MSCUNREGISTER_CMD =         0x00020001,
        MSCBLOCK_CMD =              0x00020002,
        MSCCLEAR_CMD =              0x00020003,
        MSCREPORT_CMD =             0x00020004,
        SMEADD_CMD =                0x00030000,
        SMEREMOVE_CMD =             0x00030001,
        SMEUPDATE_CMD =             0x00030002,
        ACLREMOVE_CMD =             0x00040000,
        ACLCREATE_CMD =             0x00040001,
        ACLUPDATEINFO_CMD =         0x00040002,
        ACLREMOVEADDRESSES_CMD =    0x00040003,
        ACLADDADDRESSES_CMD =       0x00040004,
        PRCADDPRINCIPAL_CMD =       0x00050000,
        PRCDELETEPRINCIPAL_CMD =    0x00050001,
        PRCALTERPRINCIPAL_CMD =     0x00050002,
        MEMADDMEMBER_CMD =          0x00060000,
        MEMDELETEMEMBER_CMD =       0x00060001,
        SBMADDSUBMITER_CMD =        0x00070000,
        SBMDELETESUBMITER_CMD =     0x00070001,
        DLADD_CMD =                 0x00080000,
        DLDELETE_CMD =              0x00080001,
        DLALTER_CMD =               0x00080002,
        GETROLE_CMD =               0x00090001
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

        inline CommandType getType() const {
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

    //=========== Profile commans ===========

    class ProfileUpdateCommand : public Command
    {
    private:
         uint8_t plan;
         uint8_t type;
         char address[21];
         smsc::profiler::Profile profile;

    public:
        ProfileUpdateCommand(const smsc::sms::Address& addr,const smsc::profiler::Profile& profile_);
        ProfileUpdateCommand() : Command(PROFILEUPDATE_CMD) {};

        virtual ~ProfileUpdateCommand() {};
        void getArgs(smsc::profiler::Profile &profile, uint8_t &plan_, uint8_t &type_, char* address_) const;
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
        void getArgs(uint8_t &plan_, uint8_t &type_, char * addr_) const;
    };

    //=========== Msc Commands ==============

    class MscRegistrateCommand : public Command
    {
    private:
        char mscNum[22];
        File::offset_type offset;
    public:
        MscRegistrateCommand(const char *mscNum_,File::offset_type argOffset);
        MscRegistrateCommand() : Command(MSCREGISTRATE_CMD) {};

        virtual ~MscRegistrateCommand() {};

        void MscRegistrateCommand::getArgs(char *mscNum_,File::offset_type& argOffset) const;
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

        void MscUnregisterCommand::getArgs(char *mscNum_) const;
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

        void MscBlockCommand::getArgs(char *mscNum_) const;
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

        void MscClearCommand::getArgs(char *mscNum_) const;
        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class MscReportCommand : public Command
    {
    private:
        char mscNum[22];
        bool status;
        File::offset_type offset;
    public:
        MscReportCommand(const char *mscNum_, bool status_, File::offset_type offset_);
        MscReportCommand() : Command(MSCREPORT_CMD) {};

        virtual ~MscReportCommand() {};

        void MscReportCommand::getArgs(char *mscNum_, bool &status_, File::offset_type &offset) const;
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

        void SmeAddCommand::getArgs(smsc::smeman::SmeInfo &si_) const;
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

        void SmeRemoveCommand::getArgs(char *smeId__) const;
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

        void SmeUpdateCommand::getArgs(smsc::smeman::SmeInfo &si_) const;
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

        void AclRemoveCommand::getArgs(smsc::acls::AclIdent& id) const;
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
        File::offset_type offset;
        std::vector<std::string> phones;
    public:
        AclCreateCommand() : Command(ACLCREATE_CMD) {};
        AclCreateCommand(std::string name_, std::string desc_,
                            std::string type_, File::offset_type offset_, std::vector<std::string> phones_);

        virtual ~AclCreateCommand() {};

        void AclCreateCommand::getArgs(std::string& name_, std::string& desc_,
                            std::string& type_, bool &type_present, File::offset_type &offset_, std::vector<std::string>& phones_) const;
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

        void AclUpdateInfoCommand::getArgs(smsc::acls::AclIdent& id, std::string& name_, std::string& desc_,
                                    std::string& type_) const;
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

        void AclRemoveAddressesCommand::getArgs(smsc::acls::AclIdent& id, std::vector<std::string>& addresses_) const;
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

        void AclAddAddressesCommand::getArgs(smsc::acls::AclIdent& id, std::vector<std::string>& addr) const;
        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Prc Commands ==============

    class PrcAddPrincipalCommand : public Command
    {
    private:
        int maxLists;
        int maxElements;
        File::offset_type offset;
        std::string address;
    public:
        PrcAddPrincipalCommand(int maxLists_, int maxElements_, File::offset_type offset_, std::string address_);
        PrcAddPrincipalCommand() : Command(PRCADDPRINCIPAL_CMD) {};

        virtual ~PrcAddPrincipalCommand() {};

        void PrcAddPrincipalCommand::getArgs(int& maxLists_, int& maxElements_, File::offset_type &offset_, std::string& address_) const;
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

        void PrcDeletePrincipalCommand::getArgs(std::string& address_) const;
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

        void PrcAlterPrincipalCommand::getArgs(int &maxLists_, int &maxElements_, std::string &addresses_) const;
        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== Mem Commands ==============

    class MemAddMemberCommand : public Command
    {
    private:
        std::string dlname;
        std::string address;
        File::offset_type offset;
    public:
        MemAddMemberCommand(File::offset_type offset_, std::string dlname_, std::string address_);
        MemAddMemberCommand() : Command(MEMADDMEMBER_CMD) {};

        virtual ~MemAddMemberCommand() {};

        void MemAddMemberCommand::getArgs(File::offset_type &offset_, std::string &dlname_, std::string &address_) const;
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

        void MemDeleteMemberCommand::getArgs(std::string &dlname_, std::string &address_) const;
        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== SbmCommands ===============

    class SbmAddSubmiterCommand : public Command
    {
    private:
        std::string dlname;
        std::string address;
        File::offset_type offset;
    public:
        SbmAddSubmiterCommand(File::offset_type offset_, std::string dlname_, std::string address_);
        SbmAddSubmiterCommand() : Command(SBMADDSUBMITER_CMD) {};

        virtual ~SbmAddSubmiterCommand() {};

        void SbmAddSubmiterCommand::getArgs(File::offset_type &offset_, std::string &dlname_, std::string &address_) const;
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

        void SbmDeleteSubmiterCommand::getArgs(std::string &dlname_, std::string &address_) const;
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
        File::offset_type offset1;
        File::offset_type offset2;
    public:
        DlAddCommand(int maxElements_, std::string dlname_, std::string owner_, File::offset_type offset1_, File::offset_type offset2_);
        DlAddCommand() : Command(DLADD_CMD) {};

        virtual ~DlAddCommand() {};

        void DlAddCommand::getArgs(int &maxElements_, File::offset_type &offset1_, File::offset_type &offset2_, std::string &dlname_, std::string &owner_) const;
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

        void DlDeleteCommand::getArgs(std::string &dlname_) const;
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

        void DlAlterCommand::getArgs(int &maxElements_, std::string &dlname_) const;
        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    //=========== GetRole command ===============

    class GetRoleCommand : public Command
    {
    protected:
        uint32_t role;
    public:
        GetRoleCommand() : Command(GETROLE_CMD), role(0) {};
        GetRoleCommand(int role_) : Command(GETROLE_CMD), role(role_) {};

        virtual ~GetRoleCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };









    // TODO: Add more commands ...
}}

#endif // __SMSC_CLUSTER_COMMANDS__
