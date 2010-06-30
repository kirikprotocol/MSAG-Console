package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.resource_group.ResourceGroup;
import ru.novosoft.smsc.admin.service.resource_group.ResourceGroupManager;

import java.io.File;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
class ServiceManagerHA extends ServiceManager {

  private final ResourceGroupManager resGroupManager;

  public ServiceManagerHA(File resourceGroupsFile, File servicesDir) throws AdminException {
    resGroupManager = new ResourceGroupManager(resourceGroupsFile, servicesDir);
  }

  @Override
  public Collection<String> getServices() {
    return resGroupManager.getResourceGroupsNames();
  }

  @Override
  public ServiceInfo getService(String service) throws AdminException {
    ResourceGroup group = resGroupManager.get(service);
    if (group == null)
      return null;
    return new ServiceInfo(service, group.listNodes(), group.getOnlineStatus(), group.getBaseDir());
  }

  @Override
  public void startService(String service) throws AdminException {
    ResourceGroup group = resGroupManager.get(service);
    if (group == null)
      throw new AdminException("CallableService '" + service + "' does not exist");

    group.offline();
  }

  @Override
  public void stopService(String service) throws AdminException {
    ResourceGroup group = resGroupManager.get(service);
    if (group == null)
      throw new AdminException("CallableService '" + service + "' does not exist");

    group.offline();
  }

  @Override
  public void swichService(String service, String toHost) throws AdminException {
    ResourceGroup group = resGroupManager.get(service);
    if (group == null)
      throw new AdminException("CallableService '" + service + "' does not exist");

    for (String node : group.listNodes()) {
      if (node.equals(toHost)) {
        group.switchOver(toHost);
        return;
      }
    }
    throw new AdminException("Unable to start callable '" + service + "' at " + toHost);
  }
}
