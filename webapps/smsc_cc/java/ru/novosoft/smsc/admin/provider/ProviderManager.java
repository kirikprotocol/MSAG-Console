package ru.novosoft.smsc.admin.provider;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class ProviderManager {

  private final ConfigFileManager<ProviderSettings> cfgFileManager;

  public ProviderManager(File configFile, File backupDir, FileSystem fileSystem) {
    this.cfgFileManager = new ConfigFileManager<ProviderSettings>(configFile, backupDir, fileSystem, new ProviderConfigFile());
  }

  public ProviderSettings getSettings() throws AdminException {
    return this.cfgFileManager.load();
  }

  public void updateSettings(ProviderSettings settings) throws AdminException {
    this.cfgFileManager.save(settings);
  }
}
