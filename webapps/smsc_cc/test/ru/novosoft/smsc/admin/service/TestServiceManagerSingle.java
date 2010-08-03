package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;

import java.io.File;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class TestServiceManagerSingle extends ServiceManager {

  private final Map<String, ServiceInfo> services;
  private final File servicesDir;

  public TestServiceManagerSingle(File servicesDir, int smscInstancesCount) {
    this.servicesDir = servicesDir;
    services = new HashMap<String, ServiceInfo>();
    services.put("ArchiveDaemon", getInfo("ArchiveDaemon", true));
    services.put("ClusterController", getInfo("ClusterController", true));
    for (int i=0; i<smscInstancesCount; i++)
      services.put("SMSC"+i, getInfo("SMSC" + i, true));
  }

  private ServiceInfo getInfo(String serviceId, boolean online) {
    return new ServiceInfo(serviceId, new String[] {"localhost"}, online ? "localhost" : null, new File(servicesDir, serviceId));
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
    throw new ServiceManagerException("cant_start_service", service);
  }
}
