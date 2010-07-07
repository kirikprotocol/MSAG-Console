package ru.novosoft.smsc.admin.service.daemon;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class DaemonException extends AdminException {
  protected DaemonException(String key, Throwable cause) {
    super(key, cause);
  }

  protected DaemonException(String key) {
    super(key);
  }

  protected DaemonException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
