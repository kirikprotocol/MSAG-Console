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
public abstract class ConfigFileManager<T extends ManagedConfigFile> {

  protected final File configFile;
  protected final File backupDir;
  protected final FileSystem fileSystem;

  protected T config;
  protected T lastAppliedConfig;
  private boolean changed;
  private long lastChangeTime = -1;

  protected ConfigFileManager(File configFile, File backupDir, FileSystem fileSystem) {
    this.configFile = configFile;
    this.backupDir = backupDir;
    this.fileSystem = fileSystem;
  }

  protected abstract T newConfigFile();

  public void setChanged() {
    lastChangeTime = System.currentTimeMillis();
    changed = true;
  }

  /**
   * @deprecated Метод должен использоваться только в тестовых целях
   */
  protected void setNotChanged(boolean updateLastChangeTime) {
    if (changed && updateLastChangeTime)
      lastChangeTime = System.currentTimeMillis();
    changed = false;
  }

  protected void lockConfig(boolean write) throws AdminException {
  }

  protected void unlockConfig() throws Exception {
  }

  protected void afterApply() throws AdminException {
  }

  protected void beforeApply() throws AdminException {
  }

  protected void beforeReset() throws AdminException {
  }

  public T getLastAppliedConfig() {
    return lastAppliedConfig;
  }

  public T getConfig() {
    return config;
  }

  public File getConfigFile() {
    return configFile;
  }

  public void apply() throws AdminException {
    if (config == null)
      return;

    beforeApply();

    Throwable unlockConfigError = null;
    try {
      lockConfig(true);

      ConfigHelper.createBackup(configFile, backupDir, fileSystem);

      InputStream is = null;
      OutputStream os = null;
      File tmpConfigFile = new File(configFile.getAbsolutePath() + ".tmp");
      try {
        is = fileSystem.getInputStream(configFile);
        os = fileSystem.getOutputStream(tmpConfigFile);

        config.save(is, os);
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

    } finally {
      try {
        unlockConfig();
      } catch (Exception e) {
        unlockConfigError = e;
      }
    }

    if (unlockConfigError != null)
      throw new ConfigException("save_error", unlockConfigError);

    afterApply();
    this.changed = false;
    lastAppliedConfig = loadConfig();
  }

  private T loadConfig() throws AdminException {
    T cfg = newConfigFile();

    Throwable unlockConfigError = null;
    InputStream is = null;
    try {
      lockConfig(false);
      is = fileSystem.getInputStream(configFile);
      cfg.load(is);
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

      try {
        unlockConfig();
      } catch (Exception e) {
        unlockConfigError = e;
      }
    }

    if (unlockConfigError != null)
      throw new ConfigException("load_error", unlockConfigError);

    return cfg;
  }

  public void reset() throws AdminException {
    this.config = loadConfig();
    this.lastAppliedConfig = loadConfig();
    if (changed)
      this.lastChangeTime = System.currentTimeMillis();
    this.changed = false;
  }

  public boolean isChanged() {
    return changed;
  }

  public long getLastChangeTime() {
    return lastChangeTime;
  }
}
