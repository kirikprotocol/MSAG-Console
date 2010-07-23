package ru.novosoft.smsc.changelog.map_limit;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.AfterReturning;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.map_limit.MapLimitManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;
import ru.novosoft.smsc.changelog.util.JavaBeanChangeLogAspectHelper;

/**
 * @author Artem Snopkov
 */
@Aspect
public class MapLimitManagerChangeLogAspect extends JavaBeanChangeLogAspectHelper{

  public MapLimitManagerChangeLogAspect() {
    super(MapLimitManager.class, ChangeLog.Subject.MAP_LIMIT);
  }

  @Pointcut("call (public void set*(*))")
  public void setter() {
  }

  @Pointcut("call (void apply())")
  public void apply() {
  }

  @Pointcut("call (void reset())")
  public void reset() {
  }


  @Around("target(m) && setter()")
  public void logSetter(MapLimitManager m, ProceedingJoinPoint pjp) throws Throwable {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    _logSetter(m, changeLog, pjp);
  }

  @AfterReturning("target(m) && apply()")
  public void logApply(MapLimitManager m) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    if (changeLog != null)
      changeLog.applyCalled(ChangeLog.Subject.MAP_LIMIT);
  }

  @AfterReturning("target(m) && reset()")
  public void logReset(MapLimitManager m) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    if (changeLog != null)
      changeLog.resetCalled(ChangeLog.Subject.MAP_LIMIT);
  }
}
