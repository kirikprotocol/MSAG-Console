
#include "TaskProcessor.h"
#include <exception>

namespace smsc { namespace infosme 
{

Hash<TaskFactory *>*  TaskFactory::registry = 0;

/* ---------------------------- PriorityContainer ---------------------------- */

TaskContainer::~TaskContainer()
{
    MutexGuard guard(tasksLock);
    
    char* key = 0; Task* task = 0; tasksByName.First();
    while (tasksByName.Next(key, task))
        if (task) { task->endProcess(); delete task; }
}
bool TaskContainer::addTask(Task* task)
{
    MutexGuard guard(tasksLock);

    const char* task_name = task->getName().c_str();
    if (!task_name || task_name[0] == '\0' || tasksByName.Exists(task_name)) return false;
    tasksByName.Insert(task_name, task);
    TaskHolder  holder(task);
    tasksByPriority.insert(holder);
    prioritySum += task->getPriority();
    return true;
}
bool TaskContainer::removeTask(std::string taskName)
{
    MutexGuard guard(tasksLock);
    const char* task_name = taskName.c_str();
    if (!task_name || task_name[0] == '\0' || !tasksByName.Exists(task_name)) return false;
    Task* task = tasksByName.Get(task_name);
    if (!task) return false;
    tasksByName.Delete(task_name);
    TaskHolder  holder(task);
    TaskSet::iterator it = tasksByPriority.find(holder);
    if (it != tasksByPriority.end()) {
        tasksByPriority.erase(it); // TODO analise and reimplement it !!!
        prioritySum -= task->getPriority();
    }
    return true;
}
Task* TaskContainer::getTask(std::string taskName)
{
    MutexGuard guard(tasksLock);
    const char* task_name = taskName.c_str();
    if (!task_name || task_name[0] == '\0' || !tasksByName.Exists(task_name)) return 0;
    return tasksByName.Get(task_name);
}
Task* TaskContainer::getNextTask()
{
    MutexGuard guard(tasksLock);
    if (prioritySum <= 0) return 0;
    
    int count = 0;
    int random = (rand()%prioritySum)+1;
    for (TaskSet::iterator it = tasksByPriority.begin(); it != tasksByPriority.end(); it++)
    {
        Task* task = it->task;
        if (!task) continue;
        count += task->getPriority();
        if (random <= count) return task;
    }
    return 0;
}

/* ---------------------------- TaskProcessor ---------------------------- */

TaskProcessor::TaskProcessor(ConfigView* config)
    : Thread(), logger(Logger::getCategory("smsc.infosme.TaskProcessor")), 
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
    
    // TODO: Init tasks here !!!
    
    logger.info("Tasks loaded.");

    logger.info("Loading task schedule ...");
    std::auto_ptr<ConfigView> schedulerCfgGuard(config->getSubConfig("Scheduler"));
    ConfigView* schedulerCfg = schedulerCfgGuard.get();
    scheduler.init(&manager, schedulerCfg);
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
    Task* task = container.getNextTask();
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

