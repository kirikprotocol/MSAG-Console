package ru.novosoft.smsc.changelog.closed_groups;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;
import ru.novosoft.smsc.util.Address;

import java.util.Collection;

/**
 * @author Artem Snopkov
 */
@Aspect
public class ClosedGroupChangeLogAspect {

  @Pointcut("call (void setDescription(String))")
  public void setDescription() {
  }

  @Pointcut("call (void addMask(ru.novosoft.smsc.util.Address)) ||" +
            "call (void removeMask(ru.novosoft.smsc.util.Address))")
  public void addRemoveMask() {
  }

  @Around("target(cg) && setDescription()")
  public void logSetDescription(ClosedGroup cg, ProceedingJoinPoint pjp) throws Throwable {

    String oldDescription = null;
    boolean failed = false;
    try {
      oldDescription = cg.getDescription();
    } catch (AdminException e) {
      failed = true;
    }

    pjp.proceed();

    if (failed)
      return;

    String newDescription;
    try {
      newDescription = cg.getDescription();
    } catch (AdminException e) {
      return;
    }

    boolean changed = oldDescription == null && newDescription != null;
    changed = changed || (oldDescription != null && newDescription == null);
    changed = changed || (oldDescription != null && newDescription != null && !oldDescription.equals(newDescription));

    if (changed) {
      ChangeLog changeLog = ChangeLogLocator.getInstance(cg.getClosedGroupManager());
      if (changeLog != null) {
        changeLog.propertyChanged(ChangeLog.Subject.CLOSED_GROUP, cg.getName(), ClosedGroup.class, "description", oldDescription, newDescription);
      }
    }
  }

  @Around("target(cg) && addRemoveMask()")
  public void logAddRemoveMask(ClosedGroup cg, ProceedingJoinPoint pjp) throws Throwable {
    Collection<Address> oldMasks = null;
    boolean failed = false;
    try {
      oldMasks = cg.getMasks();
    } catch (AdminException e) {
      failed = true;
    }

    pjp.proceed();

    if (failed)
      return;

    Collection<Address> newMasks;
    try {
      newMasks = cg.getMasks();
    } catch (AdminException e) {
      return;
    }

    ChangeLog changeLog = ChangeLogLocator.getInstance(cg.getClosedGroupManager());
    if (changeLog != null) {
      changeLog.propertyChanged(ChangeLog.Subject.CLOSED_GROUP, cg.getName(), ClosedGroup.class, "masks", oldMasks, newMasks);
    }
  }
}
