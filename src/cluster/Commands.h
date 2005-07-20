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
        SAMPLE_CMD = 1,
        SSS
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
        static Command* create(void* buffer, uint32_t len);
        
        virtual ~Command() {}
        
        inline CommandType getType() {
            return type;
        }

        virtual void* serialize(uint32_t& len) = 0;
        virtual bool deserialize(void* buffer, uint32_t len) = 0;
    };

    // #######################  Particular commands declarations #######################  
    
    class SampleCommand : public Command
    {
    private:
        
        int sampleField;

    public:
        
        SampleCommand() : Command(SAMPLE_CMD) {};
        SampleCommand(int sf) : Command(SAMPLE_CMD), sampleField(sf) {};

        virtual ~SampleCommand() {};

        /**
         * TODO: Implement
         * WARN: add commandType to output buffer
         */
        virtual void* serialize(uint32_t &len);
        /**
         * TODO: Implement
         * WARN: no commandType in input buffer !
         */
        virtual bool deserialize(void *buffer, uint32_t len);
    };

    class SSSCommand : public Command
    {
    public:
        
        SSSCommand() : Command(SSS) {};
        virtual ~SSSCommand() {};

        virtual void* serialize(uint32_t &len);
        virtual bool deserialize(void *buffer, uint32_t len);
    };
    // TODO: Add more commands ...
}}

#endif // __SMSC_CLUSTER_COMMANDS__

