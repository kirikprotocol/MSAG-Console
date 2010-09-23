package mobi.eyeline.informer.web.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Aleksandr Khalitov
 */
public class TestInformerConfigManager extends InformerConfigManager {

  public TestInformerConfigManager(File configFile, File backupDir, FileSystem fs) throws AdminException {
    super(configFile, backupDir, fs);
  }
  
}
