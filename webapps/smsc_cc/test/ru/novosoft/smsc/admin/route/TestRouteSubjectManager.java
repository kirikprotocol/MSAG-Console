package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestRouteSubjectManager extends RouteSubjectManagerImpl {

  public TestRouteSubjectManager(File configFile, File backupDir, FileSystem fs, ClusterController cc) {
    super(configFile, backupDir, fs, cc);
  }
}
