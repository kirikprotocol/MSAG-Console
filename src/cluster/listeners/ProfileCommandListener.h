#ifndef ProfileCommandListener_dot_h
#define ProfileCommandListener_dot_h

#include "cluster/Interconnect.h"
#include "profiler/profiler-types.hpp"
#include "profiler/profiler.hpp"
#include "logger/Logger.h"

namespace smsc {
namespace cluster {

using smsc::profiler::ProfilerInterface;

class ProfileCommandListener : public CommandListener 
{
protected:
    smsc::profiler::Profiler *profiler;
    smsc::logger::Logger *logger;
    void profileUpdate(const Command& command);
	void profileDelete(const Command& command);

public:
    ProfileCommandListener(smsc::profiler::Profiler * profiler_);
	virtual void handle(const Command& command);

};

}
}

#endif

