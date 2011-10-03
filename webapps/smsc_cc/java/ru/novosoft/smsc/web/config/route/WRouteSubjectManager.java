package ru.novosoft.smsc.web.config.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.route.RouteSubjectManager;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import static ru.novosoft.smsc.web.config.DiffHelper.*;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class WRouteSubjectManager extends BaseSettingsManager<RouteSubjectSettings> implements RouteSubjectManager {

  private final RouteSubjectManager wrapped;
  private final Journal j;

  public WRouteSubjectManager(RouteSubjectManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  @Override
  protected void _updateSettings(RouteSubjectSettings settings) throws AdminException {
    wrapped.updateSettings(settings);
  }

  public RouteSubjectSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public RouteSubjectSettings cloneSettings(RouteSubjectSettings settings) {
    return settings.cloneSettings();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }

  protected void logChanges(RouteSubjectSettings oldSettings, RouteSubjectSettings newSettings) {
    logChangesInSubjects(oldSettings.getSubjects(), newSettings.getSubjects());
    logChangesInRoutes(oldSettings.getRoutes(), newSettings.getRoutes());
  }

  private Subject getSubjectByName(List<Subject> subjects, String name) {
    for (Subject s : subjects)
      if (s.getName().equals(name))
        return s;
    return null;
  }

  private Route getRouteByName(List<Route> routes, String name) {
    for (Route s : routes)
      if (s.getName().equals(name))
        return s;
    return null;
  }

  private void logChangesInSubjects(List<Subject> oldSubjects, List<Subject> newSubjects) {
    for (Subject s : oldSubjects) {
      final Subject newSubj = getSubjectByName(newSubjects, s.getName());
      if (newSubj == null)
        j.user(user).remove().subject(s.getName());
      else
        findChanges(s, newSubj, Subject.class, new ChangeListener() {
          public void foundChange(String propertyName, Object oldValue, Object newValue) {
            j.user(user).change("property_changed", valueToString(oldValue), valueToString(newValue)).subject(newSubj.getName());
          }
        });
    }

    for (Subject s : newSubjects) {
      if (!oldSubjects.contains(s))
        j.user(user).add().subject(s.getName());
    }
  }

  private void logChangesInRoutes(List<Route> oldRoutes, List<Route> newRoutes) {
    for (Route oldRoute : oldRoutes) {
      final Route newRoute = getRouteByName(newRoutes, oldRoute.getName());
      if (newRoute == null)
        j.user(user).remove().route(oldRoute.getName());
      else
        findChanges(oldRoute, newRoute, Route.class, new ChangeListener() {
          public void foundChange(String propertyName, Object oldValue, Object newValue) {
            String o = valueToString(oldValue);
            String n = valueToString(newValue);
            if(!o.equals(n)) {
              j.user(user).change("property_changed", propertyName, o, n).route(newRoute.getName());
            }
          }
        });
    }

    for (Route route : newRoutes) {
      if (getRouteByName(oldRoutes, route.getName()) == null)
        j.user(user).add().route(route.getName());
    }
  }
}
