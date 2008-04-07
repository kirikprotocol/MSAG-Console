
#include "MCISmeComponent.h"
#include "admin/service/Variant.h"
#include <admin/service/Type.h>

namespace smsc {
namespace mcisme {

using namespace smsc::core::buffers;

MCISmeComponent::MCISmeComponent(MCISmeAdmin& admin)
  : logger(Logger::getInstance("smsc.mcisme.MCISmeComponent")), admin(admin)
{
  Parameters empty_params;
  Parameters string_param;
  string_param.Insert("Abonent", Parameter("Abonent", smsc::admin::service::StringType));

  Method flush_statistics((unsigned)flushStatisticsMethod, "flushStatistics", empty_params, StringType);
  Method get_statistics  ((unsigned)getStatisticsMethod,   "getStatistics", empty_params, StringType);
  Method get_runtime     ((unsigned)getRuntimeMethod,      "getRuntime", empty_params, StringType);
  Method get_scheditem   ((unsigned)getSchedItemMethod,    "getSchedItem", string_param, StringType);
  Method get_scheditems   ((unsigned)getSchedItemsMethod,    "getSchedItems", empty_params, StringType);

  methods[flush_statistics.getName()] = flush_statistics;
  methods[get_statistics.getName()] = get_statistics;
  methods[get_runtime.getName()] = get_runtime;
  methods[get_scheditem.getName()] = get_scheditem;
  methods[get_scheditems.getName()] = get_scheditems;
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
Variant MCISmeComponent::getSchedItem(const std::string Abonent)
{	
  return Variant(admin.getSchedItem(Abonent).c_str());
}

Variant MCISmeComponent::getSchedItems()
{	
  return Variant(admin.getSchedItems().c_str());
}

Variant MCISmeComponent::call(const Method& method, const Arguments& args)
  throw (AdminException)
{
  try 
  {
    //smsc_log_debug(logger, "call \"%s\"", method.getName());
        
    switch (method.getId())
    {
    case flushStatisticsMethod:
      admin.flushStatistics();
      break;
    case getStatisticsMethod:
      return getStatistics();
    case getRuntimeMethod:
      return getRuntime();
    case getSchedItemMethod:
      {
        Variant	arg = args.Get("Abonent");
        //				smsc_log_debug(logger, "Received getSchedItem call - %s", arg.getStringValue());
        return getSchedItem(arg.getStringValue());
      }
    case getSchedItemsMethod:
      {
        //				smsc_log_debug(logger, "Received getSchedItems call");
        return getSchedItems();
      }
        
    default:
      smsc_log_debug(logger, "unknown method \"%s\" [%u]", method.getName(), method.getId());
      throw AdminException("Unknown method \"%s\"", method.getName());
    }

    return Variant("");
  }
  catch (AdminException &e) {
    smsc_log_error(logger, "AdminException: %s", e.what());
    throw;
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

