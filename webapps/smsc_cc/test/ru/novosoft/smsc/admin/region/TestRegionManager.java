package ru.novosoft.smsc.admin.region;

import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestRegionManager extends RegionManagerImpl {
  public TestRegionManager(File configFile, File backupDir, FileSystem fs) {
    super(configFile, backupDir, fs);
  }
}
