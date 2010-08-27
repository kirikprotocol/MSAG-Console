package ru.novosoft.smsc.web.config.changelog;

import ru.novosoft.smsc.admin.reschedule.Reschedule;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;

import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class LocalChangeLog implements ChangeLog {

  public static final String SMSC = "subject.smsc";
  public static final String RESCHEDULE = "subject.reschedule";

  private final List<ChangeLogRecord> records = new ArrayList<ChangeLogRecord>();

  public synchronized List<ChangeLogRecord> getRecords() {
    return new ArrayList<ChangeLogRecord>(records);
  }

  public boolean isEmpty() {
    return records.isEmpty();
  }

  /**
   * Возвращает последнюю запись для указанного subject, с датой после afterTime или null, если такой нет.
   *
   * @param subject   сабжект
   * @param afterTime время, начиная с которого надо искать запись
   * @return последнюю запись для указанного subject, с датой после afterTime или null, если такой нет.
   */
  public synchronized ChangeLogRecord getLastRecord(String subject, long afterTime) {
    for (int i = records.size() - 1; i >= 0; i--) {
      ChangeLogRecord r = records.get(i);
      if (r.getSubjectKey().equals(subject)) {
        if (r.getTime() > afterTime)
          return r;
        else
          return null;
      }
    }
    return null;
  }

  public synchronized List<ChangeLogRecord> getRecords(String subject) {
    List<ChangeLogRecord> res = new ArrayList<ChangeLogRecord>();
    for (ChangeLogRecord r : records) {
      if (r.getSubjectKey().equals(subject))
        res.add(r);
    }
    return res;
  }

  /**
   * Удаляет из лога все записи с указанным сабжектом
   *
   * @param subject сабжект, записи которого надо удалить
   */
  public synchronized void removeRecords(String subject) {
    for (Iterator<ChangeLogRecord> i = records.iterator(); i.hasNext();) {
      ChangeLogRecord r = i.next();
      if (r.getSubjectKey().equals(subject))
        i.remove();
    }
  }

  private static List<Method> getGetters(Class clazz) {
    List<Method> getters = new ArrayList<Method>();
    for (Method m : clazz.getMethods()) {
      String methodName = m.getName();
      if (methodName.startsWith("get") || methodName.startsWith("is"))
        getters.add(m);
    }
    return getters;
  }

  private static List<Object> callGetters(List<Method> getters, Object instance) {
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

  private synchronized ChangeLogRecord addRecord(ChangeLogRecord.Type type, String subject, String user) {
    ChangeLogRecord r = new ChangeLogRecord(type);
    r.setUser(user);
    r.setTime(System.currentTimeMillis());
    r.setSubjectKey(subject);

    records.add(r);
    return r;
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

  private void logChanges(String subject, List<Object> oldValues, List<Object> newValues, List<Method> getters, String user) {
    for (int i = 0; i < getters.size(); i++) {
      Object oldValue = oldValues.get(i);
      Object newValue = newValues.get(i);

      boolean changed = oldValue == null && newValue != null;
      changed = changed || (oldValue != null && newValue == null);

      if (!changed && oldValue != null && newValue != null) {
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

        addRecord(ChangeLogRecord.Type.CHANGE, subject, user).setDescription("property_changed", propertyName, valueToString(oldValue), valueToString(newValue));
      }
    }
  }

  public void logChanges(SmscSettings oldSettings, SmscSettings newSettings, String user) {

    {
      CommonSettings oldCommon = oldSettings.getCommonSettings();
      CommonSettings newCommon = newSettings.getCommonSettings();
      List<Method> getters = getGetters(CommonSettings.class);
      List<Object> oldValues = callGetters(getters, oldCommon);
      List<Object> newValues = callGetters(getters, newCommon);
      logChanges(SMSC, oldValues, newValues, getters, user);
    }

    {
      List<Method> getters = getGetters(InstanceSettings.class);
      for (int i = 0; i < oldSettings.getSmscInstancesCount(); i++) {
        InstanceSettings old = oldSettings.getInstanceSettings(i);
        InstanceSettings nw = newSettings.getInstanceSettings(i);
        List<Object> oldValues = callGetters(getters, old);
        List<Object> newValues = callGetters(getters, nw);
        logChanges(SMSC, oldValues, newValues, getters, user);
      }
    }
  }

  /**
   * По коллекции политик расписаний строит Map, ключем в котором является SMPP status, значением - политика
   * передоставки для этого статуса.
   *
   * @param reschedules список политик передоставки
   * @return Map, ключем в котором является SMPP status, значением - политика передоставки для этого статуса.
   */
  private static Map<Integer, String> expandReschedules(Collection<Reschedule> reschedules) {
    Map<Integer, String> result = new HashMap<Integer, String>();
    for (Reschedule r : reschedules) {
      for (int status : r.getStatuses())
        result.put(status, r.getIntervals());
    }
    return result;
  }

  public void logChanges(RescheduleSettings oldSettings, RescheduleSettings newSettings, String user) {
    Map<Integer, String> oldReschedules = expandReschedules(oldSettings.getReschedules());
    Map<Integer, String> newReschedules = expandReschedules(newSettings.getReschedules());

    for (Map.Entry<Integer, String> e : oldReschedules.entrySet()) {
      int smppStatus = e.getKey();
      String oldReschedule = e.getValue();
      String newReschedule = newReschedules.get(smppStatus);
      if (newReschedule == null)
        addRecord(ChangeLogRecord.Type.REMOVE, RESCHEDULE, user).setDescription("reschedule_removed", String.valueOf(smppStatus));
      else if (!newReschedule.equals(oldReschedule))
        addRecord(ChangeLogRecord.Type.CHANGE, RESCHEDULE, user).setDescription("reschedule_changed", String.valueOf(smppStatus), oldReschedule, newReschedule);
    }

    for (Map.Entry<Integer, String> e : newReschedules.entrySet()) {
      int smppStatus = e.getKey();
      if (!oldReschedules.containsKey(smppStatus))
        addRecord(ChangeLogRecord.Type.ADD, RESCHEDULE, user).setDescription("reschedule_added", String.valueOf(smppStatus), e.getValue());
    }

    if (!oldSettings.getDefaultReschedule().equals(newSettings.getDefaultReschedule()))
      addRecord(ChangeLogRecord.Type.CHANGE, RESCHEDULE, user).setDescription("property_changed", "defaultReschedule", oldSettings.getDefaultReschedule(), newSettings.getDefaultReschedule());
  }

}
