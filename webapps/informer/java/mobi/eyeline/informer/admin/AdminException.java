package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.util.LocalizedException;

/**
 * Ошибка web приложения
 * @author Aleksandr Khalitov
 */
public abstract class AdminException extends LocalizedException {

  protected AdminException(String bundleName, String key, String... args) {
    super(bundleName, key, args);
  }

  protected AdminException(String bundleName, String key, Throwable cause, String... args) {
    super(bundleName, key, cause, args);
  }

  protected AdminException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  protected AdminException(String key, String... args) {
    super(key, args);
  }
}
