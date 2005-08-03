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

    typedef enum {
        SINGLE = 0,
        MASTER = 1,
        SLAVE  = 2
    } Role;

    class InterconnectManager : public Interconnect, public Thread
    {
    private:
        
        Role            role;
        std::string     master_ip;
        std::string     slave_ip;
        int             port;

        static InterconnectManager* instance;

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

        void send(Command *command);
        //Socket socket;

    public:

        static void init(Role _role, const std::string& m_ip, const std::string& s_ip, int _port);
        static void shutdown();

        static Interconnect* getInstance(){
            return instance;
        };
        
        virtual void sendCommand(Command* command);
        virtual void addListener(CommandType type, CommandListener* listener);
        virtual void activate();

        void Start();
        void Stop();

        virtual int Execute();
    };

}}

#endif // __SMSC_CLUSTER_INTERCONNECT_MANAGER__

