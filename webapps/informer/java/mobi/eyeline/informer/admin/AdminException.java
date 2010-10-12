package mobi.eyeline.informer.admin;

import java.text.MessageFormat;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * Ошибка web приложения
 * @author Aleksandr Khalitov
 */
public abstract class AdminException extends Exception {

  protected final String bundleName;
  protected final String key;

  protected final String[] args;

  protected AdminException(String bundleName, String key, String ... args) {
    this.bundleName = bundleName;
    this.key = key;
    this.args = args;
  }

  protected AdminException(String bundleName, String key, Throwable cause, String ... args) {
    super(cause);
    this.bundleName = bundleName;
    this.key = key;
    this.args = args;
  }

  protected AdminException(String key, Throwable cause, String ... args) {
    super(cause);
    this.bundleName = getClass().getName();
    this.key = key;
    this.args = args;
  }

  protected AdminException(String key, String ... args) {
    this.bundleName = getClass().getName();
    this.key = key;
    this.args = args;
  }

  public String getMessage(Locale locale) {
    String result = ResourceBundle.getBundle(bundleName, locale).getString(key);
    if(args != null && args.length > 0) {
      if(result.indexOf("{")>=0) {
        result = MessageFormat.format(result, (Object[])args);
      }else {
        for(String a : args) {
          result+=" "+a;
        }
      }
    }
    if (getCause() != null) {
      Throwable ct = getCause();
      if (ct instanceof AdminException) {
        result += ". " + ((AdminException)ct).getMessage(locale);
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
