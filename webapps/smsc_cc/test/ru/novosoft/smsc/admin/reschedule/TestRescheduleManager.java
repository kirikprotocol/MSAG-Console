package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

/**
 * @author Artem Snopkov
 */
public class TestRescheduleManager extends RescheduleManager {

  private TestConfigFileManagerHelper helper;

  public TestRescheduleManager() throws AdminException {
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
      helper = new TestConfigFileManagerHelper(TestRescheduleManager.class.getResourceAsStream("schedule.xml"));
    }
    return helper;
  }
}
