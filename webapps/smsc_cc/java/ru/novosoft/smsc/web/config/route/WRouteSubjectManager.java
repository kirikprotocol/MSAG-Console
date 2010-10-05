package ru.novosoft.smsc.web.config.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
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
    RouteSubjectSettings oldSettings = getSettings();
    wrapped.updateSettings(settings);
    logChanges(oldSettings, settings);
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

  private void logChanges(RouteSubjectSettings oldSettings, RouteSubjectSettings newSettings) {
  }

  private Subject getSubjectByName(List<Subject> subjects, String name) {
    for (Subject s : subjects)
      if (s.getName().equals(name))
        return s;
    return null;
  }

  private void logChangesInSubjects(List<Subject> oldSubjects, List<Subject> newSubjects) {    
  }
}
