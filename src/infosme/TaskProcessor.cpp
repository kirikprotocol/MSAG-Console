
#include "TaskProcessor.h"

namespace smsc { namespace infosme 
{

Hash<TaskFactory *>*  TaskFactory::registry = 0;

TaskProcessor::TaskProcessor(ConfigView* config)
    : Thread(), logger(Logger::getCategory("smsc.infosme.TaskProcessor")), 
            bStarted(false), dsInternalName(0), dsInternal(0)
{
    logger.info("Loading ...");

    std::auto_ptr<ConfigView> providerCfgGuard(config->getSubConfig("DataProvider"));
    ConfigView* providerCfg = providerCfgGuard.get();
    provider.init(providerCfg);
    
    dsInternalName = providerCfg->getString("dsInternal", 
                                            "Internal DataSource driver name missed.", true);
    dsInternal = provider.getDataSource(dsInternalName);
    if (!dsInternal)
        throw ConfigException("Failed to obtail internal DataSource driver '%s'", dsInternalName);
    
    logger.info("Internal DataSource driver '%s' obtained.", dsInternalName);
    
    logger.info("Loading tasks ...");
    
    // TODO: Init tasks here !!!
    
    logger.info("Tasks loaded.");
    
    logger.info("Loading task schedule ...");
    std::auto_ptr<ConfigView> schedulerCfgGuard(config->getSubConfig("TaskScheduler"));
    ConfigView* schedulerCfg = schedulerCfgGuard.get();
    scheduler.init(schedulerCfg);
    logger.info("Task schedule loaded.");
    logger.info("Load success.");
}
TaskProcessor::~TaskProcessor()
{
    if (dsInternalName) delete dsInternalName;
    
    // TODO: implement task set stop & cleanup
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

