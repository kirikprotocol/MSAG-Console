package ru.novosoft.smsc.admin.resource;

import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestResourceManager extends ResourceManager {
  public TestResourceManager(File configsDir, File backupDir, ClusterController cc, FileSystem fs) {
    super(configsDir, backupDir, cc, fs);
  }
}
