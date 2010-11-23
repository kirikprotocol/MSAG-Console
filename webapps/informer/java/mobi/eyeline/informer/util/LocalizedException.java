package mobi.eyeline.informer.util;

import java.text.MessageFormat;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * @author Artem Snopkov
 */
public class LocalizedException extends Exception {


  private final String bundleName;
  private final String key;

  private final String[] args;

  protected LocalizedException(String bundleName, String key, String... args) {
    this.bundleName = bundleName;
    this.key = key;
    this.args = args;
  }

  protected LocalizedException(String bundleName, String key, Throwable cause, String... args) {
    super(cause);
    this.bundleName = bundleName;
    this.key = key;
    this.args = args;
  }

  protected LocalizedException(String key, Throwable cause, String... args) {
    super(cause);
    this.bundleName = getClass().getName();
    this.key = key;
    this.args = args;
  }

  protected LocalizedException(String key, String... args) {
    this.bundleName = getClass().getName();
    this.key = key;
    this.args = args;
  }

  public String getMessage(Locale locale) {
    String result = ResourceBundle.getBundle(bundleName, locale).getString(key);
    if (args != null && args.length > 0) {
      if (result.indexOf("{") >= 0) {
        result = MessageFormat.format(result, (Object[]) args);
      } else {
        for (String a : args) {
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

  public String getMessage() {
    return getMessage(Locale.ENGLISH);
  }
}
