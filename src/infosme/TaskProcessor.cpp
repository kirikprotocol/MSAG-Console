
#include "TaskProcessor.h"

namespace smsc { namespace infosme 
{

Hash<TaskFactory *>*  TaskFactory::registry = 0;

TaskProcessor::TaskProcessor(ConfigView* config)
    : Thread(), bStarted(false)
{
    
    std::auto_ptr<ConfigView> providerConfig(config->getSubConfig("DataProvider"));
    provider.init(providerConfig.get());
}
TaskProcessor::~TaskProcessor()
{
    // TODO: implement it
}
void TaskProcessor::Start()
{
    // TODO: implement it
}
void TaskProcessor::Stop()
{
    // TODO: implement it
}
int TaskProcessor::Execute()
{
    // TODO: implement it
    return 0;
}

void TaskProcessor::MainLoop()
{
    // TODO: implement it
}

}}

