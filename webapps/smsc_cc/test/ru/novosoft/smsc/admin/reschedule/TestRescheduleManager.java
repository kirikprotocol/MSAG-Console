package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class TestRescheduleManager extends RescheduleManager {
  public TestRescheduleManager() throws AdminException {
    super(null, null, null, null);
  }

  protected RescheduleConfig createConfig() {
    return new TestRescheduleConfig();
  }
}
