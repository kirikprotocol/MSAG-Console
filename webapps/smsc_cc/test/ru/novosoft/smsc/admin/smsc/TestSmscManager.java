package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceManager;

/**
 * Тестовая реализация SmscConfig
 *
 * @author Artem Snopkov
 */
public class TestSmscManager extends SmscManager {

  public TestSmscManager(ServiceManager serviceManager, ClusterController cc, FileSystem fileSystem) throws AdminException {
    super(serviceManager, cc, fileSystem);
  }
}
