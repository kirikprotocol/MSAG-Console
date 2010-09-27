package mobi.eyeline.informer.admin.util.config;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Ошибка чтения/записи конфига
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
