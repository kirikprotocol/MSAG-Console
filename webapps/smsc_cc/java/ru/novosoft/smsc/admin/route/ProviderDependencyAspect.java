package ru.novosoft.smsc.admin.route;

import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminContextLocator;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.provider.ProviderSettings;

/**
 * @author Artem Snopkov
 */
@Aspect
public class ProviderDependencyAspect {

  @Pointcut("call (void *.updateSettings(*)) && target(routeSubjectManager) && args(settings)")
  public void updateRouteSettings(RouteSubjectManager routeSubjectManager, RouteSubjectSettings settings) {
  }

  @Before("updateRouteSettings(rsm, settings)")
  public void beforeUpdateRouteSettings(RouteSubjectManager rsm, RouteSubjectSettings settings) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(rsm);
    if (ctx == null)
      return;

    ProviderSettings ps = ctx.getProviderManager().getSettings();

    for (Route r : settings.getRoutes()) {
      if (r.getProviderId() != null && ps.getProvider(r.getProviderId()) == null)
        throw new RouteException("provider.not.found", String.valueOf(r.getAclId()));
    }
  }
}
