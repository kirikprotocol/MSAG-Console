package ru.novosoft.smsc.admin.protocol;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ProtocolException extends AdminException {
  protected ProtocolException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ProtocolException(String key) {
    super(key);
  }

  protected ProtocolException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
