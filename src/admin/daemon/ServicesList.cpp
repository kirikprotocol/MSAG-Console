#include "ServicesList.h"
#include <admin/daemon/Service.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::util::encode;

void ServicesList::add(Service *service) throw (AdminException)
{
	smsc::logger::Logger *logger = Logger::getInstance("smsc.admin.daemon.ServicesList");
	if (services.Exists(service->getId()))
		throw AdminException("Service already exists in list");
	services[service->getId()] = service;
	#ifdef SMSC_DEBUG
		smsc_log_debug(logger, "Added service:\n  cmd=%s\n  pid=%u",
								 (services[service->getId()]->getId() == 0 ? "null" : services[service->getId()]->getId()),
								 services[service->getId()]->getPid());
	#endif
}

void ServicesList::remove(const char * const serviceId) throw (AdminException)
{
	if (!services.Exists(serviceId))
		throw AdminException("Service not found");
	if (services[serviceId]->getStatus() != Service::stopped)
	{
		services[serviceId]->kill();
	}
	services.Delete(serviceId);
}

Service * ServicesList::get(const char * const serviceId)
{
	if (!services.Exists(serviceId))
		throw AdminException("Service not found");

	return services[serviceId];
}

char * ServicesList::getText() const
{
	Service *s;
	char * sname;
	std::string result = "";

	for (_ServiceList::Iterator i = services.getIterator(); i.Next(sname, s); )
	{
/*		#ifdef SMSC_DEBUG
			smsc::logger::Logger logger(Logger::getInstance("smsc.admin.daemon.ServicesList"));
			smsc_log_debug(logger, "******************");
			smsc_log_debug(logger, "  name=%s", sname);
			smsc_log_debug(logger, "  name=%s", s.getName());
			smsc_log_debug(logger, "  port=%lu", (unsigned long)s.getPid());
			smsc_log_debug(logger, "  pid=%lu",  (unsigned long)s.getPort());
			smsc_log_debug(logger, "  cmd=%s", s.getCommandLine());
		#endif
*/		
		char pid[sizeof(pid_t)*3 + 1];
		snprintf(pid, sizeof(pid), "%lu", (unsigned long)s->getPid());
		std::auto_ptr<char> tmpServiceId(encode(s->getId()));
		std::auto_ptr<char> tmpArgs(encode(s->getArgs()));
		
		result += "<service id=\"";
		result += tmpServiceId.get();
		result += "\" pid=\"";
		result += pid;
		result += "\" args=\"";
		result += tmpArgs.get();
		result += "\" autostart=\"";
		result += s->isAutostart() ? "true" : "false";
    result += "\" status=\"";
    switch (s->getStatus())
    {
    case Service::running:
      result += "running";
      break;
    case Service::stopped:
      result += "stopped";
      break;
    case Service::starting:
      result += "starting";
      break;
    case Service::stopping:
      result += "stopping";
      break;
    default:
      result += "unknown";
    }
		result += "\"/>\n";
	}
	return cStringCopy(result.c_str());
}

const char * const ServicesList::markServiceAsStopped(pid_t old_pid)
{
	#ifdef SMSC_DEBUG
		smsc::logger::Logger *logger = Logger::getInstance("smsc.admin.daemon.ServicesList");
		//smsc_log_debug(logger, "Mark service %lu as dead", (unsigned long) old_pid);
	#endif
	char * sname;
	Service *s;
	for (_ServiceList::Iterator i = services.getIterator(); i.Next(sname, s); )
	{
		#ifdef SMSC_DEBUG
			//smsc_log_debug(logger, "  test service %lu for pid", (unsigned long) s->getPid());
		#endif
		if (s->getPid() == old_pid)
		{
			#ifdef SMSC_DEBUG
				//smsc_log_debug(logger, "  FINDED SERVICE!!! %lu ", (unsigned long) old_pid);
			#endif
			s->setPid(0);
      s->setStatus(Service::stopped);
			#ifdef SMSC_DEBUG
				smsc_log_debug(logger, "SERVICE %lu MARKED AS DEAD", (unsigned long) old_pid);
			#endif
			return s->getId();
		}
	}
	return 0;
}

}
}
}
