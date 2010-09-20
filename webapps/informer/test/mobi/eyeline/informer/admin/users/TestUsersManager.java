package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;

/**
 * author: alkhal
 */
public class TestUsersManager extends UsersManager {

  public TestUsersManager(File configFile, File backupDir, FileSystem fs) throws AdminException {
    super(configFile, backupDir, fs);
  }
  
}
