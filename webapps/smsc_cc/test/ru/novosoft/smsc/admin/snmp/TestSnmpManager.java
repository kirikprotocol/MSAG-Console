package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestSnmpManager extends SnmpManager {

  private TestConfigFileManagerHelper helper;

  public TestSnmpManager(ClusterController cc) throws AdminException {
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
      helper = new TestConfigFileManagerHelper(TestSnmpManager.class.getResourceAsStream("snmp.xml"));
    }
    return helper;
  }
}
