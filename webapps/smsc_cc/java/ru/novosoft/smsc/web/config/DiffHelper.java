package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class DiffHelper {

  private static List<Method> getGetters(Class clazz, String... except) {
    List<Method> getters = new ArrayList<Method>();
    for (Method m : clazz.getMethods()) {
      String methodName = m.getName();
      if (methodName.startsWith("get") || methodName.startsWith("is")) {
        if (except != null) {
          boolean accept = true;
          for (String s : except) {
            if (s.equals(methodName)) {
              accept = false;
              break;
            }
          }
          if (!accept) {
            continue;
          }
        }
        getters.add(m);
      }
    }
    return getters;
  }

  /**
   * По очереди вызывает все геттеры из getters на instance и возвращает список с результатами
   *
   * @param getters  список геттеров, которые надо вызвать
   * @param instance экземпляр, на котором вызываются геттеры
   * @return список значений, которые вернули геттеры
   */
  public static List<Object> callGetters(List<Method> getters, Object instance) {
    List<Object> values = new ArrayList<Object>();
    for (Method getter : getters) {
      try {
        Object value = getter.invoke(instance);
        values.add(value);
      } catch (IllegalAccessException e) {
      } catch (InvocationTargetException e) {
      }
    }
    return values;
  }

  private static String collectionToString(Collection<String> values) {
    StringBuilder sb = new StringBuilder("[");
    int i = 0;
    for (String v : values) {
      if (i == 1) {
        sb.append(", ");
      } else
        i = 1;

      sb.append(v);
    }
    sb.append(']');

    return sb.toString();
  }

  public static String valueToString(Object value) {
    if (value == null)
      return "";

    if (value instanceof Collection) {
      ArrayList<String> res = new ArrayList<String>();
      for (Object v : (Collection) value)
        res.add(valueToString(v));
      return collectionToString(res);

    } else if (value instanceof Map) {
      ArrayList<String> res = new ArrayList<String>();
      for (Object v : (((Map) value).entrySet())) {
        Map.Entry e = (Map.Entry) v;
        res.add(valueToString(e.getKey()) + " : " + valueToString(e.getValue()));
      }
      return collectionToString(res);

    } else if (value instanceof Object[]) {
      ArrayList<String> res = new ArrayList<String>();
      for (Object v : (Object[]) value)
        res.add(valueToString(v));
      return collectionToString(res);

    } else if (value instanceof int[]) {
      ArrayList<String> res = new ArrayList<String>();
      for (int v : (int[]) value)
        res.add(String.valueOf(v));
      return collectionToString(res);

    } else if (value instanceof long[]) {
      ArrayList<String> res = new ArrayList<String>();
      for (long v : (long[]) value)
        res.add(String.valueOf(v));
      return collectionToString(res);

    } else if (value instanceof boolean[]) {
      ArrayList<String> res = new ArrayList<String>();
      for (boolean v : (boolean[]) value)
        res.add(String.valueOf(v));
      return collectionToString(res);

    }

    return value.toString();
  }

  private static boolean isEquals(Object oldValue, Object newValue) {
    boolean changed = oldValue == null && newValue != null;
    changed = changed || (oldValue != null && newValue == null);

    if (!changed && oldValue != null) {
      if (oldValue instanceof Object[]) {
        changed = !Arrays.equals((Object[]) oldValue, (Object[]) newValue);
      } else if (oldValue instanceof int[]) {
        changed = !Arrays.equals((int[]) oldValue, (int[]) newValue);
      } else if (oldValue instanceof long[]) {
        changed = !Arrays.equals((long[]) oldValue, (long[]) newValue);
      } else if (oldValue instanceof double[]) {
        changed = !Arrays.equals((double[]) oldValue, (double[]) newValue);
      } else if (oldValue instanceof float[]) {
        changed = !Arrays.equals((float[]) oldValue, (float[]) newValue);
      } else if (oldValue instanceof byte[]) {
        changed = !Arrays.equals((byte[]) oldValue, (byte[]) newValue);
      } else if (oldValue instanceof short[]) {
        changed = !Arrays.equals((short[]) oldValue, (short[]) newValue);
      } else if (oldValue instanceof char[]) {
        changed = !Arrays.equals((char[]) oldValue, (char[]) newValue);
      } else if (oldValue instanceof boolean[]) {
        changed = !Arrays.equals((boolean[]) oldValue, (boolean[]) newValue);
      } else {
        changed = !oldValue.equals(newValue);
      }
    }
    return !changed;
  }

  private static String extractPropertyNameFromGetterName(Method getter) {
    String propertyName;

    if (getter.getReturnType() != Boolean.TYPE)
      propertyName = getter.getName().substring(3);
    else
      propertyName = getter.getName().substring(2);

    char firstChar = propertyName.charAt(0);
    if (propertyName.length() > 1)
      propertyName = Character.toLowerCase(firstChar) + propertyName.substring(1);
    else
      propertyName = Character.toLowerCase(firstChar) + "";

    return propertyName;
  }

  private static void findChanges(List<Object> oldValues, List<Object> newValues, List<Method> getters, ChangeListener l) {
    for (int i = 0; i < getters.size(); i++) {
      Object oldValue = oldValues.get(i);
      Object newValue = newValues.get(i);

      if (!isEquals(oldValue, newValue)) {
        String propertyName = extractPropertyNameFromGetterName(getters.get(i));
        l.foundChange(propertyName, oldValue, newValue);
      }
    }
  }

  public static <T> void findChanges(T oldObj, T newObj, Class<T> clazz, ChangeListener l) {
    List<Method> getters = getGetters(clazz);
    List<Object> oldValues = callGetters(getters, oldObj);
    List<Object> newValues = callGetters(getters, newObj);
    findChanges(oldValues, newValues, getters, l);
  }

  public static <T> void findChanges(T oldObj, T newObj, Class<T> clazz, ChangeListener l, String... excludeProperties) {
    List<Method> getters = getGetters(clazz);
    for (Iterator<Method> iter = getters.iterator(); iter.hasNext();) {
      for (String prop : excludeProperties)
        if (prop.equals(extractPropertyNameFromGetterName(iter.next())))
          iter.remove();
    }
    List<Object> oldValues = callGetters(getters, oldObj);
    List<Object> newValues = callGetters(getters, newObj);
    findChanges(oldValues, newValues, getters, l);
  }

  public static interface ChangeListener {
    public void foundChange(String propertyName, Object oldValue, Object newValue);
  }

}
