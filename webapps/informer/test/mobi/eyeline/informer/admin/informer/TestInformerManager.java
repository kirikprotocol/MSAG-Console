package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.service.ServiceManager;

import java.io.File;

/**
 * @author Aleksandr Khalitov
 */
public class TestInformerManager extends InformerManagerImpl {

  public TestInformerManager(File configFile, File backupDir, FileSystem fs, ServiceManager serviceManager) throws AdminException {
    super(configFile, backupDir, fs, serviceManager);
  }
  
}
