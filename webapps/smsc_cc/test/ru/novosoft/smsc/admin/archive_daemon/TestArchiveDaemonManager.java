package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

/**
 * Тестовая реализация ArchiveDaemonConfig
 * @author Artem Snopkov
 */
public class TestArchiveDaemonManager extends ArchiveDaemonManager {

  private final TestConfigFileManagerHelper helper;

  public TestArchiveDaemonManager() throws AdminException {
    super(null, null, null);
    helper = new TestConfigFileManagerHelper(TestArchiveDaemonManager.class.getResourceAsStream("config.xml"));
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
