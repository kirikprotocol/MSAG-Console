package ru.novosoft.smsc.admin.resource;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class ResourceManagerImpl implements ResourceManager {

  private final File configsDir;
  private final File backupDir;
  private final ClusterController cc;
  private final FileSystem fs;
  private Map<String, ConfigFileManager<ResourceSettings>> resourceFilesManagers;

  public ResourceManagerImpl(File configsDir, File backupDir, ClusterController cc, FileSystem fs) {
    this.configsDir = configsDir;
    this.backupDir = backupDir;
    this.cc = cc;
    this.fs = fs;
  }

  private synchronized void reset() throws AdminException {
    if (resourceFilesManagers != null)
      return;

    this.resourceFilesManagers = new HashMap<String, ConfigFileManager<ResourceSettings>>();

    try {
      if (cc.isOnline())
        cc.lockResources(false);

      resourceFilesManagers.clear();
      File[] files = configsDir.listFiles();
      if (files == null)
        return;
      for (File file : files) {
        String fileName = file.getName();
        if (fileName.startsWith("resources_") && fileName.endsWith(".xml")) {
          String localeName = fileName.substring("resources_".length(), fileName.length() - ".xml".length());
          ConfigFileManager<ResourceSettings> mngr = new ConfigFileManager<ResourceSettings>(file, backupDir, fs, new ResourceFile());
          resourceFilesManagers.put(localeName, mngr);
        }
      }
    } finally {
      if (cc.isOnline())
        cc.unlockResources();
    }
  }

  public Collection<String> getLocales() throws AdminException {
    reset();
    return resourceFilesManagers.keySet();
  }

  public boolean containsLocale(String locale) throws AdminException {
    reset();
    return resourceFilesManagers.containsKey(locale);
  }

  public void addResourceSettings(String locale, ResourceSettings resources) throws AdminException {
    reset();
    ConfigFileManager<ResourceSettings> mngr = resourceFilesManagers.get(locale);
    if (mngr == null) {
      File resourceFile = new File(configsDir, "resources_" + locale + ".xml");
      mngr = new ConfigFileManager(resourceFile, backupDir, fs, new ResourceFile());
      resourceFilesManagers.put(locale, mngr);
    }

    try {
      if (cc.isOnline())
        cc.lockResources(true);

      mngr.save(resources);
    } finally {
      if (cc.isOnline())
        cc.unlockResources();
    }

    if (cc.isOnline())
      cc.applyResources();
  }

  public boolean removeResourceSettings(String locale) throws AdminException {
    reset();

    if (!resourceFilesManagers.containsKey(locale))
      return false;

    File resourceFile = new File(configsDir, "resources_" + locale + ".xml");
    if (fs.exists(resourceFile))
      fs.delete(resourceFile);

    resourceFilesManagers.remove(locale);
    return true;
  }

  public ResourceSettings getResourceSettings(String locale) throws AdminException {
    reset();

    ConfigFileManager<ResourceSettings> mngr = resourceFilesManagers.get(locale);
    if (mngr == null)
      return null;

    try {
      if (cc.isOnline())
        cc.lockResources(false);

      return mngr.load();
    } finally {
      if (cc.isOnline())
        cc.unlockResources();
    }
  }

 
  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;

    ConfigState configState = cc.getResourcesState();

    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();

    if (configState != null) {
      long ccLastUpdateTime = configState.getCcLastUpdateTime();
      for (Map.Entry<Integer, Long> e : configState.getInstancesUpdateTimes().entrySet()) {
        SmscConfigurationStatus s = (e.getValue() >= ccLastUpdateTime) ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
        result.put(e.getKey(), s);
      }
    }

    return result;
  }

}
