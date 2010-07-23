package ru.novosoft.smsc.changelog.util;

import org.aspectj.lang.ProceedingJoinPoint;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;

import java.lang.reflect.Method;

/**
 * @author Artem Snopkov
 */
public class JavaBeanChangeLogAspectHelper {
  private Class beanClass;
  private ChangeLog.Subject changeLogSubject;

  public JavaBeanChangeLogAspectHelper(Class beanClass, ChangeLog.Subject changeLogSubject) {
    this.beanClass = beanClass;
    this.changeLogSubject = changeLogSubject;
  }

  protected void _logSetter(Object beanInstance, ChangeLog changeLog, ProceedingJoinPoint pjp) throws Throwable {
    String setterName = pjp.getSignature().getName();
    String fieldName = setterName.substring(3);


    boolean error = false;
    Object oldValue = null;
    Method getter = ChangeLogAspectHelper.getGetter(beanClass, fieldName);
    if (getter == null)
      error = true;
    else {
      try {
        oldValue = getter.invoke(beanInstance);
      } catch (Exception e) {
        error = true;
      }
    }

    pjp.proceed();

    if (error)
      return;

    Object newValue;
    try {
      newValue = getter.invoke(beanInstance);
    } catch (Exception e) {
      return;
    }

    boolean changed;
    changed = oldValue == null && newValue != null;
    changed = changed || (oldValue != null && newValue == null);
    changed = changed || (oldValue != null && newValue != null && !oldValue.equals(newValue));

    if (changed) {
      char c = fieldName.charAt(0);
      if (fieldName.length() > 1)
        fieldName = Character.toLowerCase(c) + fieldName.substring(1);
      else
        fieldName = Character.toLowerCase(c) + "";

      if (changeLog != null)
        changeLog.propertyChanged(changeLogSubject, "Config", beanClass, fieldName, oldValue, newValue);
    }
  }
}
