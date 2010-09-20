package mobi.eyeline.informer.admin.config;

import mobi.eyeline.informer.admin.AdminException;

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
