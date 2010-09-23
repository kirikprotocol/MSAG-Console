package ru.novosoft.smsc.admin.operative_store;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class OperativeStoreException extends AdminException {
  protected OperativeStoreException(String key, Throwable cause) {
    super(key, cause);
  }

  protected OperativeStoreException(String key) {
    super(key);
  }

  protected OperativeStoreException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
