package ru.novosoft.smsc.admin.callable;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class CallableServiceException extends AdminException {
  protected CallableServiceException(String key, Throwable cause) {
    super(key, cause);
  }

  protected CallableServiceException(String key) {
    super(key);
  }

  protected CallableServiceException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
