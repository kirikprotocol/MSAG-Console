package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

/**
 * @author Artem Snopkov
 */
public class TestRescheduleManager extends RescheduleManager {

  private final TestConfigFileManagerHelper helper;

  public TestRescheduleManager() throws AdminException {
    super(null, null, null, null);
    helper = new TestConfigFileManagerHelper(TestRescheduleManager.class.getResourceAsStream("schedule.xml"));
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
