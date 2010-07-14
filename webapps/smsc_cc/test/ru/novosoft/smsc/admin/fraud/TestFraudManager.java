package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

/**
 * Тестовая реализация FraudManager
 * @author Artem Snopkov
 */
public class TestFraudManager extends FraudManager {

  private final TestConfigFileManagerHelper helper;

  public TestFraudManager() throws AdminException {
    super(null, null, null, null);
    helper = new TestConfigFileManagerHelper(TestFraudManager.class.getResourceAsStream("fraud.xml"));
  }

  public void reset() throws AdminException {
    helper.reset(this);
  }

  public void apply() throws AdminException {
    helper.apply(this);
  }
}
