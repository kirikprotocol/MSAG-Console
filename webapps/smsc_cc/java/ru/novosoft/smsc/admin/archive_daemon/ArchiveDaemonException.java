package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ArchiveDaemonException extends AdminException {

  protected ArchiveDaemonException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ArchiveDaemonException(String key) {
    super(key);
  }
}
