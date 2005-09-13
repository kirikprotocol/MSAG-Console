#ifndef __SMSC_CLUSTER_INTERCONNECT_MANAGER__
#define __SMSC_CLUSTER_INTERCONNECT_MANAGER__

#include <string>

#include "Interconnect.h"
#include "CommandDispatcher.h"
#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"
#include "CommandReader.h"

namespace smsc { namespace cluster {

using smsc::core::network::Socket;

using smsc::core::synchronization::Mutex;

    class InterconnectManager : public Interconnect, public Thread
    {
    private:

        Role            role;
        std::string     inAddr;
        std::string     attachedInAddr;
        int             port;
        int             attachedPort;

        EventMonitor    commandsMonitor;
        Array<Command*>  commands;

        CommandDispatcher* dispatcher;

    protected:

        InterconnectManager(const std::string& inAddr_, const std::string& attachedInAddr_, int _port, int _attachedPort);
        virtual ~InterconnectManager();
        bool isStopping;
        Mutex stopLock;
        bool isStoped();
        bool isMaster();
        bool isSlave();
        bool isSingle();
        void flushCommands();

        void send(Command *command);
        uint32_t readRole();

        Socket socket;
        Socket attachedSocket;

        class ArgHandler{
        public:
            ArgHandler()
                : arg(0),
                  fun(0)
            {
            };
            ArgHandler(ChangeRoleHandler * fun_, void* arg_)
                : arg(arg_),
                  fun(fun_)
            {                     
            };
            void run(Role role)
            {
                fun(role, arg);
            };
        protected:
            void* arg;
            ChangeRoleHandler *fun;
        };

        Mutex handlersLock;
        Array<ArgHandler> handlers;
        CommandReader reader;

    public:

        static void init(const std::string& inAddr_, const std::string& attachedInAddr_, int _port, int _attachedPort);
        static void shutdown();

        virtual void sendCommand(Command* command);
        virtual void addListener(CommandType type, CommandListener* listener);
        virtual void activate();
        virtual void changeRole(Role role_);
        virtual void addChangeRoleHandler(ChangeRoleHandler * fun, void* arg);

        void Start();
        void Stop();

        virtual int Execute();
        virtual Role getRole(){return role;};
    };

    class FakeInterconnect : public Interconnect
    {
    public:
        virtual void sendCommand(Command* command){};
        virtual void addListener(CommandType type, CommandListener* listener){};
        virtual void activate(){};
        virtual Role getRole(){return SINGLE;};
        virtual void changeRole(Role role_){};
        virtual void addChangeRoleHandler(ChangeRoleHandler*, void*){};
        static void init()
        {
            if (!FakeInterconnect::instance) {
                FakeInterconnect::instance = new FakeInterconnect();
            }
        };
    };

}}

#endif // __SMSC_CLUSTER_INTERCONNECT_MANAGER__
