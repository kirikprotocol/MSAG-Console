
#include "MCISmeComponent.h"

namespace smsc { namespace mcisme 
{

using namespace smsc::core::buffers;

MCISmeComponent::MCISmeComponent(MCISmeAdmin& admin)
    : logger(Logger::getInstance("smsc.mcisme.MCISmeComponent")), admin(admin)
{
    Parameters empty_params;
    Method flush_statistics((unsigned)flushStatisticsMethod, "flushStatistics", empty_params, StringType);
    Method get_statistics  ((unsigned)getStatisticsMethod,   "getStatistics", empty_params, StringType);
    Method get_runtime     ((unsigned)getRuntimeMethod,      "getRuntime", empty_params, StringType);

    methods[flush_statistics.getName()] = flush_statistics;
    methods[get_statistics.getName()] = get_statistics;
    methods[get_runtime.getName()] = get_runtime;
}

Variant MCISmeComponent::getStatistics()
{
    EventsStat stat = admin.getStatistics(); char buff[1024];
    sprintf(buff, "%d,%d,%d,%d",
            stat.missed, stat.delivered, stat.failed, stat.notified);
    return Variant(buff);
}
Variant MCISmeComponent::getRuntime()
{
    RuntimeStat stat = admin.getRuntimeStatistics(); char buff[1024];
    sprintf(buff, "%d,%d,%d,%d,%d", 
            stat.activeTasks, stat.inQueueSize, stat.outQueueSize, stat.inSpeed, stat.outSpeed);
    return Variant(buff);
}

Variant MCISmeComponent::call(const Method& method, const Arguments& args)
    throw (AdminException)
{
    try 
    {
        smsc_log_debug(logger, "call \"%s\"", method.getName());
        
        switch (method.getId())
        {
        case flushStatisticsMethod:
            admin.flushStatistics();
            break;
        case getStatisticsMethod:
            return getStatistics();
        case getRuntimeMethod:
            return getRuntime();
        
        default:
            smsc_log_debug(logger, "unknown method \"%s\" [%u]", method.getName(), method.getId());
            throw AdminException("Unknown method \"%s\"", method.getName());
        }

        return Variant("");
    }
    catch (AdminException &e) {
        smsc_log_error(logger, "AdminException: %s", e.what());
        throw e;
    }
    catch (std::exception& exc) {
        smsc_log_error(logger, "Exception: %s", exc.what());
        throw AdminException("%s", exc.what());
    }
    catch (...) {
        smsc_log_error(logger, "... Exception");
        throw AdminException("Unknown exception was caught during call");
    }
}

void MCISmeComponent::error(const char* method, const char* param)
{
    throw Exception("Parameter '%s' missed or invalid for MCISmeComponent::%s()", param, method);
}

}}

