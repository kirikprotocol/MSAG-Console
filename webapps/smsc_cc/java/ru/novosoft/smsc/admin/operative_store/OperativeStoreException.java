package ru.novosoft.smsc.admin.operative_store;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class OperativeStoreException extends AdminException {
  OperativeStoreException(String key, Throwable cause) {
    super(key, cause);
  }

  OperativeStoreException(String key) {
    super(key);
  }

  OperativeStoreException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
