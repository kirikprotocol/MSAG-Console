package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.admin.fraud.FraudSettings;
import ru.novosoft.smsc.admin.logging.LoggerSettings;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.sme.Sme;
import ru.novosoft.smsc.admin.sme.SmeServiceStatus;
import ru.novosoft.smsc.admin.sme.SmeSmscStatuses;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.snmp.SnmpSettings;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

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

  public UsersSettings getUsersSettings() throws AdminException {
    return adminContext.getUsersManager().getUsersSettings();
  }

  public void updateUsersSettings(UsersSettings settings, String user) throws AdminException {
    UsersSettings oldSettings = getUsersSettings();
    adminContext.getUsersManager().updateSettings(settings);
    journal.logChanges(oldSettings, settings, user);
  }


  public SnmpSettings getSnmpSettings() throws AdminException {
    return adminContext.getSnmpManager().getSettings();
  }


  public void updateSnmpSettings(SnmpSettings settings, String user) throws AdminException {
    adminContext.getSnmpManager().updateSettings(settings);
  }

}