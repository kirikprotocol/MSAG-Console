package ru.novosoft.smsc.admin.provider;

import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestProviderManager extends ProviderManager {
  public TestProviderManager(File configFile, File backupDir, FileSystem fileSystem) {
    super(configFile, backupDir, fileSystem);
  }
}
