
#include "InterconnectManager.h"

namespace smsc { namespace cluster 
{

static InterconnectManager::InterconnectManager* instance = 0;

InterconnectManager::InterconnectManager(Role _role, const std::string& m_ip, 
                                         const std::string& s_ip, int _port)
    : Interconnect(), Thread(), role(_role), master_ip(m_ip), slave_ip(s_ip), port(_port)
{

}
virtual ~InterconnectManager::InterconnectManager()
{

}

static void InterconnectManager::init(Role _role, const std::string& m_ip, 
                                      const std::string& s_ip, int _port)
{
    if (!InterconnectManager::instance) {
        InterconnectManager::instance = new InterconnectManager();
    }
}
static void InterconnectManager::shutdown()
{
    if (InterconnectManager::instance) {
        delete InterconnectManager::instance;
    }
}
    
void InterconnectManager::sendCommand(const Command& command)
{
    MutexGuard guard(commandsMonitor);
    // TODO: add command to queue 
    if (isStarted()) commandsMonitor.notify();
}
void InterconnectManager::addListener(CommandType type, CommandListener* listener)
{
}
void InterconnectManager::activate()
{

}

int InterconnectManager::Execute()
{
    // TODO: Send commands from commands queue (on commandsMonitor)
}


}}
