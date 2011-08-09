package mobi.eyeline.informer.admin.service;

import mobi.eyeline.informer.admin.AdminException;

import java.io.File;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class TestServiceManagerHA extends ServiceManager {

  private final Map<String, ServiceInfo> services;
  private final File servicesDir;
  private final String[] hosts;

  public TestServiceManagerHA(File servicesDir, String[] hosts) {
    this.servicesDir = servicesDir;
    this.hosts = hosts;
    services = new HashMap<String, ServiceInfo>();
    services.put("Informer", getInfo("Informer", true));
    services.put("ArchiveDaemon", getInfo("ArchiveDaemon", true));
    services.put("pvss", getInfo("pvss", true));
    services.put("dcpgw", getInfo("dcpgw", true));
  }

  private ServiceInfo getInfo(String serviceId, boolean online) {
    return new ServiceInfo(serviceId, hosts, online ? hosts[0] : null, new File(servicesDir, serviceId));
  }

  @Override
  public Collection<String> getServices() throws AdminException {
    return services.keySet();
  }

  @Override
  public ServiceInfo getService(String service) throws AdminException {
    return services.get(service);
  }

  @Override
  public void startService(String service) throws AdminException {
    if (services.containsKey(service))
      services.put(service, getInfo(service, true));
    else
      throw new ServiceManagerException("service_not_found", service);
  }

  @Override
  public void stopService(String service) throws AdminException {
    if (services.containsKey(service))
      services.put(service, getInfo(service, false));
    else
      throw new ServiceManagerException("service_not_found", service);
  }

  @Override
  public void swichService(String service, String toHost) throws AdminException {
    if (services.containsKey(service)) {
      ServiceInfo info = services.get(service);
      if (!info.getHosts().contains(toHost))
        throw new ServiceManagerException("cant_start_service", service);
      services.put(service, new ServiceInfo(service, info.getHosts(), toHost, info.getBaseDir()));
    } else
      throw new ServiceManagerException("service_not_found", service);

  }
}
