package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

import java.io.File;

/**
 * Тестовая реализация SmscConfig
 *
 * @author Artem Snopkov
 */
public class TestSmscManager extends SmscManager {

  private TestConfigFileManagerHelper helper;

  public TestSmscManager() throws AdminException {
    super(null, null, null);
  }

  public void reset() throws AdminException {
    getHelper().reset(this);
  }

  public void apply() throws AdminException {
    getHelper().apply(this);
  }

  private TestConfigFileManagerHelper getHelper() throws AdminException {
    if (helper == null) {
      helper = new TestConfigFileManagerHelper(TestSmscManager.class.getResourceAsStream("config.xml"));
    }
    return helper;
  }
}
