#include "TaskLock.h"
#include "TaskProcessor.h"
#include "DataProvider.h"
#include "ConfString.h"
#include "MsecTime.h"
#include "util/config/ConfigException.h"
#include "sms/sms.h"

using namespace smsc::util::config;

namespace {

bool isMSISDNAddress( const std::string& str )
{
    try {
        smsc::sms::Address( str.c_str() );
        return true;
    } catch (...) {}
    return false;
}

}

namespace smsc {
namespace infosme2 {

TaskProcessor::TaskProcessor() :
log_(smsc::logger::Logger::getInstance("is2.proc")),
scheduler_(*this),
dispatcher_(),
sender_(dispatcher_),
started_(false),
stopping_(false),
notified_(false),
dataProvider_(0),
finalStateSaver_(0)
{
    smsc_log_debug(log_,"ctor");
}


TaskProcessor::~TaskProcessor()
{
    stopTaskScheduler();
    stopTaskProcessor();
    if (dataProvider_) delete dataProvider_;
    // if (finalStateSaver_) delete finalStateSaver_;
    smsc_log_debug(log_,"dtor");
}


void TaskProcessor::init( ConfigView* config )
{
    smsc_log_info(log_,"init");
    MutexGuard mg(startMon_);
    if ( started_ ||
         isTaskSchedulerRunning() ) throw ConfigException("cannot init while started");

    // FIXME: cleanup the state

    storeLocation_ = config->getString("storeLocation");
    if (! storeLocation_.empty() ) {
        if( *storeLocation_.rbegin() != '/' ) {
            storeLocation_ += '/';
        }
    }
    const std::string lockfile(storeLocation_ + "taskslock.bin");
    TaskLock::Init(lockfile.c_str());

    address_ = ConfString(config->getString("Address")).str();
    if ( address_.empty() || !isMSISDNAddress(address_) ) {
        throw ConfigException("Address string '%s' is invalid", address_.c_str());
    }

    // FIXME: load other things

    try {
        std::auto_ptr<ConfigView> subcfg(config->getSubConfig("DataProvider"));
        if (!dataProvider_) dataProvider_ = new DataProvider();
        dataProvider_->init(subcfg.get());
    } catch ( ConfigException& ) {
        if (dataProvider_) delete dataProvider_;
        dataProvider_ = 0;
    }

    // init sender
    sender_.init( config );

    // loading tasks
    smsc_log_info(log_,"loading tasks...");
    std::auto_ptr< ConfigView > taskConfig;
    std::auto_ptr< std::set< std::string> > taskNames;
    try {
        taskConfig.reset(config->getSubConfig("Tasks"));
        taskNames.reset( taskConfig->getShortSectionNames() );
    } catch ( ConfigException& ) {
        smsc_log_warn(log_,"problem reading section 'Tasks'");
    }
    if ( !taskNames.get() ) { taskNames.reset(new std::set<std::string>()); }

    // adding tasks that are in storelocation
    std::vector< std::string > entries;
    entries.reserve(200);
    smsc::core::buffers::File::ReadDir( storeLocation_.c_str(),
                                        entries );
    for ( std::vector< std::string >::const_iterator i = entries.begin();
          i != entries.end();
          ++i ) {
        if ( atoi(i->c_str()) != 0 ) {
            if ( taskNames->insert(*i).second ) {
                smsc_log_debug(log_,"adding task name '%s' to the list",i->c_str());
            }
        }
    }
    // finally read all tasks
    for ( std::set< std::string >::const_iterator i = taskNames->begin();
          i != taskNames->end(); ++i ) {

        try {
            unsigned id = Task::stringToTaskId(i->c_str());
            loadTask( id, taskConfig.get() );
        } catch ( ConfigException& e ) {
            smsc_log_warn(log_,"cannot load task %s: %s",i->c_str(),e.what());
        }
    }

    scheduler_.init(config);
    startTaskScheduler();
    smsc_log_info(log_,"init finished");
}


int TaskProcessor::Execute()
{
    const msectime_type startTime = currentTimeMillis();
    smsc_log_info(log_,"execute started");

    msectime_type currentTime = startTime;
    msectime_type movingStart = startTime;
    msectime_type nextWakeTime = startTime;
    msectime_type lastNotifyTime = startTime;

    // msectime_type lastStatTime = startTime;

    try {
        while ( true ) {

            currentTime = currentTimeMillis();

            // 1. sleeping until some interesting events are there
            {
                if ( stopping_ ) break;
                int waitTime = int(nextWakeTime - currentTime);
                if ( waitTime > 0 && !notified_ ) {
                    if ( waitTime < 10 ) waitTime = 10;
                    MutexGuard mg(startMon_);
                    if ( !notified_ ) {
                        smsc_log_debug(log_,"want to sleep %u ms",waitTime);
                        startMon_.wait( waitTime );
                        continue;
                    }
                }
            }

            const unsigned deltaTime = unsigned(currentTime - movingStart);

            // 2. flipping start time
            if ( deltaTime > 1000000 ) {
                movingStart = currentTime;
                smsc_log_info(log_,"making a flip for a startTime");
                continue;
            }

            smsc_log_debug(log_,"new pass at %u, notified=%u",deltaTime,notified_);

            // 3. FIXME: dumping statistics

            // 4. processing notification
            while ( notified_ || (currentTime - lastNotifyTime) > 30000 ) {
                lastNotifyTime = currentTime;
                smsc_log_debug(log_,"notified or time passed");
                MutexGuard mg(startMon_);
                notified_ = false;
                // FIXME: process events
            }

            // 5. processing
            nextWakeTime = currentTime + sender_.send(deltaTime,1000);

            // 6. FIXME: find out if dispatcher_ has inactive tasks
        }

    } catch ( std::exception& e ) {
        smsc_log_error(log_,"exc in execute, stopping: %s", e.what());
    }
    MutexGuard mg(startMon_);
    started_ = false;
    startMon_.notifyAll();
    smsc_log_info(log_,"execute finished");
    return 0;
}


void TaskProcessor::loadTask( unsigned id, ConfigView* taskConfig )
{
    smsc_log_info(log_,"loading task %u...", id );
    char taskId[30];
    sprintf(taskId,"%u",id);

    std::auto_ptr< Config > separateConfig;
    std::auto_ptr< ConfigView > separateView;
    const std::string location = storeLocation_ + taskId;
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

    TaskInfo info;
    info.init(id,taskConfig);

    TaskGuard guard(getTask(id));
    if ( guard.get() ) {

        guard->update(info);

    } else {

        smsc::db::DataSource* taskDs = 0;
        if ( !info.isDelivery() ) {
            const char* dsId = taskConfig->getString("dsId");
            if (!dsId || dsId[0] == '\0')
                throw ConfigException("DataSource id for task %u is empty or wasn't specified",
                                      id);
            if ( !dataProvider_ ) {
                throw ConfigException("DataSourceProvider is not provided");
            }
            taskDs = dataProvider_->getDataSource(dsId);
            if (!taskDs)
                throw ConfigException("Failed to obtail DataSource driver '%s' for task %u",
                                      dsId, id);
        }
        addTask( guard.create(id,location,info,taskDs,finalStateSaver_) );
    }
    smsc_log_info(log_,"task %u is loaded",id);
}


void TaskProcessor::startTaskProcessor()
{
    MutexGuard mg(startMon_);
    if ( started_ ) return;
    smsc_log_debug(log_,"starting task processor");
    stopping_ = false;
    Start();
    started_ = true;
}


void TaskProcessor::stopTaskProcessor()
{
    MutexGuard mg(startMon_);
    if ( !started_ ) return;
    smsc_log_debug(log_,"stopping task processor");
    stopping_ = true;
    startMon_.notifyAll();
    while ( started_ ) {
        startMon_.wait();
    }
    WaitFor();
}


}
}
