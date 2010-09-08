package ru.novosoft.smsc.admin.profile;

import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminContextLocator;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.admin.resource.ResourceManager;

/**
 * @author Artem Snopkov
 */
@Aspect
public class ClosedGroupDependencyAspect {
  @Pointcut("call (void *.updateProfile(*)) && target(manager) && args(profile)")
  public void updateProfile(ProfileManager manager, Profile profile) {}

  @Before("updateProfile(manager, profile)")
  public void beforeUpdateProfile(ProfileManager manager, Profile profile) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(manager);
    if (ctx == null)
      return;

    ClosedGroupManager cgm = ctx.getClosedGroupManager();
    if (profile.getGroupId() != null && !cgm.containsGroup(profile.getGroupId()))
      throw new ProfileException("closed.group.not.found", String.valueOf(profile.getGroupId()));
  }
}
