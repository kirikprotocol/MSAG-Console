package mobi.eyeline.informer.admin.util.validation;

import mobi.eyeline.informer.admin.AdminException;

import java.util.ResourceBundle;

/**
 * Ошибка валидации
 *
 * @author Artem Snopkov
 */
public class ValidationException extends AdminException {

  ValidationException(String bundleName, String field) {
    super(ValidationException.class.getCanonicalName(), "invalid_param_value", 
        new String[]{ResourceBundle.getBundle(bundleName).getString(field)});
  }

}
