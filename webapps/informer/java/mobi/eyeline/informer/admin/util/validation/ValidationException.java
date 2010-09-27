package mobi.eyeline.informer.admin.util.validation;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Ошибка валидации
 * @author Artem Snopkov
 */
public class ValidationException extends AdminException {

  protected ValidationException(String bundleName, String key,  String ... args) {
    super(bundleName, key, args);
  }

}
