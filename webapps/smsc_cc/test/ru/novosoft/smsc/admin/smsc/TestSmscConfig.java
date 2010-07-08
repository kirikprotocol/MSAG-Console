package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * Тестовая реализация SmscConfig
 * @author Artem Snopkov
 */
public class TestSmscConfig extends SmscConfig {

  public TestSmscConfig() throws AdminException {
    super(null, null, null);
  }

  protected SmscConfigFile createSmscConfigFile(File smscConfigFile, File backupDir, FileSystem fileSystem) {
    return new TestSmscConfigFile();
  }
}
