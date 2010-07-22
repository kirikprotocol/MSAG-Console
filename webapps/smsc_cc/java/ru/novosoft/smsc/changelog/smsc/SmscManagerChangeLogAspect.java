package ru.novosoft.smsc.changelog.smsc;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Астект, описывающий логирование в ChangeLog изменений, происходящих в SmscManager
 * @author Artem Snopkov
 */

@Aspect
public class SmscManagerChangeLogAspect {

  private static List<Method> commonSettingsGetters = null;
  private static List<Method> instanceSettingsGetters = null;

  @Pointcut("call (void setCommonSettings(ru.novosoft.smsc.admin.smsc.CommonSettings))")
  public void setCommonSettings() {
  }

  @Pointcut("call (void setInstanceSettings(int, ru.novosoft.smsc.admin.smsc.InstanceSettings))")
  public void setInstanceSettings() {
  }

  private List<Method> getGetters(Class clazz) {
    List<Method> getters = new ArrayList<Method>();
    for (Method m : clazz.getMethods()) {
      if (m.getName().startsWith("get"))
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

  private void logChanges(ChangeLog changeLog, String object, Class objectClass, List<Object> oldValues, List<Object> newValues, List<Method> getters) {
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
//        System.out.println(propertyName + " " + oldValue + " -> " + newValue);
        changeLog.propertyChanged(ChangeLog.Source.SMSC, object, objectClass, propertyName, oldValue, newValue);
      }
    }
  }

  @Around("target(s) && setCommonSettings()")
  public void aroundSetCommonSettings(SmscManager s, ProceedingJoinPoint pjp) throws Throwable {
    // Get list of getters
    if (commonSettingsGetters == null)
      commonSettingsGetters = getGetters(CommonSettings.class);

    CommonSettings oldSettings = s.getCommonSettings();
    List<Object> oldValues = callGetters(commonSettingsGetters, oldSettings);

    pjp.proceed();

    CommonSettings newSettings = s.getCommonSettings();
    List<Object> newValues = callGetters(commonSettingsGetters, newSettings);

    ChangeLog cl = ChangeLogLocator.getInstance(s);
    if (cl != null)
      logChanges(cl, "Common settings", CommonSettings.class, oldValues, newValues, commonSettingsGetters);
  }

  @Around("target(s) && setInstanceSettings()")
  public void aroundSetInstanceSettings(SmscManager s, ProceedingJoinPoint pjp) throws Throwable {
    // Get list of getters
    if (instanceSettingsGetters == null)
      instanceSettingsGetters = getGetters(InstanceSettings.class);

    int instanceNumber = (Integer) pjp.getArgs()[0];

    InstanceSettings oldSettings = s.getInstanceSettings(instanceNumber);
    List<Object> oldValues = callGetters(instanceSettingsGetters, oldSettings);

    pjp.proceed();

    InstanceSettings newSettings = s.getInstanceSettings(instanceNumber);
    List<Object> newValues = callGetters(instanceSettingsGetters, newSettings);

    ChangeLog cl = ChangeLogLocator.getInstance(s);
    if (cl != null)
      logChanges(cl, "Instance " + instanceNumber + " settings", InstanceSettings.class, oldValues, newValues, instanceSettingsGetters);
  }

}
