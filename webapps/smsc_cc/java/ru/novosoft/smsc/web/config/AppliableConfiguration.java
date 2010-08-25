package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;

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

  public AppliableConfiguration(AdminContext adminContext) throws AdminException {
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

  public void setSmscSettings(SmscSettings smscSettings, String user) {
    this.smscSettings = smscSettings.cloneSettings();
    smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }

  public void applySmscSettings(String user) throws AdminException {
    adminContext.getSmscManager().updateSettings(smscSettings);
    smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, false);
  }

  public void resetSmscSettings(String user) throws AdminException {
    this.smscSettings = adminContext.getSmscManager().getSettings();
    smscSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, false);
  }

  public UpdateInfo getSmscSettigsUpdateInfo() {
    return smscSettingsUpdateInfo;
  }

  // RESCHEDULE ========================================================================================================

  public RescheduleSettings getRescheduleSettings() {
    return rescheduleSettings.cloneSettings();
  }

  public void setRescheduleSettings(RescheduleSettings rescheduleSettings, String user) {
    this.rescheduleSettings = rescheduleSettings.cloneSettings();
    rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, true);
  }

  public void applyRescheduleSettings(String user) throws AdminException {
    adminContext.getRescheduleManager().updateSettings(rescheduleSettings);
    rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, false);
  }

  public void resetRescheduleSettings(String user) throws AdminException {
    this.rescheduleSettings = adminContext.getRescheduleManager().getSettings();
    rescheduleSettingsUpdateInfo = new UpdateInfo(System.currentTimeMillis(), user, false);
  }

  public Map<Integer, SmscConfigurationStatus> getRescheduleSettingsStatus() throws AdminException {
    return adminContext.getRescheduleManager().getStatusForSmscs();
  }

  public UpdateInfo getRescheduleSettingsUpdateInfo() {
    return rescheduleSettingsUpdateInfo;
  }
}
