package ru.novosoft.smsc.admin.map_limit;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestMapLimitManager extends MapLimitManager {

  private final TestConfigFileManagerHelper helper;

  public TestMapLimitManager() throws AdminException {
    super(null, null, null, null);
    helper = new TestConfigFileManagerHelper(TestMapLimitManager.class.getResourceAsStream("maplimits.xml"));
  }

  public void reset() throws AdminException {
    helper.reset(this);
  }

  public void apply() throws AdminException {
    helper.apply(this);
  }
}
