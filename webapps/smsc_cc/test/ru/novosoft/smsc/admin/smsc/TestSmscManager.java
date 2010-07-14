package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * Тестовая реализация SmscConfig
 * @author Artem Snopkov
 */
public class TestSmscManager extends SmscManager {

  private final TestConfigFileManagerHelper helper;

  public TestSmscManager() throws AdminException {
    super(null, null, null);
    helper = new TestConfigFileManagerHelper(TestSmscManager.class.getResourceAsStream("config.xml"));
  }

  @Override
  public void apply() throws AdminException {
    helper.apply(this);
  }

  @Override
  public void reset() throws AdminException {
    helper.reset(this);
  }
}
