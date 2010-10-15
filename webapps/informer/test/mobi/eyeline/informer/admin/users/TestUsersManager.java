package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;

import java.io.File;

/**
 * author: alkhal
 */
public class TestUsersManager extends UsersManager {

  public TestUsersManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {
    super(infosme,  config, backup, fileSystem);
  }
  
}
