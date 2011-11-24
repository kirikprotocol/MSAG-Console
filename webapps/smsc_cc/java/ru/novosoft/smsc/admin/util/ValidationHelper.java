package ru.novosoft.smsc.admin.util;

import ru.novosoft.smsc.admin.AdminException;

import java.util.Map;
import java.util.Map.Entry;
import java.util.regex.Pattern;

/**
 * @author Artem Snopkov
 */
@SuppressWarnings({"ClassWithTooManyMethods"})
public class ValidationHelper {

  private final String paramNameBundle;

  public ValidationHelper(String paramNameBundle) {
    this.paramNameBundle = paramNameBundle;
  }

  public ValidationHelper(Class clazz) {
    this(clazz.getCanonicalName());
  }

  public void checkPositive(String argName, int value) throws AdminException {
    if (value <= 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkPositive(String argName, int[] value) throws AdminException {
    for (int val : value)
      if (val <= 0)
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

  public <T> void checkNotEquals(String argName, Comparable<T> value1, T value2) throws AdminException {
    if (value1.compareTo(value2) == 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public <T> void checkBetween(String argName, Comparable<T> value, T min, T max) throws AdminException {
    if (value.compareTo(min) < 0 || value.compareTo(max) > 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkNotEmpty(String argName, String value) throws AdminException {
    if (value == null || value.trim().length() == 0)
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkLen(String argName, String value, int minLen, int maxLen) throws AdminException {
    checkNotEmpty(argName, value);
    if (value.length() < minLen || value.length() > maxLen)
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkNotEmpty(String argName, String[] value) throws AdminException {
    if (value == null)
      throw new ValidationException(paramNameBundle, argName);

    for (String val : value) {
      if (val == null || val.trim().length() == 0)
        throw new ValidationException(paramNameBundle, argName);
    }
  }

  public void checkNoNulls(String argName, Map value) throws AdminException {
    for (Object o : value.entrySet()) {
      Entry e = (Entry) o;
      if (e.getKey() == null || e.getValue() == null)
        throw new ValidationException(paramNameBundle, argName);
    }
  }

  public void checkNoNulls(String argName, Iterable value) throws AdminException {
    for (Object o : value)
      if (o == null)
        throw new ValidationException(paramNameBundle, argName);
  }

  public void checkPort(String argName, int value) throws AdminException {
    if (value < 0 || value > 65535)
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkMaches(String argName, CharSequence value, Pattern pattern) throws AdminException {
    if (!pattern.matcher(value).matches())
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkNotIntersect(String argName, Iterable value, Iterable withValue) throws AdminException {
    for (Object o : value) {
      for (Object b : withValue)
        if (o.equals(b))
          throw new ValidationException(paramNameBundle, argName);
    }
  }

  public void checkSize(String argName, Object[] value, int size) throws AdminException {
    if (value.length != size)
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkNotNull(String argName, Object value) throws AdminException {
    if (value == null)
      throw new ValidationException(paramNameBundle, argName);
  }

  public void checkIn(String argName, Object value, Object[] values) throws AdminException {
    if(value != null) {
      for(Object o : values) {
        if(value.equals(o)) {
          return;
        }
      }
    }
    throw new ValidationException(paramNameBundle, argName);
  }

}
