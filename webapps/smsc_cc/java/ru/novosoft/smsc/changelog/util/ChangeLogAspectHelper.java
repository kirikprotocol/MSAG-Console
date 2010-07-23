package ru.novosoft.smsc.changelog.util;

import org.aspectj.lang.ProceedingJoinPoint;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class ChangeLogAspectHelper {

  private static String firstCharToUpperCase(String fieldName) {
    if (Character.isLowerCase(fieldName.charAt(0))) {
      char ch = fieldName.charAt(0);
      if (fieldName.length() == 1)
        fieldName = String.valueOf(Character.toUpperCase(ch));
      else
        fieldName = Character.toUpperCase(ch) + fieldName.substring(1);
    }
    return fieldName;
  }

  public static Method getGetter(Class clazz, String fieldName) {

    fieldName = firstCharToUpperCase(fieldName);

    Method getter = null;
    try {
      getter = clazz.getMethod("get" + fieldName);
    } catch (NoSuchMethodException ignored) {
    }

    if (getter == null)
      try {
        getter = FraudManager.class.getMethod("is" + fieldName);
      } catch (NoSuchMethodException e) {
      }

    return getter;
  }

  public static Method getSetter(Class clazz, String fieldName) {
    fieldName = firstCharToUpperCase(fieldName);
    Method setter = null;
    try {
      setter = clazz.getMethod("set" + fieldName);
    } catch (NoSuchMethodException ignored) {
    }
    return setter;
  }

  public static List<Method> getGetters(Class clazz) {
    List<Method> getters = new ArrayList<Method>();
    for (Method m : clazz.getMethods()) {
      String methodName = m.getName();
      if (methodName.startsWith("get") || methodName.startsWith("is"))
        getters.add(m);
    }
    return getters;
  }

  public static Object invokeGetter(Class beanClass, Object beanInstance, String fieldName) throws Exception {
    Method getter = getGetter(beanClass, fieldName);
    if (getter == null)
      throw new Exception("No getter");

    try {
      return getter.invoke(beanInstance);
    } catch (Exception e) {
      throw new Exception("Invokation exception", e);
    }
  }

  protected void _logSetter(Object beanInstance, ChangeLog changeLog) {
    

  }
}
