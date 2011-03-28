package mobi.eyeline.informer.admin.util.config;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Каркас для написания менеджеров, управляющих одним конфигурационным файлом.
 *
 * @author Artem Snopkov
 */
public class ConfigFileManager<C> {

  private static final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yy.HH.mm.ss");

  private final File configFile;
  private final File backupDir;
  private final FileSystem fileSystem;
  private final ManagedConfigFile<C> config;

  public ConfigFileManager(File configFile, File backupDir, FileSystem fileSystem, ManagedConfigFile<C> cfgFileImpl) {
    this.configFile = configFile;
    this.backupDir = backupDir;
    this.fileSystem = fileSystem;
    this.config = cfgFileImpl;
  }

  public File getConfigFile() {
    return configFile;
  }

  public FileSystem getFileSystem() {
    return fileSystem;
  }

  private File createBackup(File file, File backupDir) throws AdminException {
    if (!fileSystem.exists(backupDir)) {
      fileSystem.mkdirs(backupDir);
    }
    File backupFile = new File(backupDir, file.getName() + "." + sdf.format(new Date()));
    if (fileSystem.exists(file)) {
      fileSystem.copy(file, backupFile);
    }
    return backupFile;
  }

  private boolean rollbackConfig(File file, File configFile) throws AdminException {
    if (fileSystem.exists(file)) {
      fileSystem.copy(file, configFile);
      return true;
    }
    return false;
  }

  public File save(C conf) throws AdminException {

    File backup = createBackup(configFile, backupDir);

    InputStream is = null;
    OutputStream os = null;
    File tmpConfigFile = new File(configFile.getAbsolutePath() + ".tmp");
    try {
      if (fileSystem.exists(configFile))
        is = fileSystem.getInputStream(configFile);
      os = fileSystem.getOutputStream(tmpConfigFile, false);

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
    return backup;
  }

  public boolean rollback(File backupFile) throws AdminException {
    return rollbackConfig(backupFile, configFile);
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
