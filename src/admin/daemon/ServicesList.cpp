#include "ServicesList.h"
#include <admin/daemon/Service.h>

namespace smsc {
namespace admin {
namespace daemon {

void ServicesList::add(Service service) throw (AdminException &)
{
	if (services.Exists(service.getName()))
		throw new AdminException("Service already exists in list");
	services[service.getName()] = service;
	#ifdef SMSC_DEBUG
		logger.debug("Added service:\n  name=%s\n  cmd=%s\n  port=%u  pid=%u",
								 (services[service.getName()].getName() == 0 ? "null" : services[service.getName()].getName()),
								 (services[service.getName()].getCommandLine() == 0 ? "null" : services[service.getName()].getCommandLine()),
								 services[service.getName()].getPort(),
								 services[service.getName()].getPid());
	#endif
}

void ServicesList::remove(const char * const serviceName) throw (AdminException &)
{
	if (!services.Exists(serviceName))
		throw new AdminException("Service not found");
	if (services[serviceName].getStatus() == Service::running)
	{
		services[serviceName].kill();
	}
	services.Delete(serviceName);
}

Service & ServicesList::get(const char * const serviceName)
{
	if (!services.Exists(serviceName))
		throw AdminException("Service not found");

	return services[serviceName];
}

char * ServicesList::getText() const
{
	Service s;
	char * sname;
	std::string result = "";

	#ifdef SMSC_DEBUG
		logger.debug("ServicesList::getText()");
	#endif

	for (_ServiceList::Iterator i = services.getIterator(); i.Next(sname, s); )
	{
		#ifdef SMSC_DEBUG
			logger.debug("******************");
			logger.debug("  name=%s", sname);
			logger.debug("  name=%s", s.getName());
			logger.debug("  port=%lu", (unsigned long)s.getPid());
			logger.debug("  pid=%lu",  (unsigned long)s.getPort());
			logger.debug("  cmd=%s", s.getCommandLine());
		#endif
		
		char pid[sizeof(pid_t)*3 + 1];
		sprintf(pid, "%lu", (unsigned long)s.getPid());
		char port[sizeof(in_port_t)*3 + 1];
		sprintf(port, "%lu", (unsigned long)s.getPort());
		
		result += "<service><name>";
		result += s.getName();
		result += "</name><pid>";
		result += pid;
		result += "</pid><command_line>";
		result += s.getCommandLine();
		result += "</command_line><port>";
		result += port;
		result += "</port>";
		for (unsigned j=0; s.getArgs()[j] != 0; j++)
		{
			char argNum[sizeof(unsigned)*3+1];
			sprintf(argNum, "%u", j);
			result += "<arg num=\"";
			result += argNum;
			result += "\">";
			result += s.getArgs()[j];
			result += "</arg>";
		}
		result += "</service>\n";
	}
	return cStringCopy(result.c_str());
}

}
}
}
