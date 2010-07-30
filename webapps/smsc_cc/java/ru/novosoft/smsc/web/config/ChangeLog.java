package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.web.auth.User;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class ChangeLog {

  public static List<Method> getGetters(Class clazz) {
    List<Method> getters = new ArrayList<Method>();
    for (Method m : clazz.getMethods()) {
      String methodName = m.getName();
      if (methodName.startsWith("get") || methodName.startsWith("is"))
        getters.add(m);
    }
    return getters;
  }

  private List<Object> callGetters(List<Method> getters, Object instance) {
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

  protected void logPropertyChanged(String subject, String subjectDesc, Class subjectClass, String propertyName, Object oldValue, Object newValue, User user) {

  }

  private void logChanges(String subject, String subjectDef, Class objectClass, List<Object> oldValues, List<Object> newValues, List<Method> getters, User user) {
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
        logPropertyChanged(subject, subjectDef, objectClass, propertyName, oldValue, newValue, user);
      }
    }
  }

  public void logChanges(SmscSettings oldSettings, SmscSettings newSettings, User user) {

    {
      CommonSettings oldCommon = oldSettings.getCommonSettings();
      CommonSettings newCommon = newSettings.getCommonSettings();
      List<Method> getters = getGetters(CommonSettings.class);
      List<Object> oldValues = callGetters(getters, oldCommon);
      List<Object> newValues = callGetters(getters, newCommon);
      logChanges("SMSC", "Common settings", CommonSettings.class, oldValues, newValues, getters, user);
    }

    {
      List<Method> getters = getGetters(InstanceSettings.class);
      for (int i = 0; i < oldSettings.getSmscInstancesCount(); i++) {
        InstanceSettings old = oldSettings.getInstanceSettings(i);
        InstanceSettings nw = newSettings.getInstanceSettings(i);
        List<Object> oldValues = callGetters(getters, old);
        List<Object> newValues = callGetters(getters, nw);
        logChanges("SMSC", "Instance " + i + " settings", InstanceSettings.class, oldValues, newValues, getters, user);
      }
    }
  }

  public void logChanges(RescheduleSettings oldSettings, RescheduleSettings newSettings, User user) {
    List<Method> getters = getGetters(RescheduleSettings.class);
    List<Object> oldValues = callGetters(getters, oldSettings);
    List<Object> newValues = callGetters(getters, newSettings);
    logChanges("Reschedule", "Reschedule settings", RescheduleSettings.class, oldValues, newValues, getters, user);
  }

}
