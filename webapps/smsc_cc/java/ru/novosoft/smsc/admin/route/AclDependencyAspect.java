package ru.novosoft.smsc.admin.route;

import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminContextLocator;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.AclManager;

/**
 * @author Artem Snopkov
 */
@Aspect
public class AclDependencyAspect {

  @Pointcut("call (void *.removeAcl(int)) && target(aclManager) && args(aclId)")
  public void removeAcl(AclManager aclManager, int aclId) {}

  @Pointcut("call (void *.updateSettings(*)) && target(routeSubjectManager) && args(settings)")
  public void updateRouteSettings(RouteSubjectManager routeSubjectManager, RouteSubjectSettings settings) {}

  @Before("removeAcl(aclManager, aclId)")
  public void beforeRemoveAcl(AclManager aclManager, int aclId) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(aclManager);
    if (ctx == null)
      return;

    RouteSubjectManager rsm = ctx.getRouteSubjectManager();

    for (Route r : rsm.getSettings().getRoutes()) {
      if (r.getAclId() != null && r.getAclId() == aclId)
        throw new RouteException("acl.used.in.route", r.getName());
    }
  }

  @Before("updateRouteSettings(rsm, settings)")
  public void beforeUpdateRouteSettings(RouteSubjectManager rsm, RouteSubjectSettings settings) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(rsm);
    if (ctx == null)
      return;

    AclManager aclm = ctx.getAclManager();

    for (Route r : settings.getRoutes()) {
      try {
        if (r.getAclId() != null)
          aclm.getAcl(r.getAclId());

      } catch (AdminException e) {
        throw new RouteException("acl.not.found", String.valueOf(r.getAclId()));
      }
    }
  }
}
