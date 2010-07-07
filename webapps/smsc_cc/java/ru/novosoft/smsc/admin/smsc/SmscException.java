package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class SmscException extends AdminException {
  protected SmscException(String key, Throwable cause) {
    super(key, cause);
  }

  protected SmscException(String key) {
    super(key);
  }

  protected SmscException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
