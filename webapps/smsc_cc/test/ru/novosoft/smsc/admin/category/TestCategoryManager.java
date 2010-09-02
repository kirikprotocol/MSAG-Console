package ru.novosoft.smsc.admin.category;

import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestCategoryManager extends CategoryManager {
  public TestCategoryManager(File configFile, File backupDir, FileSystem fileSystem) {
    super(configFile, backupDir, fileSystem);
  }
}
