package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Ошибка чтения web-конфига
 * @author Aleksandr Khalitov
 */
class WebConfigException extends AdminException {
  WebConfigException(String key, Throwable cause) {
    super(key, cause);
  }

  protected WebConfigException(String key) {
    super(key);
  }

  WebConfigException(String key, String ... args) {
    super(key, args);
  }
}
