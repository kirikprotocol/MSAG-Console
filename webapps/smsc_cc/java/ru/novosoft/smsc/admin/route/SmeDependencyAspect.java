package ru.novosoft.smsc.admin.route;

import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminContextLocator;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.sme.SmeManager;

/**
 * @author Artem Snopkov
 */
@Aspect
public class SmeDependencyAspect {

  @Pointcut("call (boolean *.removeSme(String)) && target(smeManager) && args(smeId)")
  public void removeSme(SmeManager smeManager, String smeId) {}

  @Pointcut("call (void *.updateSettings(*)) && target(routeSubjectManager) && args(settings)")
  public void updateRouteSettings(RouteSubjectManager routeSubjectManager, RouteSubjectSettings settings) {}

  @Before("removeSme(smeManager, smeId)")
  public void beforeRemoveSme(SmeManager smeManager, String smeId) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(smeManager);
    if (ctx == null)
      return;

    RouteSubjectManager rsm = ctx.getRouteSubjectManager();
    beforeRemoveSme(smeManager, smeId, rsm);
  }

  void beforeRemoveSme(SmeManager smeManager, String smeId, RouteSubjectManager rsm) throws AdminException {
    RouteSubjectSettings settings = rsm.getSettings();

    for (Subject s : settings.getSubjects()) {
      if (s.getDefaultSmeId() != null && s.getDefaultSmeId().equals(smeId))
        throw new RouteException("sme.used.in.subject", s.getName());
    }

    for (Route r : settings.getRoutes()) {
      if (r.getSrcSmeId() != null && r.getSrcSmeId().equals(smeId))
        throw new RouteException("sme.used.in.route", r.getName());

      if (r.getDestinations() != null) {
        for (Destination d : r.getDestinations())
          if (d.getSmeId() != null && d.getSmeId().equals(smeId))
            throw new RouteException("sme.used.in.route", r.getName());
      }
    }
  }

  @Before("updateRouteSettings(manager, settings)")
  public void beforeUpdateRouteSettings(RouteSubjectManager manager, RouteSubjectSettings settings) throws RouteException {
    AdminContext ctx = AdminContextLocator.getContext(manager);
    if (ctx == null)
      return;

    SmeManager smeManager = ctx.getSmeManager();

    for (Subject s : settings.getSubjects()) {
      if (s.getDefaultSmeId() != null && !smeManager.contains(s.getDefaultSmeId()))
        throw new RouteException("sme.not.found", s.getDefaultSmeId());
    }

    for (Route r : settings.getRoutes()) {
      if (r.getSrcSmeId() != null && !smeManager.contains(r.getSrcSmeId()))
        throw new RouteException("sme.not.found", r.getSrcSmeId());

      if (r.getDestinations() != null) {
        for (Destination d : r.getDestinations()) {
          if (d.getSmeId() != null && !smeManager.contains(d.getSmeId()))
            throw new RouteException("sme.not.found", d.getSmeId());
        }
      }
    }
  }
}
