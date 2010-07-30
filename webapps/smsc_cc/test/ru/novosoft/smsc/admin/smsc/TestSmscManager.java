package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

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
