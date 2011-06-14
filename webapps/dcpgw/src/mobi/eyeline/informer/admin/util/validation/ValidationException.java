package mobi.eyeline.informer.admin.util.validation;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.LocalizedException;

import java.text.MessageFormat;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * Ошибка валидации
 *
 * @author Artem Snopkov
 */
public class ValidationException extends AdminException {

  private final String argsBundleName;

  ValidationException(String argsBundleName, String... args) {
    super(ValidationException.class.getCanonicalName(), "invalid_param_value", args);
    this.argsBundleName = argsBundleName;
  }

  @Override
  public String getMessage(Locale locale) {
    String result = ResourceBundle.getBundle(bundleName, locale).getString(key);
    if (args != null && args.length > 0) {
      String[] localizedArgs = new String[args.length];
      ResourceBundle argsBundle = ResourceBundle.getBundle(argsBundleName, locale);
      int i = 0;
      for(String a : args) {
        localizedArgs[i] = argsBundle.getString(a);
        i++;
      }

      if (result.indexOf("{") >= 0) {
        result = MessageFormat.format(result, (Object[]) localizedArgs);
      } else {
        for (String a : localizedArgs) {
          result += " " + a;
        }
      }
    }
    if (getCause() != null) {
      Throwable ct = getCause();
      if (ct instanceof LocalizedException) {
        result += ". " + ((LocalizedException) ct).getMessage(locale);
      } else {
        result += ". " + ct.getMessage();
      }
    }
    return result;

  }


}
