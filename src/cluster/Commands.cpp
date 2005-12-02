
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/IntHash.hpp"
#include "smpp/smpp_structures.h"
#include "profiler/profiler-types.hpp"
#include "smeman/smeinfo.h"

#include "Commands.h"

namespace smsc { namespace cluster
{
    using namespace smsc::core::synchronization;
    using smsc::core::buffers::IntHash;

    class CommandFactory
    {
    private:

        class FakeRegistry
        {
            IntHash<CommandFactory *>& getInstance() {
                static IntHash<CommandFactory *>  _factories;
                return _factories;
            }

        public:

            operator IntHash<CommandFactory *> () {
                return getInstance();
            }
            int Exist(int type) {
                return getInstance().Exist(type);
            }
            int Insert(int type, CommandFactory* factory) {
                return getInstance().Insert(type, factory);
            }
            CommandFactory* Get(int type) {
                return getInstance().Get(type);
            }
        };
        static FakeRegistry factories;
        static bool factoriesInited;

        static void registerFactory(int type, CommandFactory* factory) {
            if (factory && !factories.Exist(type))
            {
                factories.Insert(type, factory);
            }
        };
        static CommandFactory* getFactory(int type) {
            return (factories.Exist(type) ? factories.Get(type):0);
        };

    protected:

        CommandFactory(CommandType type) {
            CommandFactory::registerFactory((int)type, this);
        };

        /**
         * Method creates new instance of particular command.
         * Need to be implemented in derrived factories
         *
         * @return command    new command instance
         */
        virtual Command* create() = 0;

    public:

        // Contains all static factories instantiations
        static void initFactories();

        static Command* createCommand(CommandType type)
        {
            static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("IM.cmd");
            CommandFactory* factory = getFactory((int)type);
            if(!factory)
            {
              smsc_log_warn(log,"Failed to get factory for type=%d",type);
              return 0;
            }
            Command* rv=factory->create();
            if(!rv)
            {
              smsc_log_warn(log,"Factory failed to create command type=%d!",type);
            }
            return rv;
        };

        virtual ~CommandFactory() {};
    };

    bool CommandFactory::factoriesInited = false;
    CommandFactory::FakeRegistry CommandFactory::factories;

/* ################# Particular commands factories ################ */


    class ApplyRoutesCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new ApplyRoutesCommand(); };
    public:
        ApplyRoutesCommandFactory() : CommandFactory(APPLYROUTES_CMD) {};
        virtual ~ApplyRoutesCommandFactory() {};
    };

    //========== applyAliases =======================

    class ApplyAliasesCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new ApplyAliasesCommand(); };
    public:
        ApplyAliasesCommandFactory() : CommandFactory(APPLYALIASES_CMD) {};
        virtual ~ApplyAliasesCommandFactory() {};
    };

    //========== applyReschedule =======================

    class ApplyRescheduleCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new ApplyRescheduleCommand(); };
    public:
        ApplyRescheduleCommandFactory() : CommandFactory(APPLYRESCHEDULE_CMD) {};
        virtual ~ApplyRescheduleCommandFactory() {};
    };

    //========== profileUpdate =======================

    class ProfileUpdateCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new ProfileUpdateCommand(); };
    public:
        ProfileUpdateCommandFactory() : CommandFactory(PROFILEUPDATE_CMD) {};
        virtual ~ProfileUpdateCommandFactory() {};
    };

    //========== profileDelete =======================

    class ProfileDeleteCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new ProfileDeleteCommand(); };
    public:
        ProfileDeleteCommandFactory() : CommandFactory(PROFILEDELETE_CMD) {};
        virtual ~ProfileDeleteCommandFactory() {};
    };

    //========== mscRegistrate =======================

    class MscRegistrateCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new MscRegistrateCommand(); };
    public:
        MscRegistrateCommandFactory() : CommandFactory(MSCREGISTRATE_CMD) {};
        virtual ~MscRegistrateCommandFactory() {};
    };

    //========== mscUnregister =======================

    class MscUnregisterCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new MscUnregisterCommand(); };
    public:
        MscUnregisterCommandFactory() : CommandFactory(MSCUNREGISTER_CMD) {};
        virtual ~MscUnregisterCommandFactory() {};
    };

    //========== mscBlock =======================

    class MscBlockCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new MscBlockCommand(); };
    public:
        MscBlockCommandFactory() : CommandFactory(MSCBLOCK_CMD) {};
        virtual ~MscBlockCommandFactory() {};
    };

    //========== mscClear =======================

    class MscClearCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new MscClearCommand(); };
    public:
        MscClearCommandFactory() : CommandFactory(MSCCLEAR_CMD) {};
        virtual ~MscClearCommandFactory() {};
    };

    //========== mscClear =======================

    class MscReportCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new MscReportCommand(); };
    public:
        MscReportCommandFactory() : CommandFactory(MSCREPORT_CMD) {};
        virtual ~MscReportCommandFactory() {};
    };

    //========== smeAdd =======================

    class SmeAddCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new SmeAddCommand(); };
    public:
        SmeAddCommandFactory() : CommandFactory(SMEADD_CMD) {};
        virtual ~SmeAddCommandFactory() {};
    };

    //========== smeRemove =======================

    class SmeRemoveCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new SmeRemoveCommand(); };
    public:
        SmeRemoveCommandFactory() : CommandFactory(SMEREMOVE_CMD) {};
        virtual ~SmeRemoveCommandFactory() {};
    };

    //========== smeUpdate =======================

    class SmeUpdateCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new SmeUpdateCommand(); };
    public:
        SmeUpdateCommandFactory() : CommandFactory(SMEUPDATE_CMD) {};
        virtual ~SmeUpdateCommandFactory() {};
    };

    //========== aclRemove =======================

    class AclRemoveCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new AclRemoveCommand(); };
    public:
        AclRemoveCommandFactory() : CommandFactory(ACLREMOVE_CMD) {};
        virtual ~AclRemoveCommandFactory() {};
    };

    //========== aclCreate =======================

    class AclCreateCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new AclCreateCommand(); };
    public:
        AclCreateCommandFactory() : CommandFactory(ACLCREATE_CMD) {};
        virtual ~AclCreateCommandFactory() {};
    };

    //========== aclCreate =======================

    class AclUpdateInfoCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new AclUpdateInfoCommand(); };
    public:
        AclUpdateInfoCommandFactory() : CommandFactory(ACLUPDATEINFO_CMD) {};
        virtual ~AclUpdateInfoCommandFactory() {};
    };

    //========== aclRemoveAddresses =======================

    class AclRemoveAddressesCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new AclRemoveAddressesCommand(); };
    public:
        AclRemoveAddressesCommandFactory() : CommandFactory(ACLREMOVEADDRESSES_CMD) {};
        virtual ~AclRemoveAddressesCommandFactory() {};
    };

    //========== aclAddAddresses =======================

    class AclAddAddressesCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new AclRemoveAddressesCommand(); };
    public:
        AclAddAddressesCommandFactory() : CommandFactory(ACLADDADDRESSES_CMD) {};
        virtual ~AclAddAddressesCommandFactory() {};
    };

    //========== prcAddPrincipal =======================

    class PrcAddPrincipalCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new PrcAddPrincipalCommand(); };
    public:
        PrcAddPrincipalCommandFactory() : CommandFactory(PRCADDPRINCIPAL_CMD) {};
        virtual ~PrcAddPrincipalCommandFactory() {};
    };

    //========== prcDeletePrincipal =======================

    class PrcDeletePrincipalCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new PrcDeletePrincipalCommand(); };
    public:
        PrcDeletePrincipalCommandFactory() : CommandFactory(PRCDELETEPRINCIPAL_CMD) {};
        virtual ~PrcDeletePrincipalCommandFactory() {};
    };

    //========== prcAlterPrincipal =======================

    class PrcAlterPrincipalCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new PrcAlterPrincipalCommand(); };
    public:
        PrcAlterPrincipalCommandFactory() : CommandFactory(PRCALTERPRINCIPAL_CMD) {};
        virtual ~PrcAlterPrincipalCommandFactory() {};
    };

    //========== memAddMember =======================

    class MemAddMemberCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new MemAddMemberCommand(); };
    public:
        MemAddMemberCommandFactory() : CommandFactory(MEMADDMEMBER_CMD) {};
        virtual ~MemAddMemberCommandFactory() {};
    };

    //========== memDeleteMember =======================

    class MemDeleteMemberCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new MemDeleteMemberCommand(); };
    public:
        MemDeleteMemberCommandFactory() : CommandFactory(MEMDELETEMEMBER_CMD) {};
        virtual ~MemDeleteMemberCommandFactory() {};
    };

    //========== sbmAddSubmiter =======================

    class SbmAddSubmiterCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new SbmAddSubmiterCommand(); };
    public:
        SbmAddSubmiterCommandFactory() : CommandFactory(SBMADDSUBMITER_CMD) {};
        virtual ~SbmAddSubmiterCommandFactory() {};
    };

    //========== sbmDeleteSubmiter =======================

    class SbmDeleteSubmiterCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new SbmDeleteSubmiterCommand(); };
    public:
        SbmDeleteSubmiterCommandFactory() : CommandFactory(SBMDELETESUBMITER_CMD) {};
        virtual ~SbmDeleteSubmiterCommandFactory() {};
    };

    //========== dlAdd =======================

    class DlAddCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new DlAddCommand(); };
    public:
        DlAddCommandFactory() : CommandFactory(DLADD_CMD) {};
        virtual ~DlAddCommandFactory() {};
    };

    //========== dlDelete =======================

    class DlDeleteCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new DlDeleteCommand(); };
    public:
        DlDeleteCommandFactory() : CommandFactory(DLDELETE_CMD) {};
        virtual ~DlDeleteCommandFactory() {};
    };

    //========== dlAlter =======================

    class DlAlterCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new DlAlterCommand(); };
    public:
        DlAlterCommandFactory() : CommandFactory(DLALTER_CMD) {};
        virtual ~DlAlterCommandFactory() {};
    };
    // TODO: Add more actual factories for particular commands


/* ####################### General routines ####################### */

Mutex initFactoriesLock;
void CommandFactory::initFactories()
{
    MutexGuard guard(initFactoriesLock);

    if (!CommandFactory::factoriesInited)
    {
        static ApplyRoutesCommandFactory            _applyRoutesCommandFactory;
        static ApplyAliasesCommandFactory           _applyAliasesCommandFactory;
        static ApplyRescheduleCommandFactory        _applyRescheduleCommandFactory;

        static ProfileUpdateCommandFactory      _profileUpdateCommandFactory;
        static ProfileDeleteCommandFactory      _profileDeleteCommandFactory;

        static MscRegistrateCommandFactory      _mscRegistrateCommandFactory;
        static MscUnregisterCommandFactory      _mscUnregisterCommandFactory;
        static MscBlockCommandFactory           _mscBlockCommandFactory;
        static MscClearCommandFactory           _mscClearCommandFactory;
        static MscReportCommandFactory          _mscReportCommandFactory;

        static SmeAddCommandFactory             _smeAddCommandFactory;
        static SmeRemoveCommandFactory          _smeRemoveCommandFactory;
        static SmeUpdateCommandFactory          _smeUpdateCommandFactory;

        static AclRemoveCommandFactory              _aclRemoveCommandFactory;
        static AclCreateCommandFactory              _aclCreateCommandFactory;
        static AclUpdateInfoCommandFactory          _aclUpdateInfoCommandFactory;
        static AclRemoveAddressesCommandFactory     _aclRemoveAddressesCommandFactory;
        static AclAddAddressesCommandFactory        _aclAddAddressesCommandFactory;

        static PrcAddPrincipalCommandFactory        _prcAddPrincipalCommandFactory;
        static PrcDeletePrincipalCommandFactory     _prcDeletePrincipalCommandFactory;
        static PrcAlterPrincipalCommandFactory      _prcAlterPrincipalCommandFactory;

        static MemAddMemberCommandFactory           _memAddMemberCommandFactory;
        static MemDeleteMemberCommandFactory        _memDeleteMemberCommandFactory;

        static SbmAddSubmiterCommandFactory         _sbmAddSubmiterCommandFactory;
        static SbmDeleteSubmiterCommandFactory      _sbmDeleteSubmiterCommandFactory;

        static DlAddCommandFactory                  _dlAddCommandFactory;
        static DlDeleteCommandFactory               _dlDeleteCommandFactory;
        static DlAlterCommandFactory                _dlAlterCommandFactory;


        // TODO: Create more actual factories for particular commands

        CommandFactory::factoriesInited = true;
    }
}

Command* Command::create(CommandType type, void* buffer, uint32_t len) // static
{
    CommandFactory::initFactories();

    //printf("\nCommand, create, type: %02X\n\n", type);
    Command* command = CommandFactory::createCommand(type);
    if (command && !command->deserialize(buffer, len))
    {
        delete command;
        return 0;
    }
    return command;
}

/* ############### Particular commands declarations ############### */

void* ApplyRoutesCommand::serialize(uint32_t &len)
{
    len = 0;
    return 0;
}
bool ApplyRoutesCommand::deserialize(void *buffer, uint32_t len)
{
    if(len!=0)
    {
      smsc_log_warn(smsc::logger::Logger::getInstance("im.cmd"),"len>0 for empty command ApplyRoutesCommand (len=%d)",len);
    }

    return true;
}

//========== applyAliases =======================

void* ApplyAliasesCommand::serialize(uint32_t &len)
{
    len = 0;
    return 0;
}
bool ApplyAliasesCommand::deserialize(void *buffer, uint32_t len)
{
    if(len!=0)
    {
      smsc_log_warn(smsc::logger::Logger::getInstance("im.cmd"),"len>0 for empty command ApplyAliasesCommand (len=%d)",len);
    }

    return true;
}

//========== applyReschedule =======================

void* ApplyRescheduleCommand::serialize(uint32_t &len)
{
    len = 0;
    return 0;
}
bool ApplyRescheduleCommand::deserialize(void *buffer, uint32_t len)
{
    if(len!=0)
    {
      smsc_log_warn(smsc::logger::Logger::getInstance("im.cmd"),"len>0 for empty command ApplyRescheduleCommand (len=%d)",len);
    }

    return true;
}

//========== profileUpdate ==========================

ProfileUpdateCommand::ProfileUpdateCommand(const smsc::sms::Address& addr,const smsc::profiler::Profile& profile_)
        : Command(PROFILEUPDATE_CMD),
          profile(profile_)
{
  plan=addr.plan;
  type=addr.type;
  strcpy(address,addr.value);
}

void ProfileUpdateCommand::getArgs(smsc::profiler::Profile &profile_, uint8_t &plan_, uint8_t &type_, char *address_) const
{
    profile_ = profile;
    plan_ = plan;
    type_ = type;
    strcpy(address_, address);
}

void* ProfileUpdateCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 46 + profile.locale.length() + profile.divert.length();
    buffer = new uint8_t[len];

    uint8_t val;

    //============== Sets plan, type and Address =============
    memcpy((void*)buffer,               (const void*)&plan, 1);
    memcpy((void*)(buffer + 1),         (const void*)&type, 1);
    memcpy((void*)(buffer + 2),         (const void*)&address, 21);
    //printf("p: %d, t: %d, address: '%s'\n", plan, type, address);

    //============= Puts codePage in buffer ===============

    //printf("codePage: ");
    /*switch(profile.codepage){
    case (int)smsc::smpp::DataCoding::SMSC7BIT:
        val = 0;
        //printf("SMSC7BIT\n");
        break;
    case (int)smsc::smpp::DataCoding::LATIN1:
        val = 1;
        //printf("LATIN1\n");
        break;
    case (int)smsc::smpp::DataCoding::UCS2:
        val = 2;
        //printf("UCS2\n");
        break;
    case (int)smsc::smpp::DataCoding::UCS2 | smsc::smpp::DataCoding::LATIN1:
        val = 3;
        //printf("UCS2 | LATIN1\n");
        break;
    }*/

    uint32_t val32 = profile.codepage;
    uint32_t value32 = htonl(val32);
    memcpy((void*)(buffer + 23),        (const void*)&value32, 4);

    //============= Puts reportOption in buffer ===========

    //printf("reportOption: ");
    /*switch(profile.reportoptions){
    case smsc::profiler::ProfileReportOptions::ReportNone:
        val = 0;
        //printf("ReportNone\n");
        break;
    case smsc::profiler::ProfileReportOptions::ReportFull:
        val = 1;
        //printf("ReportFull\n");
        break;
    case smsc::profiler::ProfileReportOptions::ReportFinal:
        val = 2;
        //printf("ReportFinal\n");
        break;
    }*/

    val32 = profile.reportoptions;
    value32 = htonl(val32);
    memcpy((void*)(buffer + 27),        (const void*)&value32, 4);

    //============= Puts hideOption in buffer ===========

    //printf("hideOption: ");
    /*switch(profile.hide){
    case smsc::profiler::HideOption::hoDisabled:
        val = 0;
        //printf("hoDisabled\n");
        break;
    case smsc::profiler::HideOption::hoEnabled:
        val = 1;
        //printf("hoEnabled\n");
        break;
    case smsc::profiler::HideOption::hoSubstitute:
        val = 2;
        //printf("hoSubstitute\n");
        break;
    }*/

    val32 = profile.hide;
    value32 = htonl(value32);
    memcpy((void*)(buffer + 31),        (const void*)&value32, 4);

    //============= Puts flags in buffer ================

    val = (uint8_t)0;

#define BIT(x) (((uint8_t)1)<<((uint8_t)x))

    if(profile.hideModifiable)
        val |= (uint8_t)1;
    if(profile.divertModifiable)
        val |= BIT(1);
    if(profile.udhconcat)
        val |= BIT(2);
    if(profile.translit)
        val |= BIT(3);

    /*if(profile.hideModifiable)
        printf("hideModifaible ok\n");
    if(profile.divertModifiable)
        printf("divertModifaible ok\n");
    if(profile.udhconcat)
        printf("udhContact ok\n");
    if(profile.translit)
        printf("translit ok\n");

    if(profile.codepage & 0x80)
        val |= BIT(4);

    if(profile.codepage & 0x80)
        printf("codePage & 0x80 ok\n");*/

#undef BIT

    memcpy((void*)(buffer + 35),        (const void*)&val, 1);

    //============= Puts divertActive in buffer ================

    val = (uint8_t)0;

#define BIT(x) (((uint8_t)1)<<((uint8_t)x))

    if(profile.divertActive)
        val |= (uint8_t)1;
    if(profile.divertActiveAbsent)
        val |= BIT(1);
    if(profile.divertActiveBlocked)
        val |= BIT(2);
    if(profile.divertActiveBarred)
        val |= BIT(3);
    if(profile.divertActiveCapacity)
        val |= BIT(4);

    /*if(profile.divertActive)
        printf("divertActive ok\n");
    if(profile.divertActiveAbsent)
        printf("divertActiveAbsent ok\n");
    if(profile.divertActiveBlocked)
        printf("divertActiveBlocked ok\n");
    if(profile.divertActiveBarred)
        printf("divertActiveBarred ok\n");
    if(profile.divertActiveCapacity)
        printf("divertActiveCapacity ok\n");*/

#undef BIT

    memcpy((void*)(buffer + 36),        (const void*)&val, 1);

    //============= Puts offset =================================

    uint64_t val64 = profile.offset;
    uint64_t tmp = val64;;
    unsigned char *ptr=(unsigned char *)&val64;
    ptr[0]=(unsigned char)(tmp>>56);
    ptr[1]=(unsigned char)(tmp>>48)&0xFF;
    ptr[2]=(unsigned char)(tmp>>40)&0xFF;
    ptr[3]=(unsigned char)(tmp>>32)&0xFF;
    ptr[4]=(unsigned char)(tmp>>24)&0xFF;
    ptr[5]=(unsigned char)(tmp>>16)&0xFF;
    ptr[6]=(unsigned char)(tmp>>8)&0xFF;
    ptr[7]=(unsigned char)(tmp&0xFF);

    memcpy((void*)(buffer + 37),        (const void*)&val64, 8);

    //============= Puts local and divert in buffer =============

    memcpy((void*)(buffer + 45),                  (const void*)profile.locale.c_str(), profile.locale.length() + 1);
    //printf("local: '%s', len: %d\n", profile.locale.c_str(), profile.locale.length());
    memcpy((void*)(buffer + 46 + profile.locale.length()), (const void*)profile.divert.c_str(), profile.divert.length() + 1);
    //printf("divert: '%s', len: %d\n", profile.divert.c_str(), profile.divert.length());

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool ProfileUpdateCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 46|| !buffer)
        return false;

    /*

        buffer := <AddressInfo> <ProfileInfo>

        <AddressInfo> := <plan : 1> <type : 1> <Address : 21>

        <ProfileInfo> := <codePage : 1> <reportOption : 1> <hideOption : 1> <divertActive : 1> <flags : 1> <offset : 8>
                            <local : string> <divert : string>

        <codePage : 1>
                0 - default
                1 - Latin1
                2 - Ucs2
                3 - Ucs2AndLat


        <reportOption : 1>      Values matchs with constants of ProfileReportOptions namespace
                0 - ReportNone
                1 - ReportFull
                2 - ReportFinal

        <hideOption : 1>        Values matchs constants of HideOption namespace
                0 - hoDisabled
                1 - hoEnabled
                2 - hoSubstitute


        <divertActive : 1>
                1 bit - profile.divertActive
                2 bit - profile.divertActiveAbsent
                3 bit - profile.divertActiveBlocked
                4 bit - profile.divertActiveBarred
                5 bit - profile.divertActiveCapacity

        <flags : 1>
                1 bit - profile.hideModifaible
                2 bit - profile.divertModifaible
                3 bit - profile.udhContact
                4 bit - profile.translit
                5 bit - udds7bit (codePage)
    */

    try {

    uint8_t val;

    //====== Gets plan, type and Address =======

    memcpy((void*)&plan,    (const void*)buffer, 1);
    memcpy((void*)&type,    (const void*)((uint8_t*)buffer + 1), 1);
    memcpy((void*)&address, (const void*)((uint8_t*)buffer + 2), 21);
    //printf("p: %d, t: %d, address: '%s'\n", plan, type, address);

    //============= Gets codePage ===============

    uint32_t val32;
    memcpy((void*)&val32,     (const void*)((uint8_t*)buffer + 23), 4);
    profile.codepage = ntohl(val32);

    /*switch(val){
    case 0:
        profile.codepage = smsc::smpp::DataCoding::SMSC7BIT;
        break;
    case 1:
        profile.codepage = smsc::smpp::DataCoding::LATIN1;
        break;
    case 2:
        profile.codepage = smsc::smpp::DataCoding::UCS2;
        break;
    case 3:
        profile.codepage = smsc::smpp::DataCoding::UCS2 | smsc::smpp::DataCoding::LATIN1;
        break;
    }*/

    //printf("codePage: ");
    /*if(profile.codepage == smsc::smpp::DataCoding::SMSC7BIT)
        printf("SMSC7BIT\n");
    if(profile.codepage & (int)smsc::smpp::DataCoding::LATIN1)
        printf("LATIN1\n");
    if(profile.codepage & (int)smsc::smpp::DataCoding::UCS2)
        printf("UCS2\n");*/

    //============= Gets reportOption ===========

    memcpy((void*)&val32,     (const void*)((uint8_t*)buffer + 27), 4);
    profile.reportoptions = ntohl(val32);

    /*switch(val){
    case 0:
        profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportNone;
        break;
    case 1:
        profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportFull;
        break;
    case 2:
        profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportFinal;
        break;
    }*/

    //printf("reportOption: ");
    /*if(profile.reportoptions == smsc::profiler::ProfileReportOptions::ReportNone)
        printf("ReportNone\n");
    if(profile.reportoptions == smsc::profiler::ProfileReportOptions::ReportFull)
        printf("ReportFull\n");
    if(profile.reportoptions == smsc::profiler::ProfileReportOptions::ReportFinal)
        printf("ReportFinal\n");*/

    //============= Gets hideOption ===========

    memcpy((void*)&val32,     (const void*)((uint8_t*)buffer + 31), 4);
    profile.hide = ntohl(val32);

    /*switch(val){
    case 0:
        profile.hide = smsc::profiler::HideOption::hoDisabled;
        break;
    case 1:
        profile.hide = smsc::profiler::HideOption::hoEnabled;
        break;
    case 2:
        profile.hide = smsc::profiler::HideOption::hoSubstitute;
        break;
    }*/

    //printf("hideOption: ");
    /*if(profile.hide == smsc::profiler::HideOption::hoDisabled)
        printf("hoDisabled\n");
    if(profile.hide == smsc::profiler::HideOption::hoEnabled)
        printf("hoEnabled\n");
    if(profile.hide == smsc::profiler::HideOption::hoSubstitute)
        printf("hoSubstitute\n");*/

    //============= Gets flags ================

    memcpy((void*)&val,     (const void*)((uint8_t*)buffer + 35), 1);

#define BIT(x) (((uint8_t)1)<<((uint8_t)x))

    profile.hideModifiable =    val & (uint8_t)1;
    profile.divertModifiable =  val & BIT(1);
    profile.udhconcat =         val & BIT(2);
    profile.translit =          val & BIT(3);

    /*if(profile.hideModifiable)
        printf("hideModifaible ok\n");
    if(profile.divertModifiable)
        printf("divertModifaible ok\n");
    if(profile.udhconcat)
        printf("udhContact ok\n");
    if(profile.translit)
        printf("translit ok\n");

    if(val & BIT(4))
        profile.codepage |= 0x80;

    if(profile.codepage |= 0x80)
        printf("codePage |= 0x80 ok\n");*/

#undef BIT

    //============= Gets divertActive ================

    memcpy((void*)&val,     (const void*)((uint8_t*)buffer + 36), 1);

#define BIT(x) (((uint8_t)1)<<((uint8_t)x))

    profile.divertActive =          val & (uint8_t)1;
    profile.divertActiveAbsent =    val & BIT(1);
    profile.divertActiveBlocked =   val & BIT(2);
    profile.divertActiveBarred =    val & BIT(3);
    profile.divertActiveCapacity =  val & BIT(4);

    /*if(profile.divertActive)
        printf("divertActive ok\n");
    if(profile.divertActiveAbsent)
        printf("divertActiveAbsent ok\n");
    if(profile.divertActiveBlocked)
        printf("divertActiveBlocked ok\n");
    if(profile.divertActiveBarred)
        printf("divertActiveBarred ok\n");
    if(profile.divertActiveCapacity)
        printf("divertActiveCapacity ok\n");*/

#undef BIT

    //============= Gets offset ======================

    uint64_t val64;
    memcpy((void*)&val64,       (const void*)( (uint8_t*)buffer + 37 ), 8);

    uint64_t tmp=0;
    memset(&tmp, 0, 8);
    unsigned char *ptr=(unsigned char *)&val64;
    tmp = (uint64_t)( ptr[0] ) << 56;
    tmp += (uint64_t)( ptr[1] ) << 48;
    tmp += (uint64_t)( ptr[2] ) << 40;
    tmp += (uint64_t)( ptr[3] ) << 32;
    tmp += (uint64_t)( ptr[4] ) << 24;
    tmp += (uint64_t)( ptr[5] ) << 16;
    tmp += (uint64_t)( ptr[6] ) << 8;
    tmp += (uint64_t)( ptr[7] );

    profile.offset = tmp;

    //============= Gets local and divert ============

    profile.locale = (const char*)((uint8_t*)buffer + 45);

    //printf("local: '%s', len: %d\n", local.c_str(), local.length());

    if(45 + profile.locale.length() >= len)
        return false;

    profile.divert = (const char*)((uint8_t*)buffer + 46 + profile.locale.length());

    //printf("divert: '%s', len: %d", divert.c_str(), divert.length());

    if(46 + profile.locale.length() + profile.divert.length() != len)
        return false;

    }catch(...){
        return false;
    }


    return true;
}

//========== profileDelete ==========================

ProfileDeleteCommand::ProfileDeleteCommand(uint8_t plan_, uint8_t type_, const char *address_)
    : Command(PROFILEDELETE_CMD),
    plan(plan_),
    type(type_)
{
    strcpy(address, address_);
}

void ProfileDeleteCommand::getArgs(uint8_t &plan_, uint8_t &type_, char * addr_) const
{
    plan_ = plan;
    type_ = type;
    strcpy(addr_, address);
}

void* ProfileDeleteCommand::serialize(uint32_t &len)
{

    uint8_t* buffer = 0;

    try {

    len = 23;

    buffer = new uint8_t[len];

    //====== Gets plan, type and Address =======

    memcpy((void*)buffer,       (const void*)&plan, 1);
    memcpy((void*)(buffer + 1), (const void*)&type, 1);
    memcpy((void*)(buffer + 2), (const void*)&address, 21);
    //printf("p: %d, t: %d, address: '%s'\n", plan, type, address);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool ProfileDeleteCommand::deserialize(void *buffer, uint32_t len)
{
    if(len != 23 || !buffer)
        return false;

    try {

    /*

        buffer := <AddressInfo>

        <AddressInfo> := <plan : 1> <type : 1> <Address : 21>

    */

    //====== Gets plan, type and Address =======

    memcpy((void*)&plan,    (const void*)buffer, 1);
    memcpy((void*)&type,    (const void*)((uint8_t*)buffer + 1), 1);
    memcpy((void*)&address, (const void*)((uint8_t*)buffer + 2), 21);
    //printf("p: %d, t: %d, address: '%s'\n", plan, type, address);

    }catch(...){
        return false;
    }

    return true;
}

//========== mscRegistrate ==========================

MscRegistrateCommand::MscRegistrateCommand(const char *mscNum_,File::offset_type argOffset)
    : Command(MSCREGISTRATE_CMD)
{
    strcpy(mscNum, mscNum_);
    offset=argOffset;
}

void MscRegistrateCommand::getArgs(char *mscNum_,File::offset_type& argOffset) const
{
    strcpy(mscNum_, mscNum);
    argOffset=offset;
}


static File::offset_type htonl64(File::offset_type value)
{
  uint32_t h=htonl((uint32_t)((value>>32)&0xFFFFFFFFUL));
  uint32_t l=htonl((uint32_t)(value&0xFFFFFFFFUL));
  char buf[8];
  *((uint32_t*)buf)=h;
  *((uint32_t*)(buf+4))=l;
  return *((File::offset_type*)buf);
}

static File::offset_type ntohl64(File::offset_type value)
{
  char* buf=(char*)&value;
  uint32_t h=ntohl(*(uint32_t*)buf);
  uint32_t l=ntohl(*(uint32_t*)(buf+4));
  return (((File::offset_type)h)<<32)|l;
}


void* MscRegistrateCommand::serialize(uint32_t &len)
{
    uint8_t* buffer = 0;

    try {

    len = 22+8;

    buffer = new uint8_t[len];
    //printf("mscNum: '%s', len: %d\n", mscNum, strlen(mscNum));
    memcpy((void*)buffer, (const void*)mscNum, 22);
    File::offset_type tmp=htonl64(offset);
    memcpy((void*)( buffer + 22 ) , &tmp,8);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool MscRegistrateCommand::deserialize(void *buffer, uint32_t len)
{
    if(len != 22+8 || !buffer)
        return false;

    try {

    memcpy((void*)mscNum, (const void*)buffer, 22);
    //printf("mscNum: '%s', len: %d\n", mscNum, strlen(mscNum));
    offset=ntohl64(*(File::offset_type*)((char*)buffer+22));

    }catch(...){
        return false;
    }

    return true;
}

//========== mscUnregister ==========================

MscUnregisterCommand::MscUnregisterCommand(const char *mscNum_)
    : Command(MSCUNREGISTER_CMD)
{
    strcpy(mscNum, mscNum_);
}

void MscUnregisterCommand::getArgs(char *mscNum_) const
{
    strcpy(mscNum_, mscNum);
}

void* MscUnregisterCommand::serialize(uint32_t &len)
{

    uint8_t* buffer = 0;

    try {

    len = 22;

    //printf("mscNum: '%s', len: %d\n", mscNum, strlen(mscNum));
    buffer = new uint8_t[len];
    memcpy((void*)buffer, (const void*)mscNum, 22);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool MscUnregisterCommand::deserialize(void *buffer, uint32_t len)
{
    if(len != 22 || !buffer)
        return false;

    try {

    memcpy((void*)mscNum, (const void*)buffer, 22);
    //printf("mscNum: '%s', len: %d\n", mscNum, strlen(mscNum));

    }catch(...){
        return 0;
    }

    return true;
}

//========== mscBlock ==========================

MscBlockCommand::MscBlockCommand(const char *mscNum_)
    : Command(MSCBLOCK_CMD)
{
    strcpy(mscNum, mscNum_);
}

void MscBlockCommand::getArgs(char *mscNum_) const
{
    strcpy(mscNum_, mscNum);
}

void* MscBlockCommand::serialize(uint32_t &len)
{

    uint8_t* buffer = 0;

    try {

    len = 22;

    //printf("mscNum: '%s', len: %d\n", mscNum, strlen(mscNum));
    buffer = new uint8_t[len];
    memcpy((void*)buffer, (const void*)mscNum, 22);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool MscBlockCommand::deserialize(void *buffer, uint32_t len)
{

    if(len != 22 || !buffer)
        return false;

    try {

    memcpy((void*)mscNum, (const void*)buffer, 22);
    //printf("mscNum: '%s', len: %d\n", mscNum, strlen(mscNum));

    }catch(...){
        return false;
    }

    return true;
}

//========== MscClear ==========================

MscClearCommand::MscClearCommand(const char *mscNum_)
    : Command(MSCCLEAR_CMD)
{
    strcpy(mscNum, mscNum_);
}

void MscClearCommand::getArgs(char *mscNum_) const
{
    strcpy(mscNum_, mscNum);
}

void* MscClearCommand::serialize(uint32_t &len)
{

    uint8_t* buffer = 0;

    try {

    len = 22;

    //printf("mscNum: '%s', len: %d\n", mscNum, strlen(mscNum));
    buffer = new uint8_t[len];
    memcpy((void*)buffer, (const void*)mscNum, 22);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool MscClearCommand::deserialize(void *buffer, uint32_t len)
{
    if(len != 22 || !buffer)
        return false;

    try {

    memcpy((void*)mscNum, (const void*)buffer, 22);
    //printf("mscNum: '%s', len: %d\n", mscNum, strlen(mscNum));

    }catch(...){
        return false;
    }

    return true;
}

//========== MscReport ==========================

MscReportCommand::MscReportCommand(const char *mscNum_, bool status_, File::offset_type offset_)
    : Command(MSCREPORT_CMD),
      status(status_),
      offset(offset_)
{
    strcpy(mscNum, mscNum_);
}

void MscReportCommand::getArgs(char *mscNum_, bool &status_, File::offset_type &offset_) const
{
    strcpy(mscNum_, mscNum);
    status_ = status;
    offset_ = offset;
}

void* MscReportCommand::serialize(uint32_t &len)
{

    uint8_t* buffer = 0;

    try {

    len = 31;

    buffer = new uint8_t[len];

    uint8_t val = status ? 1 : 0;
    memcpy((void*)buffer, (const void*)&val, 1);

    uint64_t val64 = offset;
    uint64_t tmp = val64;;
    unsigned char *ptr=(unsigned char *)&val64;
    ptr[0]=(unsigned char)(tmp>>56);
    ptr[1]=(unsigned char)(tmp>>48)&0xFF;
    ptr[2]=(unsigned char)(tmp>>40)&0xFF;
    ptr[3]=(unsigned char)(tmp>>32)&0xFF;
    ptr[4]=(unsigned char)(tmp>>24)&0xFF;
    ptr[5]=(unsigned char)(tmp>>16)&0xFF;
    ptr[6]=(unsigned char)(tmp>>8)&0xFF;
    ptr[7]=(unsigned char)(tmp&0xFF);

    memcpy((void*)(buffer + 1),        (const void*)&val64, 8);

    //printf("mscNum: '%s', len: %d\n", mscNum, strlen(mscNum));
    buffer = new uint8_t[len];
    memcpy((void*)(buffer + 9), (const void*)mscNum, 22);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool MscReportCommand::deserialize(void *buffer, uint32_t len)
{
    if(len != 31 || !buffer)
        return false;

    try {

    uint8_t val;
    memcpy((void*)&val, (const void*)buffer, 1);
    status = val;

    uint64_t val64;
    memcpy((void*)&val64,       (const void*)( (uint8_t*)buffer + 1 ), 8);

    uint64_t tmp=0;
    memset(&tmp, 0, 8);
    unsigned char *ptr=(unsigned char *)&val64;
    tmp = (uint64_t)( ptr[0] ) << 56;
    tmp += (uint64_t)( ptr[1] ) << 48;
    tmp += (uint64_t)( ptr[2] ) << 40;
    tmp += (uint64_t)( ptr[3] ) << 32;
    tmp += (uint64_t)( ptr[4] ) << 24;
    tmp += (uint64_t)( ptr[5] ) << 16;
    tmp += (uint64_t)( ptr[6] ) << 8;
    tmp += (uint64_t)( ptr[7] );

    offset = tmp;
    memcpy((void*)mscNum, (const void*)( (char*)buffer + 9), 22);
    //printf("mscNum: '%s', len: %d\n", mscNum, strlen(mscNum));

    }catch(...){
        return false;
    }

    return true;
}

//========== smeAdd ==========================

SmeAddCommand::SmeAddCommand(smsc::smeman::SmeInfo si_)
    : Command(SMEADD_CMD),
    si(si_)
{
}

void SmeAddCommand::getArgs(smsc::smeman::SmeInfo &si_) const
{
    si_ = si;
}

void* SmeAddCommand::serialize(uint32_t &len)
{

  uint8_t *buffer = 0;

  try {

  len = 38 + si.rangeOfAddress.length() + si.systemType.length() +
                si.password.length() + si.hostname.length() + si.systemId.length() +
                si.receiptSchemeName.length();

  buffer = new uint8_t[len];

  uint32_t value32;

  //printf("typeOfNumber: %d, numberingPlan: %d, interfaceVersion: %d\n", si.typeOfNumber, si.numberingPlan, si.interfaceVersion);

  memcpy((void*)buffer,         (const void*)&si.typeOfNumber, 1);
  memcpy((void*)(buffer + 1),   (const void*)&si.numberingPlan, 1);
  memcpy((void*)(buffer + 2),   (const void*)&si.interfaceVersion, 1);

  value32 = htonl(si.port);
  memcpy((void*)(buffer + 3),   (const void*)&value32, 4);
  value32 = htonl(si.priority);
  memcpy((void*)(buffer + 7),   (const void*)&value32, 4);
  value32 = htonl(si.SME_N);
  memcpy((void*)(buffer + 11),  (const void*)&value32, 4);
  value32 = htonl(si.timeout);
  memcpy((void*)(buffer + 15),  (const void*)&value32, 4);
  value32 = htonl(si.proclimit);
  memcpy((void*)(buffer + 19),  (const void*)&value32, 4);
  value32 = htonl(si.schedlimit);
  memcpy((void*)(buffer + 23),  (const void*)&value32, 4);
  value32 = htonl(si.providerId);
  memcpy((void*)(buffer + 27),  (const void*)&value32, 4);

  /*intf("port: %d, priority: %d, SME_N: %d, timeout: %d\nproclimit: %d, schedlimit: %d, providerId: %d\n", si.port,
            si.priority, si.SME_N, si.timeout, si.proclimit, si.schedlimit, si.providerId);*/

  uint8_t val;

  //printf("bindMode: ");
  switch(si.bindMode){
  case smsc::smeman::smeRX:
      val = 0;
      //printf("smeRX\n");
      break;
  case smsc::smeman::smeTX:
      val = 1;
      //printf("smeTX\n");
      break;
  case smsc::smeman::smeTRX:
      val = 2;
      //printf("smeTRX\n");
      break;
  }

  val = val << (uint8_t)4;

#define BIT(x) (((uint8_t)1)<<((uint8_t)x))

  if(si.disabled)
      val |= (uint8_t)1;
  if(si.wantAlias)
      val |= BIT(1);
  if(si.forceDC)
      val |= BIT(2);
  if(si.internal)
      val |= BIT(3);

  if(si.disabled)
      printf("disabled ok\n");
  if(si.wantAlias)
      printf("wantAlias ok\n");
  if(si.forceDC)
      printf("forceDC ok\n");
  if(si.internal)
      printf("internal ok\n");

#undef BIT

  memcpy((void*)(buffer + 31), (const void*)&val, 1);

  int sz = 0;
  memcpy((void*)( (uint8_t*)buffer + 32 ),      (const void*)si.rangeOfAddress.c_str(),     si.rangeOfAddress.length() + 1);
  sz += si.rangeOfAddress.length() + 1;
  //printf("rangeOfAddress: '%s', len: %d, sz: %d\n", si.rangeOfAddress.c_str(), si.rangeOfAddress.length(), sz);
  memcpy((void*)( (uint8_t*)buffer + 32 + sz ), (const void*)si.systemType.c_str(),         si.systemType.length() + 1);
  sz += si.systemType.length() + 1;
  //printf("systemType: '%s', len: %d, sz: %d\n", si.systemType.c_str(), si.systemType.length(), sz);
  memcpy((void*)( (uint8_t*)buffer + 32 + sz ), (const void*)si.password.c_str(),           si.password.length() + 1);
  sz += si.password.length() + 1;
  //printf("password: '%s', len: %d, sz: %d\n", si.password.c_str(), si.password.length(), sz);
  memcpy((void*)( (uint8_t*)buffer + 32 + sz ), (const void*)si.hostname.c_str(),           si.hostname.length() + 1);
  sz += si.hostname.length() + 1;
  //printf("hostname: '%s', len: %d, sz: %d\n", si.hostname.c_str(), si.hostname.length(), sz);
  memcpy((void*)( (uint8_t*)buffer + 32 + sz ), (const void*)si.systemId.c_str(),           si.systemId.length() + 1);
  sz += si.systemId.length() + 1;
  //printf("systemId: '%s', len: %d, sz: %d\n", si.systemId.c_str(), si.systemId.length(), sz);
  memcpy((void*)( (uint8_t*)buffer + 32 + sz ), (const void*)si.receiptSchemeName.c_str(),  si.receiptSchemeName.length() + 1);
  //printf("receiptSchemeName: '%s', len: %d, sz: %d\n", si.receiptSchemeName.c_str(), si.receiptSchemeName.length(), sz);

  }catch(...){
      return 0;
  }

  return (void*)buffer;
}
bool SmeAddCommand::deserialize(void *buffer, uint32_t len)
{

  try {

  if(len < 39 || !buffer)
      return false;

  uint8_t val;

  uint32_t value32;

  memcpy((void*)&si.typeOfNumber,       (const void*)buffer, 1);
  memcpy((void*)&si.numberingPlan,      (const void*)((uint8_t*)buffer + 1), 1);
  memcpy((void*)&si.interfaceVersion,   (const void*)((uint8_t*)buffer + 2), 1);

  //printf("typeOfNumber: %d, numberingPlan: %d, interfaceVersion: %d\n", si.typeOfNumber, si.numberingPlan, si.interfaceVersion);

  memcpy((void*)&value32,               (const void*)((uint8_t*)buffer + 3), 4);
  si.port = ntohl(value32);
  memcpy((void*)&value32,           (const void*)((uint8_t*)buffer + 7), 4);
  si.priority = ntohl(value32);
  memcpy((void*)&value32,              (const void*)((uint8_t*)buffer + 11), 4);
  si.SME_N = ntohl(value32);
  memcpy((void*)&value32,            (const void*)((uint8_t*)buffer + 15), 4);
  si.timeout = ntohl(value32);
  memcpy((void*)&value32,          (const void*)((uint8_t*)buffer + 19), 4);
  si.proclimit = ntohl(value32);
  memcpy((void*)&value32,         (const void*)((uint8_t*)buffer + 23), 4);
  si.schedlimit = ntohl(value32);
  memcpy((void*)&value32,         (const void*)((uint8_t*)buffer + 27), 4);
  si.providerId = ntohl(value32);

  /*printf("port: %d, priority: %d, SME_N: %d, timeout: %d\nproclimit: %d, schedlimit: %d, providerId: %d\n", si.port,
            si.priority, si.SME_N, si.timeout, si.proclimit, si.schedlimit, si.providerId);*/

  memcpy((void*)&val,                   (const void*)((uint8_t*)buffer + 31), 1);


#define BIT(x) (((uint8_t)1)<<((uint8_t)x))

  si.disabled =     val & (uint8_t)1;
  si.wantAlias =    val & BIT(1);
  si.forceDC =      val & BIT(2);
  si.internal =     val & BIT(3);

  /*if(si.disabled)
      printf("disabled ok\n");
  if(si.wantAlias)
      printf("wantAlias ok\n");
  if(si.forceDC)
      printf("forceDC ok\n");
  if(si.internal)
      printf("internal ok\n");*/

#undef BIT

  val = val >> (uint8_t)4;
  //printf("bindMode: ");
  switch(val){
  case 0:
      si.bindMode = smsc::smeman::smeRX;
      //printf("smeRX\n");
      break;
  case 1:
      si.bindMode = smsc::smeman::smeTX;
      //printf("smeTX\n");
      break;
  case 2:
      si.bindMode = smsc::smeman::smeTRX;
      //printf("smeTRX\n");
      break;
  }

  int sz = 0;
  si.rangeOfAddress =       (char*)(   (uint8_t*)buffer + 32   );           sz += si.rangeOfAddress.length() + 1;   if (32 + sz >= len) return false;
  //printf("rangeOfAddress: '%s', len: %d, sz: %d\n", si.rangeOfAddress.c_str(), si.rangeOfAddress.length(), sz);
  si.systemType =           (char*)(   (uint8_t*)buffer + 32 + sz   );      sz += si.systemType.length() + 1;       if (32 + sz >= len) return false;
  //printf("systemType: '%s', len: %d, sz: %d\n", si.systemType.c_str(), si.systemType.length(), sz);
  si.password =             (char*)(   (uint8_t*)buffer + 32 + sz   );      sz += si.password.length() + 1;         if (32 + sz >= len) return false;
  //printf("password: '%s', len: %d, sz: %d\n", si.password.c_str(), si.password.length(), sz);
  si.hostname =             (char*)(   (uint8_t*)buffer + 32 + sz   );      sz += si.hostname.length() + 1;         if (32 + sz >= len) return false;
  //printf("hostname: '%s', len: %d, sz: %d\n", si.hostname.c_str(), si.hostname.length(), sz);
  si.systemId =             (char*)(   (uint8_t*)buffer + 32 + sz   );      sz += si.systemId.length() + 1;         if (32 + sz >= len) return false;
  //printf("systemId: '%s', len: %d, sz: %d\n", si.systemId.c_str(), si.systemId.length(), sz);
  si.receiptSchemeName =    (char*)(   (uint8_t*)buffer + 32 + sz   );
  //printf("receiptSchemeName: '%s', len: %d, sz: %d\n", si.receiptSchemeName.c_str(), si.receiptSchemeName.length(), sz);

  }catch(...){
      return false;
  }

  return true;
}

//========== smeRemove ==========================

SmeRemoveCommand::SmeRemoveCommand(const char *smeId_)
    : Command(SMEREMOVE_CMD)
{
    strcpy(smeId, smeId_);
}

void SmeRemoveCommand::getArgs(char *smeId_) const
{
    strcpy(smeId_, smeId);
}

void* SmeRemoveCommand::serialize(uint32_t &len)
{
    uint8_t *buffer = 0;

    try {

    len = SMEID_LENGTH;

    buffer = new uint8_t[len];

    //printf("smeId: '%s', len: %d\n", smeId, strlen(smeId));
    memcpy((void*)buffer, (const void*)&smeId, SMEID_LENGTH);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool SmeRemoveCommand::deserialize(void *buffer, uint32_t len)
{
    if(len != SMEID_LENGTH || !buffer)
        return false;

    try {

    memcpy((void*)&smeId, (const void*)buffer, SMEID_LENGTH);
    //printf("smeId: '%s', len: %d\n", smeId, strlen(smeId));

    }catch(...){
        return false;
    }

    return true;
}

//========== SmeUpdate ==========================

SmeUpdateCommand::SmeUpdateCommand(smsc::smeman::SmeInfo si_)
    : Command(SMEUPDATE_CMD),
    si(si_)
{
}

void SmeUpdateCommand::getArgs(smsc::smeman::SmeInfo &si_) const
{
    si_ = si;
}

void* SmeUpdateCommand::serialize(uint32_t &len)
{


  uint8_t *buffer = 0;

  try {

  len = 38 + si.rangeOfAddress.length() + si.systemType.length() +
                si.password.length() + si.hostname.length() + si.systemId.length() +
                si.receiptSchemeName.length();

  buffer = new uint8_t[len];

  memcpy((void*)buffer,         (const void*)&si.typeOfNumber, 1);
  memcpy((void*)(buffer + 1),   (const void*)&si.numberingPlan, 1);
  memcpy((void*)(buffer + 2),   (const void*)&si.interfaceVersion, 1);

  //printf("typeOfNumber: %d, numberingPlan: %d, interfaceVersion: %d\n", si.typeOfNumber, si.numberingPlan, si.interfaceVersion);

  uint32_t value32;

  value32 = htonl(si.port);
  memcpy((void*)(buffer + 3),   (const void*)&value32, 4);
  value32 = htonl(si.priority);
  memcpy((void*)(buffer + 7),   (const void*)&value32, 4);
  value32 = htonl(si.SME_N);
  memcpy((void*)(buffer + 11),  (const void*)&value32, 4);
  value32 = htonl(si.timeout);
  memcpy((void*)(buffer + 15),  (const void*)&value32, 4);
  value32 = htonl(si.proclimit);
  memcpy((void*)(buffer + 19),  (const void*)&value32, 4);
  value32 = htonl(si.schedlimit);
  memcpy((void*)(buffer + 23),  (const void*)&value32, 4);
  value32 = htonl(si.providerId);
  memcpy((void*)(buffer + 27),  (const void*)&value32, 4);

  /*printf("port: %d, priority: %d, SME_N: %d, timeout: %d\nproclimit: %d, schedlimit: %d, providerId: %d\n", si.port,
            si.priority, si.SME_N, si.timeout, si.proclimit, si.schedlimit, si.providerId);*/

  uint8_t val;

  //printf("bindMode: ");
  switch(si.bindMode){
  case smsc::smeman::smeRX:
      val = 0;
      //printf("smeRX\n");
      break;
  case smsc::smeman::smeTX:
      val = 1;
      //printf("smeTX\n");
      break;
  case smsc::smeman::smeTRX:
      val = 2;
      //printf("smeTRX\n");
      break;
  }

  val = val << (uint8_t)4;

#define BIT(x) (((uint8_t)1)<<((uint8_t)x))

  if(si.disabled)
      val |= (uint8_t)1;
  if(si.wantAlias)
      val |= BIT(1);
  if(si.forceDC)
      val |= BIT(2);
  if(si.internal)
      val |= BIT(3);

  /*if(si.disabled)
      printf("disabled ok\n");
  if(si.wantAlias)
      printf("wantAlias ok\n");
  if(si.forceDC)
      printf("forceDC ok\n");
  if(si.internal)
      printf("internal ok\n");*/

#undef BIT

  memcpy((void*)(buffer + 31), (const void*)&val, 1);

  int sz = 0;
  memcpy((void*)( (uint8_t*)buffer + 32 ),      (const void*)si.rangeOfAddress.c_str(),     si.rangeOfAddress.length() + 1);
  sz += si.rangeOfAddress.length() + 1;
  //printf("rangeOfAddress: '%s', len: %d, sz: %d\n", si.rangeOfAddress.c_str(), si.rangeOfAddress.length(), sz);
  memcpy((void*)( (uint8_t*)buffer + 32 + sz ), (const void*)si.systemType.c_str(),         si.systemType.length() + 1);
  sz += si.systemType.length() + 1;
  //printf("systemType: '%s', len: %d, sz: %d\n", si.systemType.c_str(), si.systemType.length(), sz);
  memcpy((void*)( (uint8_t*)buffer + 32 + sz ), (const void*)si.password.c_str(),           si.password.length() + 1);
  sz += si.password.length() + 1;
  //printf("password: '%s', len: %d, sz: %d\n", si.password.c_str(), si.password.length(), sz);
  memcpy((void*)( (uint8_t*)buffer + 32 + sz ), (const void*)si.hostname.c_str(),           si.hostname.length() + 1);
  sz += si.hostname.length() + 1;
  //printf("hostname: '%s', len: %d, sz: %d\n", si.hostname.c_str(), si.hostname.length(), sz);
  memcpy((void*)( (uint8_t*)buffer + 32 + sz ), (const void*)si.systemId.c_str(),           si.systemId.length() + 1);
  sz += si.systemId.length() + 1;
  //printf("systemId: '%s', len: %d, sz: %d\n", si.systemId.c_str(), si.systemId.length(), sz);
  memcpy((void*)( (uint8_t*)buffer + 32 + sz ), (const void*)si.receiptSchemeName.c_str(),  si.receiptSchemeName.length() + 1);
  //printf("receiptSchemeName: '%s', len: %d, sz: %d\n", si.receiptSchemeName.c_str(), si.receiptSchemeName.length(), sz);

   }catch(...){
       return 0;
   }

  return (void*)buffer;
}
bool SmeUpdateCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 39 || !buffer)
      return false;

  try {

  uint8_t val;

  uint32_t value32;

  memcpy((void*)&si.typeOfNumber,       (const void*)buffer, 1);
  memcpy((void*)&si.numberingPlan,      (const void*)((uint8_t*)buffer + 1), 1);
  memcpy((void*)&si.interfaceVersion,   (const void*)((uint8_t*)buffer + 2), 1);

  //printf("typeOfNumber: %d, numberingPlan: %d, interfaceVersion: %d\n", si.typeOfNumber, si.numberingPlan, si.interfaceVersion);

  memcpy((void*)&value32,               (const void*)((uint8_t*)buffer + 3), 4);
  si.port = ntohl(value32);
  memcpy((void*)&value32,           (const void*)((uint8_t*)buffer + 7), 4);
  si.priority = ntohl(value32);
  memcpy((void*)&value32,              (const void*)((uint8_t*)buffer + 11), 4);
  si.SME_N = ntohl(value32);
  memcpy((void*)&value32,            (const void*)((uint8_t*)buffer + 15), 4);
  si.timeout = ntohl(value32);
  memcpy((void*)&value32,          (const void*)((uint8_t*)buffer + 19), 4);
  si.proclimit = ntohl(value32);
  memcpy((void*)&value32,         (const void*)((uint8_t*)buffer + 23), 4);
  si.schedlimit = ntohl(value32);
  memcpy((void*)&value32,         (const void*)((uint8_t*)buffer + 27), 4);
  si.providerId = ntohl(value32);

  /*printf("port: %d, priority: %d, SME_N: %d, timeout: %d\nproclimit: %d, schedlimit: %d, providerId: %d\n", si.port,
            si.priority, si.SME_N, si.timeout, si.proclimit, si.schedlimit, si.providerId);*/

  memcpy((void*)&val,                   (const void*)((uint8_t*)buffer + 31), 1);


#define BIT(x) (((uint8_t)1)<<((uint8_t)x))

  si.disabled =     val & (uint8_t)1;
  si.wantAlias =    val & BIT(1);
  si.forceDC =      val & BIT(2);
  si.internal =     val & BIT(3);

  /*if(si.disabled)
      printf("disabled ok\n");
  if(si.wantAlias)
      printf("wantAlias ok\n");
  if(si.forceDC)
      printf("forceDC ok\n");
  if(si.internal)
      printf("internal ok\n");*/

#undef BIT

  val = val >> (uint8_t)4;
  //printf("bindMode: ");
  switch(val){
  case 0:
      si.bindMode = smsc::smeman::smeRX;
      //printf("smeRX\n");
  case 1:
      si.bindMode = smsc::smeman::smeTX;
      //printf("smeTX\n");
  case 2:
      si.bindMode = smsc::smeman::smeTRX;
      //printf("smeTRX\n");
  }

  int sz = 0;
  si.rangeOfAddress =       (char*)(   (uint8_t*)buffer + 32   );           sz += si.rangeOfAddress.length() + 1;   if (32 + sz >= len) return false;
  //printf("rangeOfAddress: '%s', len: %d, sz: %d\n", si.rangeOfAddress.c_str(), si.rangeOfAddress.length(), sz);
  si.systemType =           (char*)(   (uint8_t*)buffer + 32 + sz   );      sz += si.systemType.length() + 1;       if (32 + sz >= len) return false;
  //printf("systemType: '%s', len: %d, sz: %d\n", si.systemType.c_str(), si.systemType.length(), sz);
  si.password =             (char*)(   (uint8_t*)buffer + 32 + sz   );      sz += si.password.length() + 1;         if (32 + sz >= len) return false;
  //printf("password: '%s', len: %d, sz: %d\n", si.password.c_str(), si.password.length(), sz);
  si.hostname =             (char*)(   (uint8_t*)buffer + 32 + sz   );      sz += si.hostname.length() + 1;         if (32 + sz >= len) return false;
  //printf("hostname: '%s', len: %d, sz: %d\n", si.hostname.c_str(), si.hostname.length(), sz);
  si.systemId =             (char*)(   (uint8_t*)buffer + 32 + sz   );      sz += si.systemId.length() + 1;         if (32 + sz >= len) return false;
  //printf("systemId: '%s', len: %d, sz: %d\n", si.systemId.c_str(), si.systemId.length(), sz);
  si.receiptSchemeName =    (char*)(   (uint8_t*)buffer + 32 + sz   );
  //printf("receiptSchemeName: '%s', len: %d, sz: %d\n", si.receiptSchemeName.c_str(), si.receiptSchemeName.length(), sz);

  }catch(...){
      return false;
  }

  return true;
}

//========== aclRemove ==========================

AclRemoveCommand::AclRemoveCommand(smsc::acls::AclIdent id)
     : Command(ACLREMOVE_CMD),
    aclId(id)
{
}

void AclRemoveCommand::getArgs(smsc::acls::AclIdent &id) const
{
    id = aclId;
}

void* AclRemoveCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 4;

    buffer = new uint8_t[len];

    //printf("aclId: %d\n", aclId);

    uint32_t value32;
    value32 = htonl(aclId);
    memcpy((void*)buffer, (const void*)&value32, 4);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool AclRemoveCommand::deserialize(void *buffer, uint32_t len)
{
    if(len != 4 || !buffer)
        return false;

    try {

    uint32_t value32;

    memcpy((void*)&value32, (const void*)buffer, 4);
    aclId = ntohl(value32);

    //printf("aclId: %d\n", aclId);

    }catch(...){
        return false;
    }

    return true;
}

//========== aclCreate ==========================

AclCreateCommand::AclCreateCommand(std::string name_, std::string desc_,
                            std::string type_, File::offset_type offset_, std::vector<std::string> phones_)
    : Command(ACLCREATE_CMD),
    name(name_),
    description(desc_),
    cache_type(type_),
    offset(offset_)
{
    phones.empty();
    for(std::vector<std::string>::iterator it = phones_.begin(); it != phones_.end(); ++it){
        phones.push_back(*it);
    }
}

void AclCreateCommand::getArgs(std::string &name_, std::string &desc_,
                            std::string &type_, bool &type_present_, File::offset_type &offset_, std::vector<std::string> &phones_) const
{
    name_ = name;
    desc_ = description;
    type_ = cache_type;
    type_present_ = cache_type_present;
    offset_ = offset;

    phones_.empty();
    for(std::vector<std::string>::const_iterator it = phones.begin(); it != phones.end(); ++it){
        phones_.push_back(*it);
    }
}

void* AclCreateCommand::serialize(uint32_t &len)
{

    uint8_t * buffer = 0;

    try {

    len = 8 + name.length() + description.length() +
            cache_type.length() + 3;

    for(std::vector<std::string>::iterator it = phones.begin(); it != phones.end(); ++it){
        len += it->length() + 1;
    }

    buffer = new uint8_t[len];

    int sz = 0;
    memcpy((void*)buffer,                       (const void*)name.c_str(),              name.length() + 1);
    //printf("name: '%s', len: %d\n", name.c_str(), name.length());
    sz += name.length() + 1;
    memcpy((void*)((char*)buffer + sz),         (const void*)description.c_str(),       description.length() + 1);
    //printf("description: '%s', len: %d\n", description.c_str(), description.length());
    sz += description.length() + 1;
    memcpy((void*)((char*)buffer + sz),         (const void*)cache_type.c_str(),        cache_type.length() + 1);
    //printf("cache_type: '%s', len: %d\n", cache_type.c_str(), cache_type.length());
    sz += cache_type.length() + 1;

    //================== Puts offset in biffer =================

    uint64_t tmp = offset;
    uint64_t val64;

    unsigned char *ptr=(unsigned char *)&val64;
    ptr[0]=(unsigned char)(tmp>>56);
    ptr[1]=(unsigned char)(tmp>>48)&0xFF;
    ptr[2]=(unsigned char)(tmp>>40)&0xFF;
    ptr[3]=(unsigned char)(tmp>>32)&0xFF;
    ptr[4]=(unsigned char)(tmp>>24)&0xFF;
    ptr[5]=(unsigned char)(tmp>>16)&0xFF;
    ptr[6]=(unsigned char)(tmp>>8)&0xFF;
    ptr[7]=(unsigned char)(tmp&0xFF);

    memcpy((void*)((char*)buffer + sz),         (const void*)&val64,        8);
    sz += 8;

    //================== Puts phones in buffer =================

    for(std::vector<std::string>::iterator it = phones.begin(); it != phones.end(); ++it){
        memcpy((void*)((char*)buffer + sz),     (const void*)it->c_str(),       it->length() + 1);
        sz += it->length() + 1;
        //printf("phone: '%s', len: %d, sz: %d\n", it->c_str(), it->length(), sz);
    }

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool AclCreateCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 11 || !buffer)
        return false;

    try {

    int sz = 0;
    name =          (char*)buffer;              sz += name.length() + 1;            if (sz >= len) return false;
    //printf("name: '%s'\n", name.c_str());
    description =   (char*)buffer + sz;         sz += description.length() + 1;     if (sz >= len) return false;
    //printf("description: '%s'\n", description.c_str());
    cache_type =    (char*)buffer + sz;         sz += cache_type.length() + 1;      if (sz >= len) return false;
    //printf("cache_type: '%s'\n", cache_type.c_str());

    cache_type_present = cache_type.length();

    //================== Gets offset from buffer ======================

    uint64_t val64;
    memcpy((void*)&val64,       (const void*)( (uint8_t*)buffer + sz ), 8 );
    sz += 8;

    uint64_t tmp=0;
    memset(&tmp, 0, 8);
    unsigned char *ptr=(unsigned char *)&val64;
    tmp = (uint64_t)( ptr[0] ) << 56;
    tmp += (uint64_t)( ptr[1] ) << 48;
    tmp += (uint64_t)( ptr[2] ) << 40;
    tmp += (uint64_t)( ptr[3] ) << 32;
    tmp += (uint64_t)( ptr[4] ) << 24;
    tmp += (uint64_t)( ptr[5] ) << 16;
    tmp += (uint64_t)( ptr[6] ) << 8;
    tmp += (uint64_t)( ptr[7] );

    offset = tmp;

    phones.swap( std::vector<std::string>() );

    while(sz < len){
        std::string phone = (char*)buffer + sz;
        phones.push_back(phone);
        //printf("phone: '%s'\n", phone.c_str());
        sz += phone.length() + 1;
    }

    if(len != sz)
        return false;

    }catch(...){
        return false;
    }

    return true;
}

//========== aclUpdateInfo ==========================

AclUpdateInfoCommand::AclUpdateInfoCommand(smsc::acls::AclIdent id, std::string name_, std::string desc_,
                                    std::string type_)
    : Command(ACLUPDATEINFO_CMD),
    aclId(id),
    name(name_),
    description(desc_),
    cache_type(type_)
{
}

void AclUpdateInfoCommand::getArgs(smsc::acls::AclIdent &id, std::string &name_, std::string &desc_,
                                    std::string &type_) const
{
    id = aclId;
    name_ = name;
    desc_ = description;
    type_ = cache_type;
}

void* AclUpdateInfoCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 7 + name.length() + description.length() +
                cache_type.length();

    buffer = new uint8_t[len];

    uint32_t value32;

    //printf("aclId: %d\n", aclId);
    value32 = htonl(aclId);
    memcpy((void*)buffer, (const void*)&value32, 4);

    int sz = 4;
    memcpy((void*)( (uint8_t*)buffer + sz ), (const void*)name.c_str(),         name.length() + 1);
    sz += name.length() + 1;
    //printf("name: '%s', len: %d, sz: %d\n", name.c_str(), name.length(), sz);
    memcpy((void*)( (uint8_t*)buffer + sz ), (const void*)description.c_str(),  description.length() + 1);
    sz += description.length() + 1;
    //printf("description: '%s', len: %d, sz: %d\n", description.c_str(), description.length(), sz);
    memcpy((void*)( (uint8_t*)buffer + sz ), (const void*)cache_type.c_str(),   cache_type.length() + 1);
    sz += cache_type.length() + 1;
    //printf("cache_type: '%s', len: %d, sz: %d\n", cache_type.c_str(), cache_type.length(), sz);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool AclUpdateInfoCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 7 || !buffer)
        return false;

    try {

    uint32_t value32;
    memcpy((void*)&value32, (const void*)buffer, 4);
    aclId = ntohl(value32);
    //printf("aclId: %d\n", aclId);

    int sz = 4;
    name =          (char*)buffer + sz;   sz += name.length() + 1;          if(sz >= len) return false;
    //printf("name: '%s', len: %d, sz: %d\n", name.c_str(), name.length(), sz);
    description =   (char*)buffer + sz;   sz += description.length() + 1;   if(sz >= len) return false;
    //printf("description: '%s', len: %d, sz: %d\n", description.c_str(), description.length(), sz);
    cache_type =    (char*)buffer + sz;   sz += cache_type.length() + 1;
    //printf("cache_type: '%s', len: %d, sz: %d\n", cache_type.c_str(), cache_type.length(), sz);

    if(sz != len)
        return false;

    }catch(...){
        return false;
    }

    return true;
}

//========== aclRemoveAddresses ==========================

AclRemoveAddressesCommand::AclRemoveAddressesCommand(smsc::acls::AclIdent id, std::vector<std::string> addresses_)
    : Command(ACLREMOVEADDRESSES_CMD),
    aclId(id),
    addresses(addresses_)
{
}

void AclRemoveAddressesCommand::getArgs(smsc::acls::AclIdent &id, std::vector<std::string> &addresses_) const
{
    id = aclId;
    addresses_ = addresses;
}

void* AclRemoveAddressesCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 4;

    for(std::vector<std::string>::iterator it = addresses.begin(); it != addresses.end(); ++it){
        len += it->length() + 1;
    }

    buffer = new uint8_t[len];

    //printf("aclId: %d\n", aclId);
    uint32_t value32;
    value32 = htonl(aclId);
    memcpy((void*)buffer, (const void*)&value32, 4);

    int sz = 4;

    for(std::vector<std::string>::iterator it = addresses.begin(); it != addresses.end(); ++it){
        memcpy((void*)( (char*)buffer + sz ), (const void*)it->c_str(), it->length() + 1);
        sz += it->length() + 1;
        //printf("address: '%s', len: %d, sz: %d\n", it->c_str(), it->length(), sz);
    }

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool AclRemoveAddressesCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 5 || !buffer)
        return false;

    try {

    uint32_t value32;

    memcpy((void*)&value32, (const void*)buffer, 4);
    aclId = ntohl(value32);
    //printf("aclId: %d\n", aclId);

    int sz = 4;
    while(sz < len){
        std::string address = (char*)buffer + sz;     sz += address.length() + 1;
        addresses.push_back(address);
        //printf("address: '%s', len: %d, sz: %d\n", address.c_str(), address.length(), sz);
    }

    if(len != sz)
        return false;

    }catch(...){
        return false;
    }

    return true;
}

//========== AclAddAddresses ==========================

AclAddAddressesCommand::AclAddAddressesCommand(smsc::acls::AclIdent id, std::vector<std::string> addr)
    : Command(ACLADDADDRESSES_CMD),
    aclId(id)
{
    addresses.empty();
    for(std::vector<std::string>::iterator it = addr.begin(); it != addr.end(); ++it){
        addresses.push_back(*it);
    }
}

void AclAddAddressesCommand::getArgs(smsc::acls::AclIdent &id, std::vector<std::string> &addr) const
{
    id = aclId;
    addr.empty();
    for(std::vector<std::string>::const_iterator it = addresses.begin(); it != addresses.end(); ++it){
        addr.push_back(*it);
    }
}

void* AclAddAddressesCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 4;

    for(std::vector<std::string>::iterator it = addresses.begin(); it != addresses.end(); ++it){
        len += it->length();
    }

    buffer = new uint8_t[len];

    uint32_t value32;
    //printf("\naclId: %d\n", aclId);
    value32 = htonl(aclId);
    memcpy((void*)buffer, (const void*)&value32, 4);


    int sz = 4;

    for(std::vector<std::string>::iterator it = addresses.begin(); it != addresses.end(); ++it){
        memcpy((void*)( (char*)buffer + sz ), (const void*)it->c_str(), it->length() + 1);
        //printf("address: '%s'\n", it->c_str());
        sz += it->length() + 1;
    }

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool AclAddAddressesCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 5 || !buffer)
        return false;

    try {

    uint32_t value32;
    memcpy((void*)&value32, (const void*)buffer, 4);
    aclId = ntohl(value32);
    //printf("\naclId: %d\n", aclId);

    int sz = 4;
    while(sz < len){
        std::string address = (char*)buffer + sz;     sz += address.length() + 1;
        addresses.push_back(address);
        //printf("address: '%s'\n", address.c_str());
    }

    }catch(...){
        return false;
    }

    return true;
}

//========== prcAddPrincipal ==========================

PrcAddPrincipalCommand::PrcAddPrincipalCommand(int maxLists_, int maxElements_, File::offset_type offset_, std::string address_)
    : Command(PRCADDPRINCIPAL_CMD),
    maxLists(maxLists_),
    maxElements(maxElements_),
    address(address_),
    offset(offset_)
{
    //printf("constructor, offset: %lld\n", offset);
}

void PrcAddPrincipalCommand::getArgs(int &maxLists_, int &maxElements_, File::offset_type &offset_, std::string &address_) const
{
    maxLists_ = maxLists;
    maxElements_ = maxElements;
    address_ = address;
    offset_ = offset;
}

void* PrcAddPrincipalCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 17 + address.length();

    buffer = new uint8_t[len];

    uint32_t value32;

    //printf("maxLists: %d\n", maxLists);
    value32 = htonl(maxLists);
    memcpy((void*)buffer, (const void*)&value32, 4);
    //printf("maxElements: %d\n", maxElements);
    value32 = htonl(maxElements);
    memcpy((void*)( (uint8_t*)buffer + 4), (const void*)&value32, 4);

    //============= Puts offset =================================

    //printf("offset: %lld\n", offset);

    uint64_t val64 = offset;
    uint64_t tmp = val64;;
    unsigned char *ptr=(unsigned char *)&val64;
    ptr[0]=(unsigned char)(tmp>>56);
    ptr[1]=(unsigned char)(tmp>>48)&0xFF;
    ptr[2]=(unsigned char)(tmp>>40)&0xFF;
    ptr[3]=(unsigned char)(tmp>>32)&0xFF;
    ptr[4]=(unsigned char)(tmp>>24)&0xFF;
    ptr[5]=(unsigned char)(tmp>>16)&0xFF;
    ptr[6]=(unsigned char)(tmp>>8)&0xFF;
    ptr[7]=(unsigned char)(tmp&0xFF);

    memcpy((void*)(buffer + 8),        (const void*)&val64, 8);

    //================= Puts strings params =======================

    //printf("address: '%s', len: %d\n", address.c_str(), address.length());
    memcpy((void*)( (uint8_t*)buffer + 16), (const void*)address.c_str(), address.length() + 1);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool PrcAddPrincipalCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 17 || !buffer)
        return false;

    try {

    uint32_t value32;

    memcpy((void*)&value32,     (const void*)buffer,                    4);
    maxLists = ntohl(value32);
    //printf("maxLists: %d\n", maxLists);
    memcpy((void*)&value32, (const void*)( (uint8_t*)buffer + 4 ),  4);
    maxElements = ntohl(value32);
    //printf("maxElements: %d\n", maxElements);

    //============= Gets offset ======================

    uint64_t val64;
    memcpy((void*)&val64,       (const void*)( (uint8_t*)   buffer + 8 ), 8);

    uint64_t tmp=0;
    memset(&tmp, 0, 8);
    unsigned char *ptr=(unsigned char *)&val64;
    tmp = (uint64_t)( ptr[0] ) << 56;
    tmp += (uint64_t)( ptr[1] ) << 48;
    tmp += (uint64_t)( ptr[2] ) << 40;
    tmp += (uint64_t)( ptr[3] ) << 32;
    tmp += (uint64_t)( ptr[4] ) << 24;
    tmp += (uint64_t)( ptr[5] ) << 16;
    tmp += (uint64_t)( ptr[6] ) << 8;
    tmp += (uint64_t)( ptr[7] );

    offset = tmp;

    //printf("offset: %lld\n", offset);

    //=============== Gets strings params =================

    address = (char*)buffer + 16;
    //printf("address: '%s', len: %d\n", address.c_str(), address.length());

    }catch(...){
        return false;
    }

    return true;
}

//========== prcDeletePrincipal ==========================

PrcDeletePrincipalCommand::PrcDeletePrincipalCommand(std::string address_)
    : Command(PRCDELETEPRINCIPAL_CMD),
    address(address_)
{
}

void PrcDeletePrincipalCommand::getArgs(std::string &address_) const
{
    address_ = address;
}

void* PrcDeletePrincipalCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = address.length() + 1;

    buffer = new uint8_t[len];

    //printf("address: '%s'\n", address.c_str());

    memcpy((void*)buffer, (const void*)address.c_str(), address.length() + 1);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool PrcDeletePrincipalCommand::deserialize(void *buffer, uint32_t len)
{
    if(!len || !buffer)
        return false;

    try {

    address = (char*)buffer;

    //printf("address: '%s'\n", address.c_str());

    }catch(...){
        return false;
    }

    return true;
}

//========== PrcAlterPrincipal ==========================

PrcAlterPrincipalCommand::PrcAlterPrincipalCommand(int maxLists_, int maxElements_, std::string addresses_)
    : Command(PRCALTERPRINCIPAL_CMD),
    maxLists(maxLists_),
    maxElements(maxElements_),
    addresses(addresses_)
{
}

void PrcAlterPrincipalCommand::getArgs(int &maxLists_, int &maxElements_, std::string &addresses_) const
{
    maxLists_ = maxLists;
    maxElements_ = maxElements;
    addresses_ = addresses;
}

void* PrcAlterPrincipalCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 9 + addresses.length();

    buffer = new uint8_t[len];

    uint32_t value32;

    //printf("maxLists: %d\n", maxLists);
    value32 = htonl(maxLists);
    memcpy((void*)buffer, (const void*)&value32, 4);
    //printf("maxElements: %d\n", maxElements);
    value32 = htonl(maxElements);
    memcpy((void*)( (uint8_t*)buffer + 4), (const void*)&value32, 4);
    memcpy((void*)( (uint8_t*)buffer + 8), (const void*)addresses.c_str(), addresses.length() + 1);
    //printf("addresses: '%s', len: %d\n", addresses.c_str(), addresses.length());

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool PrcAlterPrincipalCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 9 || !buffer)
        return false;

    try {

    uint32_t value32;

    memcpy((void*)&value32,    (const void*)buffer,                 4);
    maxLists = ntohl(value32);
    //printf("maxLists: %d\n", maxLists);
    memcpy((void*)&value32, (const void*)( (uint8_t*)buffer + 4 ),  4);
    maxElements = ntohl(value32);
    //printf("maxElements: %d\n", maxElements);
    addresses = (char*)buffer + 8;
    //printf("addresses: '%s', len: %d\n", addresses.c_str(), addresses.length());

    }catch(...){
        return false;
    }

    return true;
}

//========== memAddMember ==========================

MemAddMemberCommand::MemAddMemberCommand(File::offset_type offset_, std::string dlname_, std::string address_)
    : Command(MEMADDMEMBER_CMD),
    dlname(dlname_),
    address(address_),
    offset(offset_)
{
}

void MemAddMemberCommand::getArgs(File::offset_type &offset_, std::string &dlname_, std::string &address_) const
{
    dlname_ = dlname;
    address_ = address;
    offset_ = offset;
}

void* MemAddMemberCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 10 + dlname.length() + address.length();

    buffer = new uint8_t[len];

    //============= Puts offset =================================

    uint64_t val64 = offset;
    uint64_t tmp = val64;;
    unsigned char *ptr=(unsigned char *)&val64;
    ptr[0]=(unsigned char)(tmp>>56);
    ptr[1]=(unsigned char)(tmp>>48)&0xFF;
    ptr[2]=(unsigned char)(tmp>>40)&0xFF;
    ptr[3]=(unsigned char)(tmp>>32)&0xFF;
    ptr[4]=(unsigned char)(tmp>>24)&0xFF;
    ptr[5]=(unsigned char)(tmp>>16)&0xFF;
    ptr[6]=(unsigned char)(tmp>>8)&0xFF;
    ptr[7]=(unsigned char)(tmp&0xFF);

    memcpy((void*)(buffer),        (const void*)&val64, 8);

    //============== Puts strings params ========================

    int sz = 8;
    memcpy((void*)buffer,                       (const void*)dlname.c_str(),    dlname.length() + 1);
    sz += dlname.length() + 1;
    //printf("dlname: '%s', len: %d, sz: %d\n", dlname.c_str(), dlname.length(), sz);
    memcpy((void*)( (uint8_t*)buffer + sz ),    (const void*)address.c_str(),   address.length() + 1);
    //printf("address: '%s', len: %d\n", address.c_str(), address.length());

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool MemAddMemberCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 10 || !buffer)
        return false;

    try {

    //============= Gets offset ======================

    uint64_t val64;
    memcpy((void*)&val64,       (const void*)(buffer), 8);

    uint64_t tmp=0;
    memset(&tmp, 0, 8);
    unsigned char *ptr=(unsigned char *)&val64;
    tmp = (uint64_t)( ptr[0] ) << 56;
    tmp += (uint64_t)( ptr[1] ) << 48;
    tmp += (uint64_t)( ptr[2] ) << 40;
    tmp += (uint64_t)( ptr[3] ) << 32;
    tmp += (uint64_t)( ptr[4] ) << 24;
    tmp += (uint64_t)( ptr[5] ) << 16;
    tmp += (uint64_t)( ptr[6] ) << 8;
    tmp += (uint64_t)( ptr[7] );

    offset = tmp;

    //=============== Gets strings params ===============

    int sz = 8;
    dlname =    (char*)buffer;          sz += dlname.length() + 1;      if (sz >= len) return false;
    //printf("dlname: '%s', len: %d, sz: %d\n", dlname.c_str(), dlname.length(), sz);
    address =   (char*)buffer + sz;
    //printf("address: '%s', len: %d\n", address.c_str(), address.length());

    }catch(...){
        return false;
    }

    return true;
}

//========== memDeleteMember ==========================

MemDeleteMemberCommand::MemDeleteMemberCommand(std::string dlname_, std::string address_)
    : Command(MEMDELETEMEMBER_CMD),
    dlname(dlname_),
    address(address_)
{
}

void MemDeleteMemberCommand::getArgs(std::string &dlname_, std::string &address_) const
{
    dlname_ = dlname;
    address_ = address;
}

void* MemDeleteMemberCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 2 + dlname.length() + address.length();

    buffer = new uint8_t[len];

    int sz = 0;
    memcpy((void*)buffer,                       (const void*)dlname.c_str(),    dlname.length() + 1);
    sz += dlname.length() + 1;
    //printf("dlname: '%s', len: %d, sz: %d\n", dlname.c_str(), dlname.length(), sz);
    memcpy((void*)( (uint8_t*)buffer + sz ),    (const void*)address.c_str(),   address.length() + 1);
    //printf("address: '%s', len: %d\n", address.c_str(), address.length());

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool MemDeleteMemberCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 2 || !buffer)
        return false;

    try {

    int sz = 0;
    dlname =    (char*)buffer;          sz += dlname.length() + 1;      if (sz >= len) return false;
    //printf("dlname: '%s', len: %d, sz: %d\n", dlname.c_str(), dlname.length(), sz);
    address =   (char*)buffer + sz;
    //printf("address: '%s', len: %d\n", address.c_str(), address.length());

    }catch(...){
        return false;
    }

    return true;
}

//========== sbmAddSubmiter ==========================

SbmAddSubmiterCommand::SbmAddSubmiterCommand(File::offset_type offset_, std::string dlname_, std::string address_)
    : Command(SBMADDSUBMITER_CMD),
    dlname(dlname_),
    address(address_),
    offset(offset_)
{
}

void SbmAddSubmiterCommand::getArgs(File::offset_type &offset_, std::string &dlname_, std::string &address_) const
{
    dlname_ = dlname;
    address_ = address;
    offset_ = offset;
}

void* SbmAddSubmiterCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 10 + dlname.length() + address.length();

    buffer = new uint8_t[len];

    //============= Puts offset =================================

    uint64_t val64 = offset;
    uint64_t tmp = val64;;
    unsigned char *ptr=(unsigned char *)&val64;
    ptr[0]=(unsigned char)(tmp>>56);
    ptr[1]=(unsigned char)(tmp>>48)&0xFF;
    ptr[2]=(unsigned char)(tmp>>40)&0xFF;
    ptr[3]=(unsigned char)(tmp>>32)&0xFF;
    ptr[4]=(unsigned char)(tmp>>24)&0xFF;
    ptr[5]=(unsigned char)(tmp>>16)&0xFF;
    ptr[6]=(unsigned char)(tmp>>8)&0xFF;
    ptr[7]=(unsigned char)(tmp&0xFF);

    memcpy((void*)(buffer),        (const void*)&val64, 8);

    //============== Puts strings params ========================

    int sz = 8;
    memcpy((void*)buffer,                       (const void*)dlname.c_str(),    dlname.length() + 1);
    sz += dlname.length() + 1;
    //printf("dlname: '%s', len: %d, sz: %d\n", dlname.c_str(), dlname.length(), sz);
    memcpy((void*)( (uint8_t*)buffer + sz ),    (const void*)address.c_str(),   address.length() + 1);
    //printf("address: '%s', len: %d\n", address.c_str(), address.length());

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool SbmAddSubmiterCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 10 || !buffer)
        return false;

    try {

    //============= Gets offset ======================

    uint64_t val64;
    memcpy((void*)&val64,       (const void*)(buffer), 8);

    uint64_t tmp=0;
    memset(&tmp, 0, 8);
    unsigned char *ptr=(unsigned char *)&val64;
    tmp = (uint64_t)( ptr[0] ) << 56;
    tmp += (uint64_t)( ptr[1] ) << 48;
    tmp += (uint64_t)( ptr[2] ) << 40;
    tmp += (uint64_t)( ptr[3] ) << 32;
    tmp += (uint64_t)( ptr[4] ) << 24;
    tmp += (uint64_t)( ptr[5] ) << 16;
    tmp += (uint64_t)( ptr[6] ) << 8;
    tmp += (uint64_t)( ptr[7] );

    offset = tmp;

    //=============== Gets strings params =================

    int sz = 8;
    dlname =    (char*)buffer;          sz += dlname.length() + 1;      if (sz >= len) return false;
    //printf("dlname: '%s', len: %d, sz: %d\n", dlname.c_str(), dlname.length(), sz);
    address =   (char*)buffer + sz;
    //printf("address: '%s', len: %d\n", address.c_str(), address.length());

    }catch(...){
        return false;
    }

    return true;
}

//========== sbmDeleteSubmiter ==========================

SbmDeleteSubmiterCommand::SbmDeleteSubmiterCommand(std::string dlname_, std::string address_)
    : Command(SBMDELETESUBMITER_CMD),
    dlname(dlname_),
    address(address_)
{
}

void SbmDeleteSubmiterCommand::getArgs(std::string &dlname_, std::string &address_) const
{
    dlname_ = dlname;
    address_ = address;
}

void* SbmDeleteSubmiterCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 2 + dlname.length() + address.length();

    buffer = new uint8_t[len];

    int sz = 0;
    memcpy((void*)buffer,                       (const void*)dlname.c_str(),    dlname.length() + 1);
    sz += dlname.length() + 1;
    //printf("dlname: '%s', len: %d, sz: %d\n", dlname.c_str(), dlname.length(), sz);
    memcpy((void*)( (uint8_t*)buffer + sz ),    (const void*)address.c_str(),   address.length() + 1);
    //printf("address: '%s', len: %d\n", address.c_str(), address.length());

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool SbmDeleteSubmiterCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 2 || !buffer)
        return false;

    try {

    int sz = 0;
    dlname =    (char*)buffer;          sz += dlname.length() + 1;      if (sz >= len) return false;
    //printf("dlname: '%s', len: %d, sz: %d\n", dlname.c_str(), dlname.length(), sz);
    address =   (char*)buffer + sz;
    //printf("address: '%s', len: %d\n", address.c_str(), address.length());

    }catch(...){
        return false;
    }

    return true;
}

//========== dlAdd ==========================

DlAddCommand::DlAddCommand(int maxElements_, std::string dlname_, std::string owner_, File::offset_type offset1_, File::offset_type offset2_)
    : Command(DLADD_CMD),
    maxElements(maxElements_),
    dlname(dlname_),
    owner(owner_),
    offset1(offset1_),
    offset2(offset2_)
{
}

void DlAddCommand::getArgs(int &maxElements_, File::offset_type &offset1_, File::offset_type &offset2_, std::string &dlname_, std::string &owner_) const
{
    maxElements_ = maxElements;
    dlname_ = dlname;
    owner_ = owner;
    offset1_ = offset1;
    offset2_ = offset2;
}

void* DlAddCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 22 + dlname.length() + owner.length();

    buffer = new uint8_t[len];

    uint32_t value32;

    //printf("maxElements: %d\n", maxElements);
    value32 = htonl(maxElements);
    memcpy((void*)buffer, (const void*)&value32, 4);
    int sz = 4;

    //============= Puts offset =================================

    uint64_t val64 = offset1;
    uint64_t tmp = val64;
    unsigned char *ptr=(unsigned char *)&val64;
    ptr[0]=(unsigned char)(tmp>>56);
    ptr[1]=(unsigned char)(tmp>>48)&0xFF;
    ptr[2]=(unsigned char)(tmp>>40)&0xFF;
    ptr[3]=(unsigned char)(tmp>>32)&0xFF;
    ptr[4]=(unsigned char)(tmp>>24)&0xFF;
    ptr[5]=(unsigned char)(tmp>>16)&0xFF;
    ptr[6]=(unsigned char)(tmp>>8)&0xFF;
    ptr[7]=(unsigned char)(tmp&0xFF);

    memcpy((void*)(buffer + sz),        (const void*)&val64, 8);
    sz += 8;

    val64 = offset2;
    tmp = val64;
    ptr=(unsigned char *)&val64;
    ptr[0]=(unsigned char)(tmp>>56);
    ptr[1]=(unsigned char)(tmp>>48)&0xFF;
    ptr[2]=(unsigned char)(tmp>>40)&0xFF;
    ptr[3]=(unsigned char)(tmp>>32)&0xFF;
    ptr[4]=(unsigned char)(tmp>>24)&0xFF;
    ptr[5]=(unsigned char)(tmp>>16)&0xFF;
    ptr[6]=(unsigned char)(tmp>>8)&0xFF;
    ptr[7]=(unsigned char)(tmp&0xFF);

    memcpy((void*)(buffer + sz),        (const void*)&val64, 8);
    sz += 8;

    //============= Puts strings params ===========================

    memcpy((void*)( (uint8_t*)buffer + sz ), (const void*)dlname.c_str(), dlname.length() +1);
    sz += dlname.length() + 1;
    //printf("dlname: '%s', len: %d, sz: %d\n", dlname.c_str(), dlname.length(), sz);
    memcpy((void*)( (uint8_t*)buffer + sz ), (const void*)owner.c_str(), owner.length() +1);
    //printf("owner: '%s', len: %d\n", owner.c_str(), owner.length());

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool DlAddCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 22 || !buffer)
        return false;

    try {

    uint32_t value32;

    memcpy((void*)&value32, (const void*)buffer, 4);
    maxElements = ntohl(value32);
    //printf("maxElements: %d\n", maxElements);

    int sz = 4;

    //================= Gtes offset ========================

    uint64_t val64;
    memcpy((void*)&val64,       (const void*)( (uint8_t*)buffer + sz ), 8);
    sz += 8;

    uint64_t tmp=0;
    memset(&tmp, 0, 8);
    unsigned char *ptr=(unsigned char *)&val64;
    tmp = (uint64_t)( ptr[0] ) << 56;
    tmp += (uint64_t)( ptr[1] ) << 48;
    tmp += (uint64_t)( ptr[2] ) << 40;
    tmp += (uint64_t)( ptr[3] ) << 32;
    tmp += (uint64_t)( ptr[4] ) << 24;
    tmp += (uint64_t)( ptr[5] ) << 16;
    tmp += (uint64_t)( ptr[6] ) << 8;
    tmp += (uint64_t)( ptr[7] );

    offset1 = tmp;

    memcpy((void*)&val64,       (const void*)( (uint8_t*)buffer + sz ), 8);
    sz += 8;

    tmp=0;
    memset(&tmp, 0, 8);
    ptr=(unsigned char *)&val64;
    tmp = (uint64_t)( ptr[0] ) << 56;
    tmp += (uint64_t)( ptr[1] ) << 48;
    tmp += (uint64_t)( ptr[2] ) << 40;
    tmp += (uint64_t)( ptr[3] ) << 32;
    tmp += (uint64_t)( ptr[4] ) << 24;
    tmp += (uint64_t)( ptr[5] ) << 16;
    tmp += (uint64_t)( ptr[6] ) << 8;
    tmp += (uint64_t)( ptr[7] );

    offset2 = tmp;

    //================= Gtes strings params ================

    dlname =    (char*)buffer + sz;     sz += dlname.length() + 1;
    //printf("dlname: '%s', len: %d, sz: %d\n", dlname.c_str(), dlname.length(), sz);
    owner =     (char*)buffer + sz;
    //printf("owner: '%s', len: %d\n", owner.c_str(), owner.length());

    }catch(...){
        return false;
    }

    return true;
}

//========== dlDelete ==========================

DlDeleteCommand::DlDeleteCommand(std::string dlname_)
    : Command(DLDELETE_CMD),
    dlname(dlname_)
{
}

void DlDeleteCommand::getArgs(std::string &dlname_) const
{
    dlname_ = dlname;
}

void* DlDeleteCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = dlname.length() + 1;

    buffer = new uint8_t[len];

    //printf("dlname: '%s', len: %d\n", dlname.c_str(), dlname.length());
    memcpy((void*)buffer, (const void*)dlname.c_str(), dlname.length() + 1);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool DlDeleteCommand::deserialize(void *buffer, uint32_t len)
{
    if(!len || !buffer)
        return false;

    try {

    dlname = (char*)buffer;
    //printf("dlname: '%s', len: %d\n", dlname.c_str(), dlname.length());

    }catch(...){
        return false;
    }

    return true;
}

//========== DlAlter ==========================

DlAlterCommand::DlAlterCommand(int maxElements_, std::string dlname_)
    : Command(DLALTER_CMD),
    maxElements(maxElements_),
    dlname(dlname_)
{
}

void DlAlterCommand::getArgs(int &maxElements_, std::string &dlname_) const
{
    maxElements_ = maxElements;
    dlname_ = dlname;
}

void* DlAlterCommand::serialize(uint32_t &len)
{

    uint8_t *buffer = 0;

    try {

    len = 5 + dlname.length();

    buffer = new uint8_t[len];

    uint32_t value32;

    //printf("maxElements: %d\n", maxElements);
    value32 = htonl(maxElements);
    memcpy((void*)buffer, (const void*)&value32, 4);
    //printf("dlname: '%s', len: %d\n", dlname.c_str(), dlname.length());
    memcpy((void*)( (uint8_t*)buffer + 4 ), (const void*)dlname.c_str(), dlname.length() + 1);

    }catch(...){
        return 0;
    }

    return (void*)buffer;
}
bool DlAlterCommand::deserialize(void *buffer, uint32_t len)
{
    if(len < 5 || !buffer)
        return false;

    try {

    uint32_t value32;

    memcpy((void*)&value32, (const void*)buffer, 4);
    maxElements = ntohl(value32);
    //printf("maxElements: %d\n", maxElements);
    dlname = (char*)buffer + 4;
    //printf("dlname: '%s', len: %d\n", dlname.c_str(), dlname.length());

    }catch(...){
        return false;
    }

    return true;
}

//========== GetRole =======================

void* GetRoleCommand::serialize(uint32_t &len)
{
    len = 4;

    uint8_t *buffer = new uint8_t[4];

    uint32_t val;
    val = htonl(role);
    memcpy((void*)buffer, (const void*)&val, 4);

    return buffer;
}
bool GetRoleCommand::deserialize(void *buffer, uint32_t len)
{
    if(len != 4 || !buffer)
        return false;

    uint32_t val;
    memcpy((void*)&val, (const void*)buffer, 4);

    role = ntohl(val);

    return true;
}



}}
