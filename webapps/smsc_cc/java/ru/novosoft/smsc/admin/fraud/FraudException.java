package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class FraudException extends AdminException {
  protected FraudException(String key, Throwable cause) {
    super(key, cause);
  }

  protected FraudException(String key) {
    super(key);
  }

  protected FraudException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
