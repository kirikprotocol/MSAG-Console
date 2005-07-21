
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/IntHash.hpp"

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
            IntHash<CommandFactory *> getInstance() {
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
            if (type && factory && !factories.Exist(type)) {
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

        static Command* createCommand(CommandType type) {
            CommandFactory* factory = getFactory((int)type);
            return (factory) ? factory->create() : 0;
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

    //========== applySmscConfig =======================

    class ApplySmscConfigCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new ApplySmscConfigCommand(); };
    public:
        ApplySmscConfigCommandFactory() : CommandFactory(APPLYSMSCCONFIG_CMD) {};
        virtual ~ApplySmscConfigCommandFactory() {};
    };

    //========== applyServices =======================

    class ApplyServicesCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new ApplyServicesCommand(); };
    public:
        ApplyServicesCommandFactory() : CommandFactory(APPLYSERVICES_CMD) {};
        virtual ~ApplyServicesCommandFactory() {};
    };

    //========== applyLocalResource =======================

    class ApplyLocaleResourceCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new ApplyLocaleResourceCommand(); };
    public:
        ApplyLocaleResourceCommandFactory() : CommandFactory(APPLYLOCALRESOURCE_CMD) {};
        virtual ~ApplyLocaleResourceCommandFactory() {};
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
        MscBlockCommandFactory() : CommandFactory(MSCUNREGISTER_CMD) {};
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
        virtual Command* create() { return new PrcAlterPrincipalCommand(); };
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
        static ApplySmscConfigCommandFactory        _applySmscConfigCommandFactory;
        static ApplyServicesCommandFactory          _applyServicesCommandFactory;
        static ApplyLocaleResourceCommandFactory    _applyLocalResourceCommandFactory;

        static ProfileUpdateCommandFactory      _profileUpdateCommandFactory;
        static ProfileDeleteCommandFactory      _profileDeleteCommandFactory;

        static MscRegistrateCommandFactory      _mscRegistrateCommandFactory;
        static MscUnregisterCommandFactory      _mscUnregisterCommandFactory;
        static MscBlockCommandFactory           _mscBlockCommandFactory;
        static MscClearCommandFactory           _mscClearCommandFactory;

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
    
    return 0;
}
bool ApplyRoutesCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== applyAliases =======================

void* ApplyAliasesCommand::serialize(uint32_t &len)
{
    
    return 0;
}
bool ApplyAliasesCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== applyReschedule =======================

void* ApplyRescheduleCommand::serialize(uint32_t &len)
{
    return 0;
}
bool ApplyRescheduleCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== applySmscConfig =======================

void* ApplySmscConfigCommand::serialize(uint32_t &len)
{
    return 0;
}
bool ApplySmscConfigCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== applyServices =======================

void* ApplyServicesCommand::serialize(uint32_t &len)
{
    return 0;
}
bool ApplyServicesCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== applyLocalRoutes =======================

void* ApplyLocaleResourceCommand::serialize(uint32_t &len)
{
    return 0;
}
bool ApplyLocaleResourceCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== profileUpdate ==========================

void* ProfileUpdateCommand::serialize(uint32_t &len)
{
    return 0;
}
bool ProfileUpdateCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== profileDelete ==========================

void* ProfileDeleteCommand::serialize(uint32_t &len)
{
    return 0;
}
bool ProfileDeleteCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== mscRegistrate ==========================

void* MscRegistrateCommand::serialize(uint32_t &len)
{
    return 0;
}
bool MscRegistrateCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== mscUnregister ==========================

void* MscUnregisterCommand::serialize(uint32_t &len)
{
    return 0;
}
bool MscUnregisterCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== mscBlock ==========================

void* MscBlockCommand::serialize(uint32_t &len)
{
    return 0;
}
bool MscBlockCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== MscBlock ==========================

void* MscClearCommand::serialize(uint32_t &len)
{
    return 0;
}
bool MscClearCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== smeAdd ==========================

void* SmeAddCommand::serialize(uint32_t &len)
{
    return 0;
}
bool SmeAddCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== smeRemove ==========================

void* SmeRemoveCommand::serialize(uint32_t &len)
{
    return 0;
}
bool SmeRemoveCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== SmeUpdate ==========================

void* SmeUpdateCommand::serialize(uint32_t &len)
{
    return 0;
}
bool SmeUpdateCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== aclRemove ==========================

void* AclRemoveCommand::serialize(uint32_t &len)
{
    return 0;
}
bool AclRemoveCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== aclCreate ==========================

void* AclCreateCommand::serialize(uint32_t &len)
{
    return 0;
}
bool AclCreateCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== aclUpdateInfo ==========================

void* AclUpdateInfoCommand::serialize(uint32_t &len)
{
    return 0;
}
bool AclUpdateInfoCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== aclRemoveAddresses ==========================

void* AclRemoveAddressesCommand::serialize(uint32_t &len)
{
    return 0;
}
bool AclRemoveAddressesCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== AclAddAddresses ==========================

void* AclAddAddressesCommand::serialize(uint32_t &len)
{
    return 0;
}
bool AclAddAddressesCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== prcAddPrincipal ==========================

void* PrcAddPrincipalCommand::serialize(uint32_t &len)
{
    return 0;
}
bool PrcAddPrincipalCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== prcDeletePrincipal ==========================

void* PrcDeletePrincipalCommand::serialize(uint32_t &len)
{
    return 0;
}
bool PrcDeletePrincipalCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== PrcAlterPrincipal ==========================

void* PrcAlterPrincipalCommand::serialize(uint32_t &len)
{
    return 0;
}
bool PrcAlterPrincipalCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== memAddMember ==========================

void* MemAddMemberCommand::serialize(uint32_t &len)
{
    return 0;
}
bool MemAddMemberCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== memDeleteMember ==========================

void* MemDeleteMemberCommand::serialize(uint32_t &len)
{
    return 0;
}
bool MemDeleteMemberCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== sbmAddSubmiter ==========================

void* SbmAddSubmiterCommand::serialize(uint32_t &len)
{
    return 0;
}
bool SbmAddSubmiterCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== sbmDeleteSubmiter ==========================

void* SbmDeleteSubmiterCommand::serialize(uint32_t &len)
{
    return 0;
}
bool SbmDeleteSubmiterCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== dlAdd ==========================

void* DlAddCommand::serialize(uint32_t &len)
{
    return 0;
}
bool DlAddCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== dlDelete ==========================

void* DlDeleteCommand::serialize(uint32_t &len)
{
    return 0;
}
bool DlDeleteCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}

//========== DlAlter ==========================

void* DlAlterCommand::serialize(uint32_t &len)
{
    return 0;
}
bool DlAlterCommand::deserialize(void *buffer, uint32_t len)
{
    if(len || buffer)
        return false;
    
    return true;
}
    


}}
