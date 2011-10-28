package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.util.DBExportSettings;

/**
 * Тестовая реализация ArchiveDaemonConfig
 *
 * @author Artem Snopkov
 */
public class TestArchiveDaemonManager extends ArchiveDaemonManagerImpl {

  public TestArchiveDaemonManager(ServiceManager serviceManager, FileSystem fs, DBExportSettings exportSettings) throws AdminException {
    super(serviceManager, fs, exportSettings);
  }
}
