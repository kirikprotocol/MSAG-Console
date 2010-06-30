package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.daemon.ControlledService;
import ru.novosoft.smsc.admin.service.daemon.DaemonHS;

import java.io.File;
import java.util.Collection;
import java.util.List;

/**
 * @author Artem Snopkov
 */
class ServiceManagerHS extends ServiceManager {

  private DaemonHS daemon;

  public ServiceManagerHS(String host, int port, File servicesDir, Collection<String> hosts) {
    daemon = new DaemonHS(host, port, servicesDir, hosts);
  }

  @Override
  public Collection<String> getServices() throws AdminException {
    return daemon.getServiceIds();
  }

  @Override
  public ServiceInfo getService(String service) throws AdminException {
    ControlledService serv = daemon.getService(service);
    return new ServiceInfo(serv.getId(), daemon.getHosts(), serv.getHost(), serv.getBaseDir());
  }

  @Override
  public void startService(String service) throws AdminException {
    daemon.startService(service);
  }

  @Override
  public void stopService(String service) throws AdminException {
    daemon.shutdownService(service);
  }

  @Override
  public void swichService(String service, String toHost) throws AdminException {
    daemon.switchOver(service);
  }
}
