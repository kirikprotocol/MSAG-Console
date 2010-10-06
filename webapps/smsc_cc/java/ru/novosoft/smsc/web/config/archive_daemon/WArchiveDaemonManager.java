package ru.novosoft.smsc.web.config.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.config.DiffHelper;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.List;

import static ru.novosoft.smsc.web.config.DiffHelper.ChangeListener;
import static ru.novosoft.smsc.web.config.DiffHelper.findChanges;

/**
 * Декоратор, добавляющий журналирование вызовов ArchiveDaemonManager
 *
 * @author Artem Snopkov
 */
public class WArchiveDaemonManager extends BaseSettingsManager<ArchiveDaemonSettings> implements ArchiveDaemonManager {


  private final ArchiveDaemonManager wrapped;
  private final Journal j;

  public WArchiveDaemonManager(ArchiveDaemonManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  public ArchiveDaemonSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public void _updateSettings(ArchiveDaemonSettings settings) throws AdminException {
    wrapped.updateSettings(settings);
  }

  @Override
  protected void logChanges(ArchiveDaemonSettings oldSettings, ArchiveDaemonSettings newSettings) {
    findChanges(oldSettings, newSettings, ArchiveDaemonSettings.class, new ChangeListener() {
      public void foundChange(String propertyName, Object oldValue, Object newValue) {
        j.user(user).change("property.changed", propertyName, DiffHelper.valueToString(oldValue), DiffHelper.valueToString(newValue));
      }
    });
  }

  public ArchiveDaemonSettings cloneSettings(ArchiveDaemonSettings settings) {
    return settings.cloneSettings();
  }

  public String getDaemonOnlineHost() throws AdminException {
    return wrapped.getDaemonOnlineHost();
  }

  public void switchDaemon(String toHost) throws AdminException {
    wrapped.switchDaemon(toHost);
    j.user(user).switchTo(toHost).archiveDaemon();
  }

  public void startDaemon() throws AdminException {
    wrapped.startDaemon();
    j.user(user).start().archiveDaemon();
  }

  public void stopDaemon() throws AdminException {
    wrapped.stopDaemon();
    j.user(user).stop().archiveDaemon();    
  }

  public List<String> getDaemonHosts() throws AdminException {
    return wrapped.getDaemonHosts();
  }
}
