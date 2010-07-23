package ru.novosoft.smsc.changelog.msc;

import org.aspectj.lang.annotation.AfterReturning;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.msc.MscManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;
import ru.novosoft.smsc.util.Address;

/**
 * @author Artem Snopkov
 */
@Aspect
public class MscManagerChangeLogAspect {

  @Pointcut("call (void ru.novosoft.smsc.admin.msc.MscManager.addMsc(ru.novosoft.smsc.util.Address))")
  public void addMsc() {
  }

  @Pointcut("call (void ru.novosoft.smsc.admin.msc.MscManager.removeMsc(ru.novosoft.smsc.util.Address))")
  public void removeMsc() {
  }

  @AfterReturning("target(m) && args(addr) && addMsc()")
  public void logAddMsc(MscManager m, Address addr) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    if (changeLog != null)
      changeLog.objectAdded(ChangeLog.Subject.MSC, addr);
  }

  @AfterReturning("target(m) && args(addr) && removeMsc()")
  public void logRemoveMsc(MscManager m, Address addr) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(m);
    if (changeLog != null)
      changeLog.objectRemoved(ChangeLog.Subject.MSC, addr);
  }

}
