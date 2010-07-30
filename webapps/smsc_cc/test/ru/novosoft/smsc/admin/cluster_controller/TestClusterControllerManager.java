package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceManager;

/**
 * @author Artem Snopkov
 */
public class TestClusterControllerManager extends ClusterControllerManager {
  public TestClusterControllerManager(ServiceManager serviceManager, FileSystem fileSystem) throws AdminException {
    super(serviceManager, fileSystem);
  }
}
