package ru.novosoft.smsc.web.config.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerManager;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.config.DiffHelper;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.List;

import static ru.novosoft.smsc.web.config.DiffHelper.findChanges;
import static ru.novosoft.smsc.web.config.DiffHelper.valueToString;

/**
 * User: artem
 * Date: 26.10.11
 */
public class WClusterControllerManager extends BaseSettingsManager<ClusterControllerSettings> implements ClusterControllerManager {

  private final ClusterControllerManager wrapped;
  private final String user;
  private final Journal j;

  public WClusterControllerManager(ClusterControllerManager wrapped, String user, Journal j) {
    super(user);
    this.wrapped = wrapped;
    this.user = user;
    this.j = j;
  }

  public ClusterControllerSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public ClusterControllerSettings cloneSettings(ClusterControllerSettings settings) {
    return settings.cloneSettings();
  }

  @Override
  protected void _updateSettings(ClusterControllerSettings settings) throws AdminException {
    wrapped.updateSettings(settings);
  }

  @Override
  protected void logChanges(ClusterControllerSettings oldSettings, ClusterControllerSettings settings) {
    findChanges(oldSettings, settings, ClusterControllerSettings.class, new DiffHelper.ChangeListener() {
      public void foundChange(String propertyName, Object oldValue, Object newValue) {
        j.user(user).change("property_changed", propertyName, valueToString(oldValue), valueToString(newValue)).clusterController();
      }
    });
  }

  public void startClusterController() throws AdminException {
    wrapped.startClusterController();
    j.user(user).start().clusterController();
  }

  public void stopClusterController() throws AdminException {
    wrapped.stopClusterController();
    j.user(user).stop().clusterController();
  }

  public void switchClusterController(String toHost) throws AdminException {
    wrapped.switchClusterController(toHost);
    j.user(user).switchTo(toHost).clusterController();
  }

  public String getControllerOnlineHost() throws AdminException {
    return wrapped.getControllerOnlineHost();
  }

  public List<String> getControllerHosts() throws AdminException {
    return wrapped.getControllerHosts();
  }
}
