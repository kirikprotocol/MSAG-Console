
#include "TaskProcessor.h"

namespace smsc { namespace infosme 
{

Hash<TaskFactory *>*  TaskFactory::registry = 0;

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
        tasksByPriority.erase(it);
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
    for (TaskSet::iterator it = tasksByPriority.begin(); it !=tasksByPriority.end(); it++)
    {
        Task* task = it->task;
        if (!task) continue;
        count += task->getPriority();
        if (random <= count) return task;
    }
    return 0;
}

TaskProcessor::TaskProcessor(ConfigView* config)
    : Thread(), logger(Logger::getCategory("smsc.infosme.TaskProcessor")), 
            bStarted(false), dsInternalName(0), dsInternal(0)
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
    // TODO: Init tasks here !!!

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

bool TaskProcessor::addTask(const Task* task)
{
    // TODO: implement it
    return false;
}
bool TaskProcessor::removeTask(std::string taskName)
{
    // TODO: implement it
    return false;
}

}}

