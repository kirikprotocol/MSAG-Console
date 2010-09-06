package ru.novosoft.smsc.admin.region;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class RegionManager {

  private final ConfigFileManager<RegionSettings> cfgFileManager;

  public RegionManager(File configFile, File backupDir, FileSystem fs) {
    this.cfgFileManager = new ConfigFileManager<RegionSettings>(configFile, backupDir, fs, new RegionsConfig());
  }

  public RegionSettings getSettings() throws AdminException {
    return cfgFileManager.load();
  }

  public void updateSettings(RegionSettings s) throws AdminException {
    cfgFileManager.save(s);
  }
}
