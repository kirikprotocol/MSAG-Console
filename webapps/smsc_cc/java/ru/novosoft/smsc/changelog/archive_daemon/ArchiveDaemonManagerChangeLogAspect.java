package ru.novosoft.smsc.changelog.archive_daemon;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.AfterReturning;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;
import ru.novosoft.smsc.changelog.util.JavaBeanChangeLogAspectHelper;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * @author Artem Snopkov
 */

@Aspect
public class ArchiveDaemonManagerChangeLogAspect extends JavaBeanChangeLogAspectHelper {

  public ArchiveDaemonManagerChangeLogAspect( ) {
    super(ArchiveDaemonManager.class, ChangeLog.Subject.ARCHIVE_DAEMON);
  }

  @Pointcut("call (public void ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager.set*(*))")
  public void setter() {}

  @Pointcut("call (void ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager.apply())")
  public void apply() {
  }

  @Pointcut("call (void ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager.reset())")
  public void reset() {
  }

  @Around("target(m) && setter()")
  public void logSetter(ArchiveDaemonManager m, ProceedingJoinPoint pjp) throws Throwable {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    _logSetter(m, changeLog, pjp);
  }

  @AfterReturning("target(m) && apply()")
  public void logApply(ArchiveDaemonManager m) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    if (changeLog != null)
      changeLog.applyCalled(ChangeLog.Subject.ARCHIVE_DAEMON);
  }

  @AfterReturning("target(m) && reset()")
  public void logReset(ArchiveDaemonManager m) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    if (changeLog != null)
      changeLog.resetCalled(ChangeLog.Subject.ARCHIVE_DAEMON);
  }

}
