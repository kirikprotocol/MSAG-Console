package ru.novosoft.smsc.admin.provider;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ProviderException extends AdminException {
  protected ProviderException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ProviderException(String key) {
    super(key);
  }

  protected ProviderException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
