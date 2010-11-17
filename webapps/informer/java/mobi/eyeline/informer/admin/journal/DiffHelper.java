package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;

/**
 * Helper для поиска и журналирования изменений
 * @author Artem Snopkov
 */
class DiffHelper {

  protected final Subject subject;

  public DiffHelper(Subject subject) {
    this.subject = subject;
  }

  /**
   * Возвращает все геттеры класса, за исключение excpet
   * @param clazz класс
   * @param except геттеры, котрые нужно исключить из результата
   * @return геттеры класса, за исключение excpet
   */
  protected static List<Method> getGetters(Class clazz, String ... except) {
    List<Method> getters = new ArrayList<Method>();
    for (Method m : clazz.getMethods()) {
      String methodName = m.getName();
      if (methodName.startsWith("get") || methodName.startsWith("is")) {
        if(except != null) {
          boolean accept = true;
          for(String s : except) {
            if(s.equals(methodName)) {
              accept = false;
              break;
            }
          }
          if(!accept) {
            continue;
          }
        }
        getters.add(m);
      }
    }
    return getters;
  }

  /**
   * Возвращает значения по геттрем
   * @param getters геттеры
   * @param instance объект
   * @return список значений
   */
  @SuppressWarnings({"EmptyCatchBlock"})
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

  /**
   * Сравнивает списки значений и записывает в журнал различия
   * @param j журнал
   * @param oldValues список старых значений
   * @param newValues список новых значений
   * @param getters геттеры
   * @param user пользователь, сделавший изменения
   * @param bundleMessage ключ сообщения для журнала (возможно шаблон)
   * @param addParameters параметры для шаблона сообщения
   * @throws mobi.eyeline.informer.admin.AdminException ошибка записи в журнал
   */
  protected void logChanges(Journal j, List<Object> oldValues, List<Object> newValues, List<Method> getters, String user, String bundleMessage, String... addParameters) throws AdminException {
    for (int i = 0; i < getters.size(); i++) {
      Object oldValue = oldValues.get(i);
      Object newValue = newValues.get(i);

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

      if (changed) {
        Method getter = getters.get(i);
        String propertyName = getter.getName().startsWith("is") ? getter.getName().substring(2) : getter.getName().substring(3);
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

        j.addRecord(JournalRecord.Type.CHANGE, subject, user, bundleMessage, args);
      }
    }

  }
  /**
   * Сравнивает списки значений и записывает в журнал различия с дефолтным сообщением
   * @param j журнал
   * @param oldValues список старых значений
   * @param newValues список новых значений
   * @param getters геттеры
   * @param user пользователь, сделавший изменения
   * @throws mobi.eyeline.informer.admin.AdminException ошибка записи в журнал
   */
  protected void logChanges(Journal j, List<Object> oldValues, List<Object> newValues, List<Method> getters, String user) throws AdminException {
    logChanges(j, oldValues, newValues, getters, user, "property_changed");
  }


  public void logMapsDiff(Journal journal,Subject subject, String objectId, Map oldMap,  Map newMap, String userName) throws AdminException {
    for(Object key : oldMap.keySet()) {
       Object oldVal = oldMap.get(key);
       Object newVal = newMap.get(key);
       if(newVal==null) {
            journal.addRecord(JournalRecord.Type.REMOVE, subject, userName,"obj_property_removed",objectId,valueToString(key),valueToString(oldVal));
       }
       else {
         if(!newVal.equals(oldVal)) {
            journal.addRecord(JournalRecord.Type.CHANGE, subject, userName,"obj_property_changed",objectId,valueToString(key), valueToString(oldVal), valueToString(newVal));
         }
       }
    }
    for(Object key : newMap.keySet()) {
       Object oldVal = oldMap.get(key);
       if(oldVal==null) {
          Object newVal = newMap.get(key);
          journal.addRecord(JournalRecord.Type.ADD, subject, userName,"obj_property_added",objectId, valueToString(key), valueToString(newVal));
       }
    }
  }
}
