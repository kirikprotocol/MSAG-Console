package ru.novosoft.smsc.admin.timezone;

import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestTimezoneManager extends TimezoneManagerImpl {
  public TestTimezoneManager(File configFile, File backupDir, FileSystem fileSystem, ClusterController cc) {
    super(configFile, backupDir, fileSystem, cc);
  }
}
