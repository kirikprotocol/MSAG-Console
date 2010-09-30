package ru.novosoft.smsc.admin.category;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class CategoryManagerImpl implements CategoryManager {
  
  private final ConfigFileManager<CategorySettings> cfgFileManager;

  public CategoryManagerImpl(File configFile, File backupDir, FileSystem fileSystem) {
    this.cfgFileManager = new ConfigFileManager<CategorySettings>(configFile, backupDir, fileSystem, new CategoryConfigFile());
  }

  public CategorySettings getSettings() throws AdminException {
    return this.cfgFileManager.load();
  }

  public void updateSettings(CategorySettings settings) throws AdminException {
    this.cfgFileManager.save(settings);
  }
}
