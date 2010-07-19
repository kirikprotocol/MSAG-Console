package ru.novosoft.smsc.admin.util;

import ru.novosoft.smsc.admin.AdminException;

import java.util.Locale;
import java.util.ResourceBundle;

/**
 * @author Artem Snopkov
 */
public class ValidationException extends AdminException {

  private final String paramNameBundle;

  protected ValidationException(String paramNameBundle, String paramName) {
    super(paramName);
    this.paramNameBundle = paramNameBundle;
  }

  public String getMessage(Locale locale) {
    String errorText = ResourceBundle.getBundle(bundleName, locale).getString("invalid_param_value");
    String paramName = ResourceBundle.getBundle(paramNameBundle, locale).getString(key);
    return errorText + ": " + paramName;
  }
}
