package ru.novosoft.smsc.admin.smsc;

import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminContextLocator;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.resource.ResourceManager;

/**
 * @author Artem Snopkov
 */


@Aspect
public class ResourcesDependencyAspect {

  @Pointcut("call (void SmscManager.updateSettings(SmscSettings))")
  public void updateSmscSettings() {}

  @Pointcut("call (boolean ru.novosoft.smsc.admin.resource.ResourceManager.removeResourceSettings(*))")
  public void removeResource() {}

  @Before("target(smscManager) && args(smscSettings) && updateSmscSettings()")
  public void beforeUpdateSettings(SmscManager smscManager, SmscSettings smscSettings) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(smscManager);

    ResourceManager resourceManager = ctx.getResourceManager();

    CommonSettings cs = smscSettings.getCommonSettings();
    for (String locale : cs.getLocales()) {
      if (!resourceManager.containsLocale(locale))
        throw new SmscException("locale_not_found", locale);
    }
  }

  @Before("target(resourceManager) && args(locale) && removeResource()")
  public void beforeRemoveResource(ResourceManager resourceManager, String locale) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(resourceManager);

    SmscManager smscManager = ctx.getSmscManager();
    CommonSettings smscSettings = smscManager.getSettings().getCommonSettings();

    for (String l : smscSettings.getLocales()) {
      if (l.equals(locale))
        throw new SmscException("locale_used", locale);
    }
  }
}
