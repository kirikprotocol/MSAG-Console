package ru.novosoft.smsc.admin.closed_groups;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;

import java.io.File;
import java.util.ArrayList;

/**
 * @author Artem Snopkov
 */
public class TestClosedGroupManager extends ClosedGroupManager {

  public TestClosedGroupManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    super(configFile, backupDir, cc, fs);
  }
}
