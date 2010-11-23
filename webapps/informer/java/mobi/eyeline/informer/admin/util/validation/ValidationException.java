package mobi.eyeline.informer.admin.util.validation;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Ошибка валидации
 *
 * @author Artem Snopkov
 */
public class ValidationException extends AdminException {

  ValidationException(String bundleName, String key, String... args) {
    super(bundleName, key, args);
  }

}
