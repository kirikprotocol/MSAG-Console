package ru.novosoft.smsc.admin.timezone;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class TimezoneException extends AdminException {
  protected TimezoneException(String key, Throwable cause) {
    super(key, cause);
  }

  protected TimezoneException(String key) {
    super(key);
  }

  protected TimezoneException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
