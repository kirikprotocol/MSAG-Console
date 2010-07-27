package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

import java.util.HashMap;
import java.util.Map;

/**
 * Тестовая реализация FraudManager
 *
 * @author Artem Snopkov
 */
public class TestFraudManager extends FraudManager {

  private TestConfigFileManagerHelper helper;

  public TestFraudManager(ClusterController cc) throws AdminException {
    super(null, null, cc, null);
    helper = new TestConfigFileManagerHelper(TestFraudManager.class.getResourceAsStream("fraud.xml"));
  }

  public void reset() throws AdminException {
    helper.reset(this);
  }

  public void apply() throws AdminException {
    helper.apply(this);
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    Map<Integer, SmscConfigurationStatus> res = new HashMap<Integer, SmscConfigurationStatus>();
    res.put(0, SmscConfigurationStatus.UP_TO_DATE);
    res.put(1, SmscConfigurationStatus.UP_TO_DATE);
    return res;
  }
}
