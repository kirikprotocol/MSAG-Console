package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.List;

/**
 * @author Artem Snopkov
 */
public class AppliableConfiguration {

  private final AdminContext adminContext;
  private final Journal journal;

  private UpdateInfo smscSettingsUpdateInfo;
  private UpdateInfo rescheduleSettingsUpdateInfo;
  private UpdateInfo usersSettingsUpdateInfo;
  private UpdateInfo mapLimitSettingsUpdateInfo;

  public AppliableConfiguration(AdminContext adminContext, Journal journal) throws AdminException {
    this.adminContext = adminContext;
    this.journal = journal;

    this.smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);

    this.rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);

    this.usersSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);

    this.mapLimitSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);
  }

  // SMSC ==============================================================================================================

  public SmscSettings getSmscSettings() throws AdminException {
    return adminContext.getSmscManager().getSettings();
  }

  public void setSmscSettings(SmscSettings smscSettings, String user) throws AdminException {
    SmscSettings oldSettings = adminContext.getSmscManager().getSettings();
    adminContext.getSmscManager().updateSettings(smscSettings);
    journal.logChanges(oldSettings, smscSettings, user);
    smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }

  public UpdateInfo getSmscSettingsUpdateInfo() {
    return smscSettingsUpdateInfo;
  }

  // RESCHEDULE ========================================================================================================

  public RescheduleSettings getRescheduleSettings() throws AdminException {
    return adminContext.getRescheduleManager().getSettings();
  }

  public void setRescheduleSettings(RescheduleSettings rescheduleSettings, String user) throws AdminException {
    RescheduleSettings oldSettings = adminContext.getRescheduleManager().getSettings();
    adminContext.getRescheduleManager().updateSettings(rescheduleSettings);
    journal.logChanges(oldSettings, rescheduleSettings, user);
    rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }

  public UpdateInfo getRescheduleSettingsUpdateInfo() {
    return rescheduleSettingsUpdateInfo;
  }

  // USERS ========================================================================================================

  public UsersSettings getUsersSettings() throws AdminException {
    return adminContext.getUsersManager().getUsersSettings();
  }

  public void setUsersSettings(UsersSettings settings, String user) throws AdminException {
    UsersSettings oldSettings = adminContext.getUsersManager().getUsersSettings();
    adminContext.getUsersManager().updateSettings(settings);
    journal.logChanges(oldSettings, settings, user);
    usersSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }

  public UpdateInfo getUsersSettingsUpdateInfo() {
    return usersSettingsUpdateInfo;
  }

  // MAP LIMITS ========================================================================================================

  public MapLimitSettings getMapLimitSettings() throws AdminException {
    return adminContext.getMapLimitManager().getSettings();
  }

  public void setMapLimitSettings(MapLimitSettings settings, String user) throws AdminException {
    MapLimitSettings oldSettings = adminContext.getMapLimitManager().getSettings();
    adminContext.getMapLimitManager().updateSettings(settings);
    journal.logChanges(oldSettings, settings, user);
    mapLimitSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }
}