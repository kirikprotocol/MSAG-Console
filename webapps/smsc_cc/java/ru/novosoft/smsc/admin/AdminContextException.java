package ru.novosoft.smsc.admin;

/**
 * @author Artem Snopkov
 */
public class AdminContextException extends AdminException {
  protected AdminContextException(String key, Throwable cause) {
    super(key, cause);
  }

  protected AdminContextException(String key) {
    super(key);
  }

  protected AdminContextException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
