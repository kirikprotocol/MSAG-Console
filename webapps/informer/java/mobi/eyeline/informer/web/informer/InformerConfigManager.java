package mobi.eyeline.informer.web.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.web.config.ConfigFileManager;

import java.io.File;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * @author Aleksandr Khalitov
 */
public class InformerConfigManager {

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private final ConfigFileManager<InformerSettings> cfgFileManager;

  public InformerConfigManager(File configFile, File backupDir, FileSystem fs) throws AdminException {
    cfgFileManager = new ConfigFileManager<InformerSettings>(configFile, backupDir, fs, new InformerConfig());
  }

  public InformerSettings getConfigSettings() throws AdminException {
    try{
      lock.readLock().lock();
      return cfgFileManager.load();
    }finally {
      lock.readLock().unlock();
    }
  }

  public void updateSettings(InformerSettings informerSettings) throws AdminException{
    try{
      lock.writeLock().lock();
      cfgFileManager.save(informerSettings);
    }finally {
      lock.writeLock().unlock();
    }

  }
}
