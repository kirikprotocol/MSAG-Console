package ru.novosoft.smsc.changelog.alias;

import org.aspectj.lang.annotation.AfterReturning;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogLocator;

/**
 * @author Artem Snopkov
 */

@Aspect
public class AliasManagerChangeLogAspect {

  @Pointcut ("call (public void addAlias(ru.novosoft.smsc.admin.alias.Alias))")
  public void addAlias() {}

  @Pointcut ("call (public void deleteAlias(ru.novosoft.smsc.admin.alias.Alias))")
  public void deleteAlias() {}

  @AfterReturning("target(am) && args(alias) && addAlias()")
  public void logAddAlias(AliasManager am, Alias alias) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(am);
    if (changeLog != null)
      changeLog.objectAdded(ChangeLog.Subject.ALIAS, alias);
  }

  @AfterReturning("target(am) && args(alias) && deleteAlias()")
  public void logDeleteAlias(AliasManager am, Alias alias) {
    ChangeLog changeLog = ChangeLogLocator.getInstance(am);
    if (changeLog != null)
      changeLog.objectRemoved(ChangeLog.Subject.ALIAS, alias);
  }

}
