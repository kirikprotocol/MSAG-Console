package mobi.eyeline.informer.admin;

/**
 * Ошибка чтения web-конфига
 *
 * @author Aleksandr Khalitov
 */
class WebConfigException extends AdminException {
  WebConfigException(String key, Throwable cause) {
    super(key, cause);
  }

  protected WebConfigException(String key) {
    super(key);
  }

  WebConfigException(String key, String... args) {
    super(key, args);
  }
}
