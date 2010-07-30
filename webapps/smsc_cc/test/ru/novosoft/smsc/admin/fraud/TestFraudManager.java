package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

/**
 * Тестовая реализация FraudManager
 *
 * @author Artem Snopkov
 */
public class TestFraudManager extends FraudManager {

  public TestFraudManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    super(configFile, backupDir, cc, fs);
  }
}
