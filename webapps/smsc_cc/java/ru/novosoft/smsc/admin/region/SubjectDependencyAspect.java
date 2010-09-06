package ru.novosoft.smsc.admin.region;

import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminContextLocator;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.RouteSubjectManager;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.admin.route.Subject;

import java.util.List;

/**
 * @author Artem Snopkov
 */
@Aspect
public class SubjectDependencyAspect {
  @Pointcut("call (void *.updateSettings(*)) && target(regionManager) && args(settings)")
  public void updateRegions(RegionManager regionManager, RegionSettings settings) {
  }

  @Pointcut("call (void *.updateSettings(*)) && target(routeSubjectManager) && args(settings)")
  public void updateRouteSettings(RouteSubjectManager routeSubjectManager, RouteSubjectSettings settings) {
  }

  @Before("updateRegions(manager, settings)")
  public void beforeUpdateRegions(RegionManager manager, RegionSettings settings) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(manager);
    if (ctx == null)
      return;

    RouteSubjectManager rsm = ctx.getRouteSubjectManager();

    RouteSubjectSettings s = rsm.getSettings();
    List<Subject> subjects = s.getSubjects();

    for (Region r : settings.getRegions()) {
      for (String subjectId : r.getSubjects())
        if (!containsSubject(subjects, subjectId))
          throw new RegionException("subject.not.found", subjectId);
    }
  }

  @Before("updateRouteSettings(manager, settings)")
  public void beforeUpdateRouteSettings(RouteSubjectManager manager, RouteSubjectSettings settings) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(manager);
    if (ctx == null)
      return;

    RegionManager rm = ctx.getRegionManager();

    List<Subject> subjects = settings.getSubjects();

    for (Region r : rm.getSettings().getRegions())
      for (String subjectId : r.getSubjects()) {
        if (!containsSubject(subjects, subjectId))
          throw new RegionException("subject.used.in.regions", subjectId);
      }
  }

  private boolean containsSubject(List<Subject> subjects, String subjectName) {
    for (Subject s : subjects)
      if (s.getName().equals(subjectName))
        return true;
    return false;
  }
}
