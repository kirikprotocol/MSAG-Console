package ru.novosoft.smsc.web.config.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.config.DiffHelper;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.lang.reflect.Method;
import java.util.List;

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
    ArchiveDaemonSettings oldSettings = getSettings();
    wrapped.updateSettings(settings);
    List<Method> getters = DiffHelper.getGetters(ArchiveDaemonSettings.class);
    List<Object> oldValues = DiffHelper.callGetters(getters, oldSettings);
    List<Object> newValues = DiffHelper.callGetters(getters, settings);
    DiffHelper.logChanges(j, JournalRecord.Subject.ARCHIVE_DAEMON, oldValues, newValues, getters, user);
  }

  public ArchiveDaemonSettings cloneSettings(ArchiveDaemonSettings settings) {
    return settings.cloneSettings();
  }

  public String getDaemonOnlineHost() throws AdminException {
    return wrapped.getDaemonOnlineHost();
  }

  public void switchDaemon(String toHost) throws AdminException {
    wrapped.switchDaemon(toHost);
    JournalRecord r = j.addRecord(JournalRecord.Type.SWITCH, JournalRecord.Subject.ARCHIVE_DAEMON, user);
    r.setDescription("archive_daemon.switched", toHost);
  }

  public void startDaemon() throws AdminException {
    wrapped.startDaemon();
    JournalRecord r = j.addRecord(JournalRecord.Type.START, JournalRecord.Subject.ARCHIVE_DAEMON, user);
    r.setDescription("archive_daemon.started");
  }

  public void stopDaemon() throws AdminException {
    wrapped.stopDaemon();
    JournalRecord r = j.addRecord(JournalRecord.Type.STOP, JournalRecord.Subject.ARCHIVE_DAEMON, user);
    r.setDescription("archive_daemon.stopped");
  }

  public List<String> getDaemonHosts() throws AdminException {
    return wrapped.getDaemonHosts();
  }
}
