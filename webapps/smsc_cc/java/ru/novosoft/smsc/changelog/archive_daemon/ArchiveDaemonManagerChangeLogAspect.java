package ru.novosoft.smsc.changelog.archive_daemon;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * @author Artem Snopkov
 */

@Aspect
public class ArchiveDaemonManagerChangeLogAspect {

  @Pointcut("call (public void set*(*))")
  public void setter() {}

  @Around("target(m) && setter()")
  public void logSetter(ArchiveDaemonManager m, ProceedingJoinPoint pjp) throws Throwable {
    String setterName = pjp.getSignature().getName();
    String fieldName = setterName.substring(3);
    try {
      Method getter = ArchiveDaemonManager.class.getMethod("get" + fieldName);
      Object oldValue = getter.invoke(m);

      pjp.proceed();

      Object newValue = getter.invoke(m);

      boolean changed;
      changed = oldValue == null && newValue != null;
      changed = changed || (oldValue != null && newValue == null);
      changed = changed || (oldValue != null && newValue != null && !oldValue.equals(newValue));

      if (changed) {
        ChangeLog ch = ChangeLogLocator.getInstance(m);
        char c = fieldName.charAt(0);
        if (fieldName.length() > 1)
          fieldName = Character.toLowerCase(c) + fieldName.substring(1);
        else
          fieldName = Character.toLowerCase(c) + "";
        if (ch != null)
          ch.propertyChanged(ChangeLog.Source.ARCHIVE_DAEMON, "Config", ArchiveDaemonManager.class, fieldName, oldValue, newValue);
      }

    } catch (NoSuchMethodException e) {
    } catch (InvocationTargetException e) {
    } catch (IllegalAccessException e) {
    }
  }

}
