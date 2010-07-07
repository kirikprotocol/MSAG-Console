package ru.novosoft.smsc.admin;

import java.util.Locale;
import java.util.ResourceBundle;

public abstract class AdminException extends Exception {

  protected final String bundleName;
  protected final String key;
  protected final String causeMessage;

  protected AdminException(String key, Throwable cause) {
    super(cause);
    this.bundleName = getClass().getName();
    this.key = key;
    if (cause != null)
      this.causeMessage = cause.getMessage();
    else
      this.causeMessage = null;
  }

  protected AdminException(String key) {
    this.bundleName = getClass().getName();
    this.key = key;
    this.causeMessage = null;
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
    return  result;
  }

  public String getMessage() {
    return getMessage(Locale.ENGLISH);
  }
}
