package ru.novosoft.smsc.changelog.snmp;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.snmp.SnmpManager;
import ru.novosoft.smsc.admin.snmp.SnmpObject;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;
import ru.novosoft.smsc.changelog.util.ChangeLogAspectHelper;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
@Aspect
public class SnmpManagerChangeLogAspect {

  @Pointcut("call (public void ru.novosoft.smsc.admin.snmp.SnmpManager.setCounterInterval(int))")
  public void setCounterInterval() {
  }

  @Pointcut("call (public void ru.novosoft.smsc.admin.snmp.SnmpManager.setDefaultSnmpObject(*))")
  public void setDefaultSnmpObject() {
  }

  @Pointcut("call (public void ru.novosoft.smsc.admin.snmp.SnmpManager.setSnmpObjects(*))")
  public void setSnmpObjects() {
  }

  @Around("target(m) && setCounterInterval()")
  public void logSetCounterInterval(SnmpManager m, ProceedingJoinPoint pjp) throws Throwable {
    int oldValue = m.getCounterInterval();

    pjp.proceed();

    int newValue = m.getCounterInterval();

    if (oldValue != newValue) {
      ChangeLog changeLog = ChangeLogLocator.getInstance(m);
      if (changeLog != null)
        changeLog.propertyChanged(ChangeLog.Subject.SNMP, null, SnmpManager.class, "counterInterval", oldValue, newValue);
    }
  }

  private static void logDifferences(ChangeLog changeLog, String subjectDesc, SnmpObject oldValue, SnmpObject newValue) {
    List<Method> getters = ChangeLogAspectHelper.getGetters(SnmpObject.class);

    try {
      for (Method m : getters) {
        Object oldVal = m.invoke(oldValue);
        Object newVal = m.invoke(newValue);

        boolean changed;
        changed = oldVal == null && newVal != null;
        changed = changed || (oldVal != null && newVal == null);
        changed = changed || (oldVal != null && newVal != null && !oldVal.equals(newVal));

        if (changed) {
          String propertyName = m.getName().substring(3);
          char f = propertyName.charAt(0);
          if (propertyName.length() > 1)
            propertyName = Character.toLowerCase(f) + propertyName.substring(1);
          else
            propertyName = Character.toLowerCase(f) + "";
          changeLog.propertyChanged(ChangeLog.Subject.SNMP, subjectDesc, SnmpObject.class, propertyName, oldVal, newVal);
        }
      }
    } catch (Exception e) {
    }
  }

  @Around("target(m) && setDefaultSnmpObject()")
  public void logSetDefaultSnmpObject(SnmpManager m, ProceedingJoinPoint pjp) throws Throwable {
    SnmpObject oldValue = m.getDefaultSnmpObject();
    pjp.proceed();
    SnmpObject newValue = m.getDefaultSnmpObject();

    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    logDifferences(changeLog, "defaultSnmpObject", oldValue, newValue);
  }

  @Around("target(m) && setSnmpObjects()")
  public void logSetSnmpObjects(SnmpManager m, ProceedingJoinPoint pjp) throws Throwable {
    Map<String, SnmpObject> oldObjects = m.getSnmpObjects();

    pjp.proceed();
    
    Map<String, SnmpObject> newObjects = m.getSnmpObjects();

    ChangeLog changeLog = ChangeLogLocator.getInstance(m);

    for (Map.Entry<String, SnmpObject> e : oldObjects.entrySet()) {
      SnmpObject oldVal = e.getValue();
      SnmpObject newVal = newObjects.get(e.getKey());
      if (newVal == null) {
        changeLog.objectRemoved(ChangeLog.Subject.SNMP, e.getKey());
      } else {
        logDifferences(changeLog, e.getKey(), oldVal, newVal);
        newObjects.remove(e.getKey());
      }
    }

    for (String key : newObjects.keySet()) {
      changeLog.objectAdded(ChangeLog.Subject.SNMP, key);
    }
  }
}
