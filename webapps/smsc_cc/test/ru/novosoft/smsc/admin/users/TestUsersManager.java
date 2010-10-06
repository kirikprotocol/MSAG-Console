package ru.novosoft.smsc.admin.users;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * author: alkhal
 */
public class TestUsersManager extends UsersManagerImpl {

  public TestUsersManager(File configFile, File backupDir, FileSystem fs) throws AdminException {
    super(configFile, backupDir, fs);
  }
  
}
