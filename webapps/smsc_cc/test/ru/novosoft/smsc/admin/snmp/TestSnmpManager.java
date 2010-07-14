package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestSnmpManager extends SnmpManager{

  private final TestConfigFileManagerHelper helper;

  public TestSnmpManager() throws AdminException {
    super(null, null, null, null);
    helper = new TestConfigFileManagerHelper(TestSnmpManager.class.getResourceAsStream("snmp.xml"));
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
