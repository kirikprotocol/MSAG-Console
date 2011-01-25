package mobi.eyeline.informer.admin.ftpserver;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.service.ServiceManager;

import java.io.File;
import java.util.List;

/**
 * User: artem
 * Date: 25.01.11
 */
abstract class AbstractFtpServerManager implements FtpServerManager {

  protected final ServiceManager serviceManager;
  protected final String serviceId;

  protected AbstractFtpServerManager(ServiceManager serviceManager, String serviceId) {
    this.serviceManager = serviceManager;
    this.serviceId = serviceId;
  }

  @Override
  public void startServer() throws AdminException {
    serviceManager.startService(serviceId);
  }

  @Override
  public void stopServer() throws AdminException {
    serviceManager.stopService(serviceId);
  }

  @Override
  public void switchServer(String toHost) throws AdminException {
    serviceManager.swichService(serviceId, toHost);
  }

  @Override
  public String getOnlineHost() throws AdminException {
    return serviceManager.getService(serviceId).getOnlineHost();
  }

  @Override
  public List<String> getServerHosts() throws AdminException {
    return serviceManager.getService(serviceId).getHosts();
  }

  protected File getServerBaseDir() throws AdminException {
    return serviceManager.getService(serviceId).getBaseDir();
  }
}
