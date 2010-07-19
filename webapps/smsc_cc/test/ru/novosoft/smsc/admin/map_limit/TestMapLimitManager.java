package ru.novosoft.smsc.admin.map_limit;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestMapLimitManager extends MapLimitManager {

  private TestConfigFileManagerHelper helper;

  public TestMapLimitManager() throws AdminException {
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
      helper = new TestConfigFileManagerHelper(TestMapLimitManager.class.getResourceAsStream("maplimits.xml"));
    }
    return helper;
  }
}
