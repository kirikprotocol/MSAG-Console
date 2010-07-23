package ru.novosoft.smsc.changelog.closed_groups;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.AfterReturning;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;

import java.util.Collection;

/**
 * @author Artem Snopkov
 */
@Aspect
public class ClosedGroupManagerChangeLogAspect {

  @Pointcut("call (ru.novosoft.smsc.admin.closed_groups.ClosedGroup ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager.addGroup(String, String))")
  public void addGroup() {
  }

  @Pointcut("call (boolean ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager.removeGroup(int))")
  public void removeGroup() {
  }

  @AfterReturning(pointcut="target(cgm) && addGroup()", returning="cg")
  public void logAddGroup(ClosedGroupManager cgm, ClosedGroup cg) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(cgm);
    if (changeLog != null)
      changeLog.objectAdded(ChangeLog.Subject.CLOSED_GROUP, cg);
  }

  @Around("target(cgm) && removeGroup()")
  public boolean logRemoveGroup(ClosedGroupManager cgm, ProceedingJoinPoint pjp) throws Throwable {

    Collection<ClosedGroup> oldGroups = null;
    try {
      oldGroups = cgm.groups();
    } catch (Exception ignored) {
    }

    boolean res = (Boolean)pjp.proceed();

    if (res && oldGroups != null) {
      ChangeLog changeLog = ChangeLogLocator.getInstance(cgm);
      if (changeLog == null)
        return res;

      int groupId = (Integer)pjp.getArgs()[0];
      for (ClosedGroup cg : oldGroups) {
        if (cg.getId() == groupId) {
          changeLog.objectRemoved(ChangeLog.Subject.CLOSED_GROUP, cg);
          break;
        }
      }
    }
    return res;
  }
}
