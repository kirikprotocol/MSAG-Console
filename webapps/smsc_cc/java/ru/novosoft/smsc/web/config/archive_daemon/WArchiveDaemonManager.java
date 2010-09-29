package ru.novosoft.smsc.web.config.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonSettings;
import ru.novosoft.smsc.web.config.DiffHelper;
import ru.novosoft.smsc.web.config.SettingsManager;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.lang.reflect.Method;
import java.util.List;

/**
 * Декоратор, добавляющий журналирование вызовов ArchiveDaemonManager
 *
 * @author Artem Snopkov
 */
public class WArchiveDaemonManager implements ArchiveDaemonManager, SettingsManager<ArchiveDaemonSettings> {

  private static String lastUpdateUser;
  private static long lastUpdateTime;

  private final ArchiveDaemonManager wrapped;
  private final Journal j;
  private final String user;

  public WArchiveDaemonManager(ArchiveDaemonManager wrapped, Journal j, String user) {
    this.wrapped = wrapped;
    this.j = j;
    this.user = user;
  }

  public ArchiveDaemonSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public void updateSettings(ArchiveDaemonSettings settings) throws AdminException {
    ArchiveDaemonSettings oldSettings = getSettings();
    wrapped.updateSettings(settings);
    List<Method> getters = DiffHelper.getGetters(ArchiveDaemonSettings.class);
    List<Object> oldValues = DiffHelper.callGetters(getters, oldSettings);
    List<Object> newValues = DiffHelper.callGetters(getters, settings);
    DiffHelper.logChanges(j, JournalRecord.Subject.ARCHIVE_DAEMON, oldValues, newValues, getters, user);
    lastUpdateUser = user;
    lastUpdateTime = System.currentTimeMillis();
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

  public String getLastUpdateUser() {
    return lastUpdateUser;
  }

  public long getLastUpdateTime() {
    return lastUpdateTime;
  }
}
