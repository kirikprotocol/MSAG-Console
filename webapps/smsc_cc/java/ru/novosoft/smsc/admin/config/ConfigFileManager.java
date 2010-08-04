package ru.novosoft.smsc.admin.config;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.ConfigHelper;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Каркас для написания менеджеров, управляющих одним конфигурационным файлом.
 *
 * @author Artem Snopkov
 */
public class ConfigFileManager<C> {

  protected final File configFile;
  protected final File backupDir;
  protected final FileSystem fileSystem;
  protected final ManagedConfigFile<C> config;

  public ConfigFileManager(File configFile, File backupDir, FileSystem fileSystem, ManagedConfigFile<C> cfgFileImpl) {
    this.configFile = configFile;
    this.backupDir = backupDir;
    this.fileSystem = fileSystem;
    this.config = cfgFileImpl;
  }

  public File getConfigFile() {
    return configFile;
  }

  public long getLastModified() {
    return configFile.lastModified();
  }

  public void save(C conf) throws AdminException {

    ConfigHelper.createBackup(configFile, backupDir, fileSystem);

    InputStream is = null;
    OutputStream os = null;
    File tmpConfigFile = new File(configFile.getAbsolutePath() + ".tmp");
    try {
      if (configFile.exists())
        is = fileSystem.getInputStream(configFile);
      os = fileSystem.getOutputStream(tmpConfigFile);

      config.save(is, os, conf);
    } catch (AdminException e) {
      throw e;
    } catch (Exception e) {
      throw new ConfigException("save_error", e);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }

      if (os != null)
        try {
          os.close();
        } catch (IOException ignored) {
        }
    }

    fileSystem.delete(configFile);
    fileSystem.rename(tmpConfigFile, configFile);
  }

  public C load() throws AdminException {
    InputStream is = null;
    try {
      is = fileSystem.getInputStream(configFile);
      return config.load(is);
    } catch (AdminException e) {
      throw e;
    } catch (Exception e) {
      throw new ConfigException("load_error", e);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
    }
  }
}
