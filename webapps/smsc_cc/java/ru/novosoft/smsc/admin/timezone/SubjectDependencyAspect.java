package ru.novosoft.smsc.admin.timezone;

import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminContextLocator;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.RouteException;
import ru.novosoft.smsc.admin.route.RouteSubjectManager;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.sme.SmeManager;

import java.util.Collection;
import java.util.List;

/**
 * @author Artem Snopkov
 */
@Aspect
public class SubjectDependencyAspect {

  @Pointcut("call (void *.updateSettings(*)) && target(timezoneManager) && args(timezones)")
  public void updateTimezones(TimezoneManager timezoneManager, TimezoneSettings timezones) {}

  @Pointcut("call (void *.updateSettings(*)) && target(routeSubjectManager) && args(settings)")
  public void updateRouteSettings(RouteSubjectManager routeSubjectManager, RouteSubjectSettings settings) {}

  @Before("updateTimezones(manager, settings)")
  public void beforeUpdateTimezones(TimezoneManager manager, TimezoneSettings settings) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(manager);
    if (ctx == null)
      return;

    List<Subject> subjects = ctx.getRouteSubjectManager().getSettings().getSubjects();

    for (Timezone tz : settings.getTimezones()) {
      if (tz.getSubject() != null && !containsSubject(subjects, tz.getSubject()))
        throw new TimezoneException("subject.not.found", tz.getSubject());
    }
  }

  private boolean containsSubject(List<Subject> subjects, String subjectName) {
    for (Subject s : subjects)
      if (s.getName().equals(subjectName))
        return true;
    return false;
  }

  @Before("updateRouteSettings(manager, settings)")
  public void beforeUpdateRouteSettings(RouteSubjectManager manager, RouteSubjectSettings settings) throws AdminException {
    AdminContext ctx = AdminContextLocator.getContext(manager);
    if (ctx == null)
      return;

    Collection<Timezone> timezones = ctx.getTimezoneManager().getSettings().getTimezones();
    List<Subject> subjects = settings.getSubjects();

    for (Timezone tz : timezones) {
      if (tz.getSubject() != null && !containsSubject(subjects, tz.getSubject()))
        throw new TimezoneException("subject.used.in.timezones", tz.getSubject());
    }
  }

}
