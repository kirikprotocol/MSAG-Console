package ru.novosoft.smsc.web.journal;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;

/**
 * @author Artem Snopkov
 */
class DiffHelper {

  protected final String subject;

  public DiffHelper(String subject) {
    this.subject = subject;
  }

  protected static List<Method> getGetters(Class clazz) {
    return getGetters(clazz, null);
  }

  protected static List<Method> getGetters(Class clazz, Set<String> except) {
    List<Method> getters = new ArrayList<Method>();
    for (Method m : clazz.getMethods()) {
      String methodName = m.getName();
      if ((methodName.startsWith("get") || methodName.startsWith("is")) && (except == null || !except.contains(methodName))) {
        getters.add(m);
      }
    }
    return getters;
  }

  protected static List<Object> callGetters(List<Method> getters, Object instance) {
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

  private static String valueToString(Object value) {
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

  protected void logChanges(Journal j, List<Object> oldValues, List<Object> newValues, List<Method> getters, String user, String bundleMessage, String... addParameters) {
    for (int i = 0; i < getters.size(); i++) {
      Object oldValue = oldValues.get(i);
      Object newValue = newValues.get(i);

      boolean changed = oldValue == null && newValue != null;
      changed = changed || (oldValue != null && newValue == null);

      if (!changed && oldValue != null) {
        if (oldValue instanceof Object[]) {
          changed = !Arrays.equals((Object[]) oldValue, (Object[]) newValue);
        } else
          changed = !oldValue.equals(newValue);
      }

      if (changed) {
        Method getter = getters.get(i);
        String propertyName = getter.getName().substring(3);
        char firstChar = propertyName.charAt(0);
        if (propertyName.length() > 1)
          propertyName = Character.toLowerCase(firstChar) + propertyName.substring(1);
        else
          propertyName = Character.toLowerCase(firstChar) + "";

        String[] args = new String[addParameters.length + 3];
        args[0] = propertyName;
        args[1] = valueToString(oldValue);
        args[2] = valueToString(newValue);
        System.arraycopy(addParameters, 0, args, 3, addParameters.length);

        j.addRecord(JournalRecord.Type.CHANGE, subject, user).setDescription(bundleMessage, args);
      }
    }

  }

  protected void logChanges(Journal j, List<Object> oldValues, List<Object> newValues, List<Method> getters, String user) {
    logChanges(j, oldValues, newValues, getters, user, "property_changed");
  }

}
