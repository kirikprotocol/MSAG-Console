package mobi.eyeline.informer.web.config;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ConfigException extends AdminException {
  public ConfigException(String key, Throwable cause) {
    super(key, cause);
  }

  public ConfigException(String key, String ... args) {
    super(key, args);
  }
}
