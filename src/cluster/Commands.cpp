
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
            if (type && factory && !factories.Exist(type)) {
                factories.Insert(type, factory);
            }
        };
        static CommandFactory* getFactory(int type) {
            return (factories.Exist(type) ? factories.Get(type):0);
        };

    protected:

        CommandFactory(int type) {
            CommandFactory::registerFactory(type, this);
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

        static Command* createCommand(int type) {
            CommandFactory* factory = getFactory(type);
            return (factory) ? factory->create() : 0;
        };

        virtual ~CommandFactory() {};
    };
    
    bool CommandFactory::factoriesInited = false;
    CommandFactory::FakeRegistry CommandFactory::factories;

/* ################# Particular commands factories ################ */

    class SampleCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new SampleCommand(); };
    public:
        SampleCommandFactory() : CommandFactory(SAMPLE_CMD) {};
        virtual ~SampleCommandFactory() {};
    };
    class SSSCommandFactory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new SSSCommand(); };
    public:
        SSSCommandFactory() : CommandFactory(SSS) {};
        virtual ~SSSCommandFactory() {};
    };
    // TODO: Add more actual factories for particular commands


/* ####################### General routines ####################### */

Mutex initFactoriesLock;
void CommandFactory::initFactories()
{
    MutexGuard guard(initFactoriesLock);
    
    if (!CommandFactory::factoriesInited) 
    {
        static SampleCommandFactory     _sampleCommandFactory;
        static SSSCommandFactory        _sssCommandFactory;

        // TODO: Create more actual factories for particular commands

        CommandFactory::factoriesInited = true;
    }
}

Command* Command::create(void* buffer, uint32_t len) // static
{
    CommandFactory::initFactories();

    if (len < sizeof(int16_t)) return 0;
    int16_t type = ntohs(*((int16_t *)buffer));
    
    Command* command = CommandFactory::createCommand(type);
    void* buff = ((int16_t*)buffer)+1;
    if (command && !command->deserialize(buff, len-sizeof(int16_t))) 
    {
        delete command;
        return 0;
    }
    return command;
}

/* ############### Particular commands declarations ############### */

void* SampleCommand::serialize(uint32_t &len)
{
    // TODO: Implement
}
bool SampleCommand::deserialize(void *buffer, uint32_t len)
{
    // TODO: Implement
}

void* SSSCommand::serialize(uint32_t &len)
{
    // TODO: Implement
}
bool SSSCommand::deserialize(void *buffer, uint32_t len)
{
    // TODO: Implement
}


}}
