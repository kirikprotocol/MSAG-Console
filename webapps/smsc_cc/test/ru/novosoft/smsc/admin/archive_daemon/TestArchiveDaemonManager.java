package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceManager;

/**
 * Тестовая реализация ArchiveDaemonConfig
 *
 * @author Artem Snopkov
 */
public class TestArchiveDaemonManager extends ArchiveDaemonManager {

  public TestArchiveDaemonManager(ServiceManager serviceManager, FileSystem fs) throws AdminException {
    super(serviceManager, fs);
  }
}
