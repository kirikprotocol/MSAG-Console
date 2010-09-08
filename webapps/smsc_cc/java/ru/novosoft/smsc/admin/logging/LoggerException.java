package ru.novosoft.smsc.admin.logging;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class LoggerException extends AdminException {
  protected LoggerException(String key, Throwable cause) {
    super(key, cause);
  }

  protected LoggerException(String key) {
    super(key);
  }

  protected LoggerException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
