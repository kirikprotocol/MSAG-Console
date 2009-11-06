#include "Task.h"
#include "logger/Logger.h"
#include "db/DataSource.h"
#include "DataProvider.h"
#include "TaskProcessor.h"
#include "TaskInfo.h"

namespace {
smsc::core::synchronization::Mutex createLogMutex;
}

namespace smsc {
namespace infosme2 {

using namespace smsc::util::config;

smsc::logger::Logger* Task::log_ = 0;

/*
Task* Task::createTask( const TaskProcessor&          taskProcessor,
                        const char*                   taskId,
                        ConfigView*                   taskConfig )
{
    if ( !log_ ) {
        MutexGuard mg(createLogMutex);
        if ( !log_ ) {
            log_ = smsc::logger::Logger::getInstance("is2.mktask");
        }
    }

    if ( !taskId || *taskId == '\0') {
        throw ConfigException("Task id empty or was not specified");
    }
    char* endptr;
    unsigned id = unsigned(strtoul(taskId,&endptr,10));
    if ( id == 0 || *endptr != '\0' ) {
        throw ConfigException("Task id %s cannot be converted to unsigned",taskId);
    }
    char buf[30];
    sprintf(buf,"%u",id);
    taskId = buf;
    smsc_log_info(log_,"loading task %u ...",id);

    // trying to load from a separate config
    std::auto_ptr< Config > separateConfig;
    std::auto_ptr< ConfigView > separateView;
    const std::string location = taskProcessor.storeLocation() + taskId;
    if ( smsc::core::buffers::File::Exists(location.c_str()) ) {
        const std::string fname = location + "/config.xml";
        if ( smsc::core::buffers::File::Exists(fname.c_str()) ) {
            separateConfig.reset(Config::createFromFile(fname.c_str()));
            separateView.reset(new ConfigView(*separateConfig.get()));
            smsc_log_debug(log_,"task %u is using a separate config '%s'",id,fname.c_str());
            taskConfig = separateView.get();
        }
    }
    if ( ! separateView.get() ) {
        // not loaded, using default config
        if ( !taskConfig ) {
            throw ConfigException("tasks config is not passed in");
        }
        separateView.reset( taskConfig->getSubConfig(taskId) );
        smsc_log_debug(log_,"task %u is using an embedded config",id);
        taskConfig = separateView.get();
    }

    bool delivery = false;
    try { delivery = taskConfig->getBool("delivery"); }
    catch ( ConfigException& e ) {}

    smsc::db::DataSource* taskDs = 0;
    if ( !delivery ) {
        const char* dsId = taskConfig->getString("dsId");
        if (!dsId || dsId[0] == '\0')
            throw ConfigException("DataSource id for task '%s' empty or wasn't specified",
                                  taskId);
        if ( !taskProcessor.getDataProvider() ) {
            throw ConfigException("DataSourceProvider is not provided");
        }
        taskDs = taskProcessor.getDataProvider()->getDataSource(dsId);
        if (!taskDs)
            throw ConfigException("Failed to obtail DataSource driver '%s' for task '%s'",
                                  dsId, taskId);
    }

    if ( ! smsc::core::buffers::File::Exists(location.c_str()) ) {
        // making a directory
        smsc::core::buffers::File::MkDir(location.c_str());
    }

    // creation of task info structure
    TaskInfo info(id,delivery);
    info.init( taskConfig );
    std::auto_ptr<Task> task(new Task(location,info,taskDs,taskProcessor.getFinalStateSaver()));
    smsc_log_info(log_,"task %u is loaded",id);
    return task.release();
}
 */


unsigned Task::stringToTaskId( const char* taskId )
{
    if ( !taskId || !*taskId ) throw ConfigException("taskId is null");
    char* endptr;
    unsigned ret = strtoul(taskId,&endptr,10);
    if ( *endptr != '\0') {
        throw ConfigException("taskId '%s' is not fully converted",taskId);
    } else if ( ret == 0 ) {
        throw ConfigException("taskId 0 is forbidden");
    }
    return ret;
}


bool Task::prefetchMessage( time_t now, int regionId )
{
    MutexGuard mg(lock_);

    if ( !isActive ) return false;
    if ( prefetched_ ) {
        if ( prefetch_.id == regionId ) {
            if ( prefetch_.date <= now ) return true;
            else return false;
        }
        suspendMessage(prefetch_);
        prefetched_ = false;
    }

    RegionMap::iterator iter = regionMap_.find(regionId);
    if ( iter != regionMap_.end() ) {
        MessageList& list = iter->second;
        if ( !list.empty() ) {
            Message& msg = list.front();
            if ( msg.date <= now ) {
                prefetch_ = msg;
                list.pop_front();
                return (prefetched_ = true);
            }
        }
    }

    smsc_log_debug(log_,"task %u/'%s' taking from store",id_,info_.name.c_str());
    assert(!prefetched_);
    while ( store_.getNextMessage(prefetch_) ) {
        prefetched_ = true;

        if ( prefetch_.date > now ) {
            // do not create cache entry, simply remember that message is prefetched
            return false;
        }

        // msg date is ok

        if ( prefetch_.regionId == regionId ) {
            // matched!
            return true;
        }
        suspendMessage(prefetch_);
        prefetched_ = false;
    }
    // no more messages in store
    return false;
}


const Message& Task::getPrefetched()
{
    MutexGuard mg(lock_);
    if ( !prefetched_ ) throw smsc::util::Exception("logic problem in task %u: msg is not prefetched",id_);
    prefetched_ = false;
    return prefetch_;
}


void Task::setMessageState( const Message& msg, uint8_t state )
{
    MutexGuard mg(lock_);
    // FIXME: set state
    if ( state == MessageState::RETRY ) {
        suspendMessage(msg);
    } else {
        smsc_log_warn(log_,"task %u/'%s' setting message state is not impl", id_, info_.name.c_str());
    }
}


void Task::update( const TaskInfo& info )
{
    smsc_log_info(log_,"updating task %u...",id_);
    MutexGuard mg(lock_);
    setInfo(info);
}



Task::~Task()
{
    smsc_log_debug(log_,"task %u/'%s' dtor",id_,info_.name.c_str());
}


Task::Task( unsigned              id,
            const std::string&    location,
            const TaskInfo&       taskInfo,
            smsc::db::DataSource* taskDs,
            FinalStateSaver*      saver ) :
id_(id),
location_(location),
users_(0),
active_(false),
prefetched_(false),
store_(location)
{
    smsc_log_debug(log_,"task %u/'%s' ctor",id_,taskInfo.name.c_str());
    setInfo(taskInfo);
}


void Task::setInfo( const TaskInfo& info )
{
    // FIXME: changing state
    info_ = info;
}


void Task::suspendMessage( const Message& msg )
{
    if (!prefetched_ && &msg != &prefetch_ ) {
        prefetched_ = true;
        prefetch_ = msg;
    } else {
        // looking for a map entry
        RegionMap::iterator iter = regionMap_.lower_bound(msg.regionId);
        if ( iter == regionMap_.end() || iter->first != msg.regionId ) {
            iter = regionMap_.insert
                (iter, RegionMap::value_type
                 ( msg.regionId, RegionMap::mapped_type() ) );
        }
        iter->second.push_back( msg );
    }
}

}
}
