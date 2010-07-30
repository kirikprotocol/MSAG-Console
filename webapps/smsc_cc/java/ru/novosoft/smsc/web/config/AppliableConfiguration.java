package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.web.auth.User;

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

  public AppliableConfiguration(AdminContext adminContext) {
    this.adminContext = adminContext;
    this.smscSettings = adminContext.getSmscManager().getSettings();
    this.smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);
    
    this.rescheduleSettings = adminContext.getRescheduleManager().getSettings();
    this.rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), null, false);
  }

  // SMSC ==============================================================================================================

  public Map<Integer, SmscConfigurationStatus> getSmscSettingsStatus() throws AdminException {
    return adminContext.getSmscManager().getStatusForSmscs();
  }
  
  public SmscSettings getSmscSettings() {
    return smscSettings.cloneSettings();
  }

  public void setSmscSettings(SmscSettings smscSettings, String String) {
    this.smscSettings = smscSettings.cloneSettings();
    smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), String, true);
  }

  public void applySmscSettings(String String) throws AdminException {
    adminContext.getSmscManager().updateSettings(smscSettings);
    smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), String, false);
  }

  public void resetSmscSettings(String String) {
    this.smscSettings = adminContext.getSmscManager().getSettings();
    smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), String, false);
  }

  public UpdateInfo getSmscSettigsUpdateInfo() {
    return smscSettingsUpdateInfo;
  }

  // RESCHEDULE ========================================================================================================

  public RescheduleSettings getRescheduleSettings() {
    return rescheduleSettings.cloneSettings();
  }

  public void setRescheduleSettings(RescheduleSettings rescheduleSettings, String String) {
    this.rescheduleSettings = rescheduleSettings.cloneSettings();
    rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), String, true);
  }

  public void applyRescheduleSettings(String String) throws AdminException {
    adminContext.getRescheduleManager().updateSettings(rescheduleSettings);
    rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), String, false);
  }

  public void resetRescheduleSettings(String String) {
    this.rescheduleSettings = adminContext.getRescheduleManager().getSettings();
    rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), String, false);
  }

  public UpdateInfo getRescheduleSettingsUpdateInfo() {
    return rescheduleSettingsUpdateInfo;
  }
}
