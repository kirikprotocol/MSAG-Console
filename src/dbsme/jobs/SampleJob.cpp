
#include "SampleJob.h"

namespace smsc { namespace dbsme 
{

void SampleJob::init(ConfigView* config)
    throw(ConfigException)
{
}

void SampleJob::process(Command& command, DataSource& ds)
    throw(CommandProcessException)
{
    printf("Process command called !!!\n");
    throw CommandProcessException();
}

}}

