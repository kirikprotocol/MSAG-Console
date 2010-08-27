package ru.novosoft.smsc.admin.users;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

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
