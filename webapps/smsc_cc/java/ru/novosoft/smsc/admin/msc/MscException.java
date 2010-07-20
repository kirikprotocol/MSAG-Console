package ru.novosoft.smsc.admin.msc;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class MscException extends AdminException {
  protected MscException(String key, Throwable cause) {
    super(key, cause);
  }

  protected MscException(String key) {
    super(key);
  }

  protected MscException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
