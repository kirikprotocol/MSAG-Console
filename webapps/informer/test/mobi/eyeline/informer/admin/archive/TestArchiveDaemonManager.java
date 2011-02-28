package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.service.ServiceManager;

import java.io.File;

/**
 * @author Aleksandr Khalitov
 */
public class TestArchiveDaemonManager extends ArchiveDaemonManager{
  public TestArchiveDaemonManager(File configFile, File backupDir, FileSystem fs, ServiceManager serviceManager) {
    super(configFile, backupDir, fs, serviceManager);
  }
}
