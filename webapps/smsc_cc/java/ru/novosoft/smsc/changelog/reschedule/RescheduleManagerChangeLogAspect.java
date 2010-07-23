package ru.novosoft.smsc.changelog.reschedule;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.AfterReturning;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.reschedule.RescheduleManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;
import ru.novosoft.smsc.changelog.util.JavaBeanChangeLogAspectHelper;

/**
 * @author Artem Snopkov
 */
@Aspect
public class RescheduleManagerChangeLogAspect extends JavaBeanChangeLogAspectHelper {

  public RescheduleManagerChangeLogAspect() {
    super(RescheduleManager.class, ChangeLog.Subject.RESCHEDULE);
  }

  @Pointcut("call (public void ru.novosoft.smsc.admin.reschedule.RescheduleManager.set*(*))")
  public void setter() {
  }

  @Pointcut("call (void ru.novosoft.smsc.admin.reschedule.RescheduleManager.apply())")
  public void apply() {
  }

  @Pointcut("call (void ru.novosoft.smsc.admin.reschedule.RescheduleManager.reset())")
  public void reset() {
  }


  @Around("target(m) && setter()")
  public void logSetter(RescheduleManager m, ProceedingJoinPoint pjp) throws Throwable {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    _logSetter(m, changeLog, pjp);
  }

  @AfterReturning("target(m) && apply()")
  public void logApply(RescheduleManager m) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    if (changeLog != null)
      changeLog.applyCalled(ChangeLog.Subject.RESCHEDULE);
  }

  @AfterReturning("target(m) && reset()")
  public void logReset(RescheduleManager m) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    if (changeLog != null)
      changeLog.resetCalled(ChangeLog.Subject.RESCHEDULE);
  }
}
