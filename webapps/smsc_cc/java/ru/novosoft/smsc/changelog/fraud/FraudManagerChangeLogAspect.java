package ru.novosoft.smsc.changelog.fraud;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.AfterReturning;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;
import ru.novosoft.smsc.changelog.util.ChangeLogAspectHelper;
import ru.novosoft.smsc.changelog.util.JavaBeanChangeLogAspectHelper;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * @author Artem Snopkov
 */
@Aspect
public class FraudManagerChangeLogAspect extends JavaBeanChangeLogAspectHelper {
  
  public FraudManagerChangeLogAspect() {
    super(FraudManager.class, ChangeLog.Subject.FRAUD);
  }

  @Pointcut("call (public void ru.novosoft.smsc.admin.fraud.FraudManager.set*(*))")
  public void setter() {
  }

  @Pointcut("call (void ru.novosoft.smsc.admin.fraud.FraudManager.apply())")
  public void apply() {
  }

  @Pointcut("call (void ru.novosoft.smsc.admin.fraud.FraudManager.reset())")
  public void reset() {
  }


  @Around("target(m) && setter()")
  public void logSetter(FraudManager m, ProceedingJoinPoint pjp) throws Throwable {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    _logSetter(m, changeLog, pjp);
  }

  @AfterReturning("target(m) && apply()")
  public void logApply(FraudManager m) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    if (changeLog != null)
      changeLog.applyCalled(ChangeLog.Subject.FRAUD);
  }

  @AfterReturning("target(m) && reset()")
  public void logReset(FraudManager m) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    if (changeLog != null)
      changeLog.resetCalled(ChangeLog.Subject.FRAUD);
  }
}
