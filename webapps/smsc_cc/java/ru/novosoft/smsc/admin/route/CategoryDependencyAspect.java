package ru.novosoft.smsc.admin.route;

import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminContextLocator;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.category.CategorySettings;
import ru.novosoft.smsc.admin.provider.ProviderSettings;

/**
 * @author Artem Snopkov
 */
@Aspect
public class CategoryDependencyAspect {
  @Pointcut("call (void *.updateSettings(*)) && target(routeSubjectManager) && args(settings)")
  public void updateRouteSettings(RouteSubjectManager routeSubjectManager, RouteSubjectSettings settings) {
  }

  @Before("updateRouteSettings(rsm, settings)")
  public void beforeUpdateRouteSettings(RouteSubjectManager rsm, RouteSubjectSettings settings) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(rsm);
    if (ctx == null)
      return;

    CategorySettings ps = ctx.getCategoryManager().getSettings();

    for (Route r : settings.getRoutes()) {
      if (r.getCategoryId() != null && ps.getCategory(r.getCategoryId()) == null)
        throw new RouteException("category.not.found", String.valueOf(r.getCategoryId()));
    }
  }
}
