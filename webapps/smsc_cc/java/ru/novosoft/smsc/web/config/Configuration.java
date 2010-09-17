package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.admin.fraud.FraudSettings;
import ru.novosoft.smsc.admin.logging.LoggerSettings;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.ArrayList;
import java.util.List;

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

  public LoggerSettings getLoggerSettings() throws AdminException {
    return adminContext.getLoggerManager().getSettings();
  }

  public void updateLoggerSettings(LoggerSettings loggerSettings, String user) throws AdminException {
    LoggerSettings oldSettings = getLoggerSettings();
    adminContext.getLoggerManager().updateSettings(loggerSettings);
    journal.logChanges(oldSettings, loggerSettings, user);
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

  public ClosedGroup getClosedGroup(int groupId, String user) throws AdminException {
    return new LoggedClosedGroup(adminContext.getClosedGroupManager().getGroup(groupId), journal, user);
  }

  public List<ClosedGroup> getClosedGroups(String user) throws AdminException {
    List<ClosedGroup> gs = adminContext.getClosedGroupManager().groups();
    List<ClosedGroup> result = new ArrayList<ClosedGroup>(gs.size());
    for(ClosedGroup g : gs) {
      result.add(new LoggedClosedGroup(g, journal, user));
    }
    return result;
  }

  public ClosedGroup addGroup(String name, String description, String user) throws AdminException {
    ClosedGroup g = new LoggedClosedGroup(adminContext.getClosedGroupManager().addGroup(name, description), journal, user);
    journal.logClosedGroupAdd(name, user);
    return g;
  }

  public boolean removeGroup(int groupId, String user) throws AdminException {
    ClosedGroup cg = adminContext.getClosedGroupManager().removeGroup(groupId);
    if(cg != null) {
      journal.logClosedGroupRemove(cg.getName(), user);
      return true;
    }
    return false;
  }

  public FraudSettings getFraudSettings() throws AdminException {
    return adminContext.getFraudManager().getSettings();
  }

  public void updateFraudSettings(FraudSettings settings, String user) throws AdminException{
    FraudSettings oldSettings = getFraudSettings();
    adminContext.getFraudManager().updateSettings(settings);
    journal.logChanges(oldSettings, settings, user);
  }
}