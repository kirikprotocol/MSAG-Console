
#include "TaskProcessor.h"
#include <exception>

namespace smsc { namespace infosme 
{

/* ---------------------------- PriorityContainer ---------------------------- */

TaskContainer::~TaskContainer()
{
    MutexGuard guard(tasksLock);
    
    char* key = 0; Task* task = 0; tasks.First();
    while (tasks.Next(key, task))
        if (task) task->finalize();
}
bool TaskContainer::addTask(Task* task)
{
    __require__(task);
    MutexGuard guard(tasksLock);

    const char* task_name = task->getName().c_str();
    if (!task_name || task_name[0] == '\0' || tasks.Exists(task_name)) return false;
    tasks.Insert(task_name, task);
    prioritySum += task->getPriority();
    return true;
}
bool TaskContainer::removeTask(std::string taskName)
{
    MutexGuard guard(tasksLock);
    
    const char* task_name = taskName.c_str();
    if (!task_name || task_name[0] == '\0' || !tasks.Exists(task_name)) return false;
    Task* task = tasks.Get(task_name);
    if (!task) return false;
    tasks.Delete(task_name);
    prioritySum -= task->getPriority();
    task->finalize();
    return true;
}
TaskGuard TaskContainer::getTask(std::string taskName)
{
    MutexGuard guard(tasksLock);
    
    const char* task_name = taskName.c_str();
    if (!task_name || task_name[0] == '\0' || !tasks.Exists(task_name)) return 0;
    Task* task = tasks.Get(task_name);
    return TaskGuard((task && !task->isFinalizing()) ? task:0);
}
TaskGuard TaskContainer::getNextTask()
{
    MutexGuard guard(tasksLock);
    
    if (prioritySum <= 0) return 0;
    
    int count = 0;
    int random = (rand()%prioritySum)+1;
    
    char* key = 0; Task* task = 0; tasks.First();
    while (tasks.Next(key, task))
        if (task) 
        { 
            count += task->getPriority();
            if (random <= count && !task->isFinalizing()) return TaskGuard(task);    
        }
    
    return TaskGuard(0);
}

/* ---------------------------- TaskProcessor ---------------------------- */

TaskProcessor::TaskProcessor(ConfigView* config)
    : TaskProcessorAdapter(), Thread(),
        logger(Logger::getCategory("smsc.infosme.TaskProcessor")), 
            bStarted(false), bNeedExit(false), dsInternalName(0), dsInternal(0)
{
    logger.info("Loading ...");

    std::auto_ptr<ConfigView> threadPoolCfgGuard(config->getSubConfig("ThreadPool"));
    ConfigView* threadPoolCfg = threadPoolCfgGuard.get();
    manager.init(threadPoolCfg); // loads up thread pool for tasks

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
    std::auto_ptr<ConfigView> tasksCfgGuard(config->getSubConfig("Tasks"));
    ConfigView* tasksCfg = tasksCfgGuard.get();
    switchTimeout = tasksCfg->getInt("switchTimeout");
    if (switchTimeout <= 0) 
        throw ConfigException("Task switch timeout should be positive");
    
    std::auto_ptr< std::set<std::string> > setGuard(tasksCfg->getShortSectionNames());
    std::set<std::string>* set = setGuard.get();
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        try
        {
            const char* taskName = (const char *)i->c_str();
            if (!taskName || taskName[0] == '\0')
                throw ConfigException("Task name empty or wasn't specified");
            logger.info("Loading task '%s' ...", taskName);
            
            std::auto_ptr<ConfigView> taskConfigGuard(tasksCfg->getSubConfig(taskName));
            ConfigView* taskConfig = taskConfigGuard.get();
            const char* dsId = taskConfig->getString("dsId");
            if (!dsId || dsId[0] == '\0')
                throw ConfigException("DataSource id for task '%s' empty or wasn't specified",
                                      taskName);
            DataSource* taskDs = provider.getDataSource(dsId);
            if (!taskDs)
                throw ConfigException("Failed to obtail DataSource driver '%s' for task '%s'", 
                                      dsId, taskName);
            
            if (!container.addTask(new Task(taskConfig, taskName, taskDs, dsInternal)))
                throw ConfigException("Failed to add task. Task '%s' already registered.",
                                      taskName);
        }
        catch (ConfigException& exc)
        {
            logger.error("Load of tasks failed ! Config exception: %s", exc.what());
            throw;
        }
    }
    logger.info("Tasks loaded.");

    logger.info("Loading task schedule ...");
    std::auto_ptr<ConfigView> schedulerCfgGuard(config->getSubConfig("Scheduler"));
    ConfigView* schedulerCfg = schedulerCfgGuard.get();
    scheduler.init(this, schedulerCfg);
    logger.info("Task schedule loaded.");

    logger.info("Load success.");
}
TaskProcessor::~TaskProcessor()
{
    this->Stop();

    // TODO: implement task set stop & cleanup
    if (dsInternalName) delete dsInternalName;
}
void TaskProcessor::Start()
{
    MutexGuard guard(startLock);
    
    if (!bStarted)
    {
        logger.info("Starting ...");
        bNeedExit = false;
        awake.Wait(0);
        Thread::Start();
        bStarted = true;
        logger.info("Started.");
    }
}
void TaskProcessor::Stop()
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        logger.info("Stopping ...");
        bNeedExit = true;
        awake.Signal();
        exited.Wait();
        bStarted = false;
        logger.info("Stoped.");
    }
}
int TaskProcessor::Execute()
{
    bool first = true;
    while (!bNeedExit)
    {
        awake.Wait((first) ? 100:switchTimeout);
        if (bNeedExit) break;
        try 
        {
            MainLoop(); first = false;
        } 
        catch (std::exception& exc) 
        {
            awake.Wait(0); first = true;
            logger.error("Exception occurred during tasks execution : %s", exc.what());
        }
    }
    exited.Signal();
    return 0;
}

void TaskProcessor::MainLoop()
{
    logger.info("Entering MainLoop");
    TaskGuard taskGuard = container.getNextTask(); 
    Task* task = taskGuard.get();
    if (!task) return;
    
    Message message;
    Connection* connection = dsInternal->getConnection();
    if (connection && task->getNextMessage(connection, message))
    {
        logger.debug("Sending message ...");
        // TODO: send sms here !!!
        logger.debug("Message sent.");
    }
}

}}

