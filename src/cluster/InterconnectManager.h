#ifndef __SMSC_CLUSTER_INTERCONNECT_MANAGER__
#define __SMSC_CLUSTER_INTERCONNECT_MANAGER__

#include <string>

#include "Interconnect.h"
#include "CommandDispatcher.h"
#include "core/synchronization/Mutex.hpp"
//#include "core/network/Socket.hpp"

namespace smsc { namespace cluster {

//using smsc::core::network::Socket;

using smsc::core::synchronization::Mutex;

    class InterconnectManager : public Interconnect, public Thread
    {
    private:

        Role            role;
        std::string     master_ip;
        std::string     slave_ip;
        int             port;

        EventMonitor    commandsMonitor;
        Array<Command*>  commands;

        CommandDispatcher* dispatcher;

    protected:

        InterconnectManager(Role _role, const std::string& m_ip, const std::string& s_ip, int _port);
        virtual ~InterconnectManager();
        bool isStopping;
        Mutex stopLock;
        Mutex dispatcherLock;
        bool isStoped();
        bool isMaster();
        bool isSlave();
        bool isSingle();

        void send(Command *command);
        //Socket socket;

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

        Array<ArgHandler> handlers;

    public:

        static void init(Role _role, const std::string& m_ip, const std::string& s_ip, int _port);
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

}}

#endif // __SMSC_CLUSTER_INTERCONNECT_MANAGER__
