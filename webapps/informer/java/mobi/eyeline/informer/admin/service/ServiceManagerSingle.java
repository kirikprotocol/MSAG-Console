package mobi.eyeline.informer.admin.service;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.service.daemon.ControlledService;
import mobi.eyeline.informer.admin.service.daemon.Daemon;

import java.io.File;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
class ServiceManagerSingle extends ServiceManager {

  private final Daemon daemon ;

  public ServiceManagerSingle(String host, int port, File servicesFolder) {
    daemon = new Daemon(host, port, servicesFolder);
  }

  @Override
  public Collection<String> getServices() throws AdminException {
    return daemon.getServiceIds();
  }

  @Override
  public ServiceInfo getService(String service) throws AdminException {
    ControlledService serv = daemon.getService(service);
    if (serv == null)
      return null;
    return new ServiceInfo(serv.getId(), new String[] {daemon.getHost()}, serv.getHost(), serv.getBaseDir());
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
    stopService(service);
    startService(service);
  }
}
