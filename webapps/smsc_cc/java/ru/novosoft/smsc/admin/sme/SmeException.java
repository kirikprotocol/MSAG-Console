package ru.novosoft.smsc.admin.sme;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class SmeException extends AdminException {
  protected SmeException(String key, Throwable cause) {
    super(key, cause);
  }

  protected SmeException(String key) {
    super(key);
  }

  protected SmeException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
