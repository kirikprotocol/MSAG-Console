
#include "SampleJob.h"

namespace smsc { namespace dbsme 
{

void SampleJob::init(ConfigView* config)
    throw(ConfigException)
{
    Job::init(config);
}

void SampleJob::process(Command& command, Statement& stmt) 
    throw(CommandProcessException)
{
    printf("Process command called !!!\n");
    throw CommandProcessException();
}

}}

