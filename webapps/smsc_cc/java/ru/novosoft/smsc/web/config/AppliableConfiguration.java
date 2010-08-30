package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.config.changelog.ChangeLogRecord;
import ru.novosoft.smsc.web.config.changelog.LocalChangeLog;

import java.util.List;

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

  private UsersSettings usersSettings;

  private UpdateInfo usersSettingsUpdateInfo;

  public AppliableConfiguration(AdminContext adminContext) throws AdminException {
    this.adminContext = adminContext;
    this.changeLog = new LocalChangeLog();

    this.smscSettings = adminContext.getSmscManager().getSettings();
    this.smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);

    this.rescheduleSettings = adminContext.getRescheduleManager().getSettings();
    this.rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);

    this.usersSettings = adminContext.getUsersManager().getUsersSettings();
    this.usersSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);
  }

  public boolean hasNotAppliedChanges() {
    return !changeLog.isEmpty();
  }

  public void applyAll(String user) throws AdminException {
    applySmscSettings(user);
    applyRescheduleSettings(user);
    applyUsersSettings(user);
  }

  public void resetAll(String user) throws AdminException {
    resetSmscSettings(user);
    resetRescheduleSettings(user);
    resetUsersSettings(user);
  }

  // SMSC==============================================================================================================

  public SmscSettings getSmscSettings() {
    return smscSettings.cloneSettings();
  }

  public UsersSettings getUsersSettings() {
    return usersSettings.cloneSettings();
  }

  public void setUsersSettings(UsersSettings settings, String user) {
    changeLog.logChanges(this.usersSettings, settings, user);
    this.usersSettings = settings.cloneSettings();
    usersSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }

  public void applyUsersSettings(String user) throws AdminException{
    if (changeLog.hasRecords(LocalChangeLog.USERS)) {
      adminContext.getUsersManager().updateSettings(usersSettings);
      // todo скопировать все записи для сабжекта SMSC в журнал
      changeLog.removeRecords(LocalChangeLog.USERS);
    }
  }

  public void resetUsersSettings(String user) throws AdminException {
    this.usersSettings = adminContext.getUsersManager().getUsersSettings();
    changeLog.removeRecords(LocalChangeLog.USERS);
  }

  public List<ChangeLogRecord> getUsersSettingsChanges() {
    return changeLog.getRecords(LocalChangeLog.USERS);
  }


  public void setSmscSettings(SmscSettings smscSettings, String user) {
    changeLog.logChanges(this.smscSettings, smscSettings, user);
    this.smscSettings = smscSettings.cloneSettings();
    smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }

  public void applySmscSettings(String user) throws AdminException {
    if (changeLog.hasRecords(LocalChangeLog.SMSC)) {
      adminContext.getSmscManager().updateSettings(smscSettings);
      // todo скопировать все записи для сабжекта SMSC в журнал
      changeLog.removeRecords(LocalChangeLog.SMSC);
    }
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

  // RESCHEDULE========================================================================================================

  public RescheduleSettings getRescheduleSettings() {
    return rescheduleSettings.cloneSettings();
  }

  public void setRescheduleSettings(RescheduleSettings rescheduleSettings, String user) {
    changeLog.logChanges(this.rescheduleSettings, rescheduleSettings, user);
    this.rescheduleSettings = rescheduleSettings.cloneSettings();
    rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }

  public void applyRescheduleSettings(String user) throws AdminException {
    if (changeLog.hasRecords(LocalChangeLog.RESCHEDULE)) {
      adminContext.getRescheduleManager().updateSettings(rescheduleSettings);
      // todo скопировать все записи для сабжекта RESCHEDULE в журнал
      changeLog.removeRecords(LocalChangeLog.RESCHEDULE);
    }
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

  public UpdateInfo getUsersSettingsUpdateInfo() {
    return usersSettingsUpdateInfo;
  }
}