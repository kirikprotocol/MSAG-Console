package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.web.config.changelog.ChangeLogRecord;
import ru.novosoft.smsc.web.config.changelog.LocalChangeLog;

import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class AppliableConfiguration {

  private final AdminContext adminContext;

  private SmscSettings smscSettings;
  private UpdateInfo smscSettingsUpdateInfo;

  private RescheduleSettings rescheduleSettings;
  private UpdateInfo rescheduleSettingsUpdateInfo;

  private LocalChangeLog changeLog;

  public AppliableConfiguration(AdminContext adminContext) throws AdminException {
    this.adminContext = adminContext;
    this.changeLog = new LocalChangeLog();

    this.smscSettings = adminContext.getSmscManager().getSettings();
    this.smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);

    this.rescheduleSettings = adminContext.getRescheduleManager().getSettings();
    this.rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);
  }

  public boolean hasNotAppliedChanges() {
    return !changeLog.isEmpty();
  }

  // SMSC ==============================================================================================================

  public SmscSettings getSmscSettings() {
    return smscSettings.cloneSettings();
  }

  public void setSmscSettings(SmscSettings smscSettings, String user) {
    changeLog.logChanges(this.smscSettings, smscSettings, user);
    this.smscSettings = smscSettings.cloneSettings();
    smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }

  public void applySmscSettings(String user) throws AdminException {
    adminContext.getSmscManager().updateSettings(smscSettings);
    // todo скопировать все записи для сабжекта SMSC в журнал
    changeLog.removeRecords(LocalChangeLog.SMSC);
  }

  public void resetSmscSettings(String user) throws AdminException {
    this.smscSettings = adminContext.getSmscManager().getSettings();
    changeLog.removeRecords(LocalChangeLog.SMSC);
  }

  public UpdateInfo getSmscSettingsUpdateInfo() {
    return smscSettingsUpdateInfo;
  }

  public List<ChangeLogRecord> getSmscSettingsChanges() {
    return changeLog.getRecords(LocalChangeLog.SMSC);
  }

  // RESCHEDULE ========================================================================================================

  public RescheduleSettings getRescheduleSettings() {
    return rescheduleSettings.cloneSettings();
  }

  public void setRescheduleSettings(RescheduleSettings rescheduleSettings, String user) {
    changeLog.logChanges(this.rescheduleSettings, rescheduleSettings, user);
    this.rescheduleSettings = rescheduleSettings.cloneSettings();
    rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }

  public void applyRescheduleSettings(String user) throws AdminException {
    adminContext.getRescheduleManager().updateSettings(rescheduleSettings);
    // todo скопировать все записи для сабжекта RESCHEDULE в журнал
    changeLog.removeRecords(LocalChangeLog.RESCHEDULE);
  }

  public void resetRescheduleSettings(String user) throws AdminException {
    this.rescheduleSettings = adminContext.getRescheduleManager().getSettings();
    changeLog.removeRecords(LocalChangeLog.RESCHEDULE);
  }

  public UpdateInfo getRescheduleSettingsUpdateInfo() {
    return rescheduleSettingsUpdateInfo;
  }

  public List<ChangeLogRecord> getRescheduleSettingsChanges() {
    return changeLog.getRecords(LocalChangeLog.RESCHEDULE);
  }
}
