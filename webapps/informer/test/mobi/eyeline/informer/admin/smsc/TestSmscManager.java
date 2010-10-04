package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;

import java.io.File;

/**
 * @author Aleksandr Khalitov
 */
public class TestSmscManager extends SmscManager{

  public TestSmscManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {
    super(infosme, config, backup, fileSystem);
  }
  
}
