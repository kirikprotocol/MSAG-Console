package ru.novosoft.smsc.admin.util;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ValidationHelper {

  private final String paramNameBundle;

  public ValidationHelper(String paramNameBundle) {
    this.paramNameBundle = paramNameBundle;
  }

  public void checkPositive(String argName, int value) throws AdminException {
    if (value <= 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkNegative(String argName, int value) throws AdminException {
    if (value >= 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public <T> void checkGreaterThan(String argName, Comparable<T> value, T limit) throws AdminException {
    if (value.compareTo(limit) <= 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public <T> void checkGreaterOrEqualsTo(String argName, Comparable<T> value, T limit) throws AdminException {
    if (value.compareTo(limit) < 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public <T> void checkLessThan(String argName, Comparable<T> value, T limit) throws AdminException {
    if (value.compareTo(limit) >= 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public <T> void checkLessOrEqualsTo(String argName, Comparable<T> value, T limit) throws AdminException {
    if (value.compareTo(limit) > 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public <T> void checkEquals(String argName, Comparable<T> value, T limit) throws AdminException {
    if (value.compareTo(limit) != 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public <T> void checkBetween(String argName, Comparable<T> value, T min, T max) throws AdminException {
    if (value.compareTo(min) < 0 || value.compareTo(max) > 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkNotEmpty(String argName, String value) throws ValidationException {
    if (value == null || value.trim().length() == 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkNotEmpty(String argName, String[] value) throws ValidationException {
    if (value == null)
      throw new ValidationException(paramNameBundle, argName);

    for (String val : value) {
      if (val == null || val.trim().length() == 0)
        throw new ValidationException(paramNameBundle, argName);
    }

  }

  public void checkPort(String argName, int value) throws ValidationException {
    if (value < 0 || value > 65535)
      throw new ValidationException(paramNameBundle, argName);
  }

}
