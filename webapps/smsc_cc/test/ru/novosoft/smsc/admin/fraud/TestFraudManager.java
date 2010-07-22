package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

/**
 * Тестовая реализация FraudManager
 *
 * @author Artem Snopkov
 */
public class TestFraudManager extends FraudManager {

  private TestConfigFileManagerHelper helper;

  public TestFraudManager(ClusterController cc) throws AdminException {
    super(null, null, cc, null);
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
