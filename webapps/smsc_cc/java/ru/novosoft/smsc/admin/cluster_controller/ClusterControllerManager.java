package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class ClusterControllerManager extends ConfigFileManager<ClusterControllerConfig> {

  public ClusterControllerManager(File configFile, File backupDir, FileSystem fileSystem) throws AdminException {
    super(configFile, backupDir, fileSystem);
    reset();
  }

  protected ClusterControllerConfig getLastAppliedConfig() {
    return super.getLastAppliedConfig();
  }

  @Override
  protected ClusterControllerConfig newConfigFile() {
    return new ClusterControllerConfig();
  }

  public int getListenerPort() {
    return config.getListenerPort();
  }

  public void setListenerPort(int port) {
    config.setListenerPort(port);
    changed = true;
  }
}
