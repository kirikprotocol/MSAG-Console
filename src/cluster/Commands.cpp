
#include "Commands.h"

#include "core/buffers/IntHash.hpp"

namespace smsc { namespace cluster 
{
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
            int Exists(int type) { 
                return getInstance().Exists(type); 
            }
            int Insert(int type, CommandFactory* factory) {
                return getInstance().Insert(type, factory);
            }
            CommandFactory* Get(int type) {
                return getInstance().Get(type);
            }
        };
        static FakeRegistry factories;
        static factoriesInited;
        
        static void registerFactory(int type, CommandFactory* factory) {
            if (type && factory && !factories.Exists(type)) {
                factories.Insert(type, factory);
            }
        };
        static CommandFactory* getFactory(int type) {
            return (factories.Exists(type) ? factories.Get(type):0);
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
    
    CommandFactory::FakeRegistry CommandFactory::factories;
    CommandFactory::factoriesInited = false;

/* ################# Particular commands factories ################ */

    class SampleCommand1Factory : public CommandFactory
    {
    protected:
        virtual Command* create() { return new SampleCommand1(); };
    public:
        SampleCommand1Factory() : CommandFactory(SAMPLE_CMD1_T);
        virtual ~SampleCommand1Factory() {};
    };
    // TODO: Add more actual factories for particular commands


/* ####################### General routines ####################### */

Mutex initFactoriesLock;
void CommandFactory::initFactories()
{
    MutexGuard guard(initFactoriesLock);
    
    if (!CommandFactory::factoriesInited) 
    {
        static SampleCommand1Factory    _sampleCommand1Factory;

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
    if (command && !command->deserialize(buffer+sizeof(int16_t), len-sizeof(int16_t))) 
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


}}
