package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.TestConfigFileManagerHelper;

/**
 * Тестовая реализация ArchiveDaemonConfig
 *
 * @author Artem Snopkov
 */
public class TestArchiveDaemonManager extends ArchiveDaemonManager {

  private TestConfigFileManagerHelper helper;

  public TestArchiveDaemonManager() throws AdminException {
    super(null, null, null);
  }

  public void reset() throws AdminException {
    getHelper().reset(this);
  }

  public void apply() throws AdminException {
    getHelper().apply(this);
  }

  private TestConfigFileManagerHelper getHelper() throws AdminException {
    if (helper == null) {
      helper = new TestConfigFileManagerHelper(ArchiveDaemonManager.class.getResourceAsStream("config.xml"));
    }
    return helper;
  }
}
