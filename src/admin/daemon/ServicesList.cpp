#include "ServicesList.h"
#include <admin/daemon/Service.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::util::encode;

void ServicesList::add(Service *service) throw (AdminException)
{
	log4cpp::Category &logger(Logger::getCategory("smsc.admin.daemon.ServicesList"));
	if (services.Exists(service->getId()))
		throw AdminException("Service already exists in list");
	services[service->getId()] = service;
	#ifdef SMSC_DEBUG
		logger.debug("Added service:\n  cmd=%s\n  port=%u  pid=%u",
								 (services[service->getId()]->getId() == 0 ? "null" : services[service->getId()]->getId()),
								 //(services[service->getId()]->getName() == 0 ? "null" : services[service->getId()]->getName()),
								 services[service->getId()]->getPort(),
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
			log4cpp::Category &logger(Logger::getCategory("smsc.admin.daemon.ServicesList"));
			logger.debug("******************");
			logger.debug("  name=%s", sname);
			logger.debug("  name=%s", s.getName());
			logger.debug("  port=%lu", (unsigned long)s.getPid());
			logger.debug("  pid=%lu",  (unsigned long)s.getPort());
			logger.debug("  cmd=%s", s.getCommandLine());
		#endif
*/		
		char pid[sizeof(pid_t)*3 + 1];
		snprintf(pid, sizeof(pid), "%lu", (unsigned long)s->getPid());
		char port[sizeof(in_port_t)*3 + 1];
		snprintf(port, sizeof(port), "%lu", (unsigned long)s->getPort());
		std::auto_ptr<char> tmpServiceId(encode(s->getId()));
		//std::auto_ptr<char> tmpServiceName(encode(s->getName()));
		std::auto_ptr<char> tmpArgs(encode(s->getArgs()));
		
		result += "<service id=\"";
		result += tmpServiceId.get();
		//result += "\" name=\"";
		//result += tmpServiceName.get();
		result += "\" pid=\"";
		result += pid;
		result += "\" port=\"";
		result += port;
		result += "\" args=\"";
		result += tmpArgs.get();
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
		log4cpp::Category &logger(Logger::getCategory("smsc.admin.daemon.ServicesList"));
		//logger.debug("Mark service %lu as dead", (unsigned long) old_pid);
	#endif
	char * sname;
	Service *s;
	for (_ServiceList::Iterator i = services.getIterator(); i.Next(sname, s); )
	{
		#ifdef SMSC_DEBUG
			//logger.debug("  test service %lu for pid", (unsigned long) s->getPid());
		#endif
		if (s->getPid() == old_pid)
		{
			#ifdef SMSC_DEBUG
				//logger.debug("  FINDED SERVICE!!! %lu ", (unsigned long) old_pid);
			#endif
			s->setPid(0);
      s->setStatus(Service::stopped);
			#ifdef SMSC_DEBUG
				logger.debug("SERVICE %lu MARKED AS DEAD", (unsigned long) old_pid);
			#endif
			return s->getId();
		}
	}
	return 0;
}

}
}
}
