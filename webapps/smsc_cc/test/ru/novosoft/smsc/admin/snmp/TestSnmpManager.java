package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class TestSnmpManager extends SnmpManager {

  public TestSnmpManager(File configFile, File backupDir, ClusterController cc, FileSystem fileSystem) throws AdminException {
    super(configFile, backupDir, cc, fileSystem);
  }
}
