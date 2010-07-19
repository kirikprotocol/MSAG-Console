package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

/**
 * Тестовая реализация FraudManager
 *
 * @author Artem Snopkov
 */
public class TestFraudManager extends FraudManager {

  private TestConfigFileManagerHelper helper;

  public TestFraudManager() throws AdminException {
    super(null, null, null, null);
  }

  public void reset() throws AdminException {
    getHelper().reset(this);
  }

  public void apply() throws AdminException {
    getHelper().apply(this);
  }

  private TestConfigFileManagerHelper getHelper() throws AdminException {
    if (helper == null) {
      helper = new TestConfigFileManagerHelper(TestFraudManager.class.getResourceAsStream("fraud.xml"));
    }
    return helper;
  }
}
