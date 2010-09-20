package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.config.ConfigFileManager;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * author: alkhal
 */
public class UsersManager {

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private final ConfigFileManager<UsersSettings> cfgFileManager;

  public UsersManager(File configFile, File backupDir, FileSystem fs) throws AdminException {
    cfgFileManager = new ConfigFileManager<UsersSettings>(configFile, backupDir, fs, new UsersConfig());
  }

  public UsersSettings getUsersSettings() throws AdminException{
    try{
      lock.readLock().lock();
      return cfgFileManager.load();
    }finally {
      lock.readLock().unlock();
    }
  }

  public void updateSettings(UsersSettings usersSettings) throws AdminException{
    try{
      lock.writeLock().lock();
      cfgFileManager.save(usersSettings);
    }finally {
      lock.writeLock().unlock();
    }

  }


}
