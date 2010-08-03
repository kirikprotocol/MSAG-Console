package ru.novosoft.smsc.admin.sme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestSmeManager extends SmeManager {

  public TestSmeManager(File configFile, File backupDir, ClusterController cc, ServiceManager sm, FileSystem fs) throws AdminException {
    super(configFile, backupDir, cc, sm, fs);
  }
}
