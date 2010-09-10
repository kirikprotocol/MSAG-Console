package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.journal.Journal;

/**
 * @author Artem Snopkov
 */
public class Configuration {

  private final AdminContext adminContext;
  private final Journal journal;

  public Configuration(AdminContext adminContext, Journal journal) {
    this.adminContext = adminContext;
    this.journal = journal;
  } 

  public SmscSettings getSmscSettings() throws AdminException {
    return adminContext.getSmscManager().getSettings();
  }

  public void updateSmscSettings(SmscSettings smscSettings, String user) throws AdminException {
    SmscSettings oldSettings = getSmscSettings();
    adminContext.getSmscManager().updateSettings(smscSettings);
    journal.logChanges(oldSettings, smscSettings, user);
  }

  public RescheduleSettings getRescheduleSettings() throws AdminException {
    return adminContext.getRescheduleManager().getSettings();
  }

  public void updateRescheduleSettings(RescheduleSettings rescheduleSettings, String user) throws AdminException {
    RescheduleSettings oldSettings = getRescheduleSettings();
    adminContext.getRescheduleManager().updateSettings(rescheduleSettings);
    journal.logChanges(oldSettings, rescheduleSettings, user);
  }

  public UsersSettings getUsersSettings() throws AdminException {
    return adminContext.getUsersManager().getUsersSettings();
  }

  public void updateUsersSettings(UsersSettings settings, String user) throws AdminException {
    UsersSettings oldSettings = getUsersSettings();
    adminContext.getUsersManager().updateSettings(settings);
    journal.logChanges(oldSettings, settings, user);
  }


  public MapLimitSettings getMapLimitSettings() throws AdminException {
    return adminContext.getMapLimitManager().getSettings();
  }

  public void updateMapLimitSettings(MapLimitSettings settings, String user) throws AdminException {
    MapLimitSettings oldSettings = getMapLimitSettings();
    adminContext.getMapLimitManager().updateSettings(settings);
    journal.logChanges(oldSettings, settings, user);
  }

}
