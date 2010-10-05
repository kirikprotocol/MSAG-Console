package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestRescheduleManager extends RescheduleManagerImpl {

  public TestRescheduleManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    super(configFile, backupDir, cc, fs);
  }

  public static RescheduleSettings createRescheduleSettings() {
    return new RescheduleSettings();
  }
}
