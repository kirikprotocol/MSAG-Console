package ru.novosoft.smsc.admin.config;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ConfigException extends AdminException {
  public ConfigException(String key, Throwable cause) {
    super(key, cause);
  }

  public ConfigException(String key) {
    super(key);
  }

  public ConfigException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
