package mobi.eyeline.informer.admin;

import java.util.Locale;
import java.util.ResourceBundle;

public abstract class AdminException extends Exception {

  protected final String bundleName;
  protected final String key;
  protected String causeMessage;

  protected AdminException(String key, Throwable cause) {
    super(cause);
    this.bundleName = getClass().getName();
    this.key = key;
  }

  protected AdminException(String key) {
    this.bundleName = getClass().getName();
    this.key = key;
  }

  protected AdminException(String key, String causeMessage) {
    this.bundleName = getClass().getName();
    this.key = key;
    this.causeMessage = causeMessage;
  }

  public String getMessage(Locale locale) {
    String result = ResourceBundle.getBundle(bundleName, locale).getString(key);
    if (causeMessage != null)
      result += ". " + causeMessage;
    else if (getCause() != null) {
      Throwable ct = getCause();
      if (ct instanceof AdminException) {
        result += ". " + ((AdminException)ct).getMessage(locale);
      } else {
        result += ". " + ct.getMessage();
      }
    }
    return  result;
  }

  public String getMessage() {
    return getMessage(Locale.ENGLISH);
  }
}
