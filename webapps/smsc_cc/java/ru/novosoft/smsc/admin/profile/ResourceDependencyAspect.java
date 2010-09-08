package ru.novosoft.smsc.admin.profile;

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
public class ResourceDependencyAspect {

  @Pointcut("call (void *.updateProfile(*)) && target(manager) && args(profile)")
  public void updateProfile(ProfileManager manager, Profile profile) {}

  @Before("updateProfile(manager, profile)")
  public void beforeUpdateProfile(ProfileManager manager, Profile profile) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(manager);
    if (ctx == null)
      return;

    ResourceManager rm = ctx.getResourceManager();
    if (!rm.containsLocale(profile.getLocale()))
      throw new ProfileException("resource.not.found", profile.getLocale());
  }
}
