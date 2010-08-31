package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.config.AppliableConfiguration;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.config.changelog.ChangeLogRecord;

import javax.faces.application.FacesMessage;
import javax.faces.component.html.HtmlSelectOneMenu;
import javax.faces.event.ActionEvent;
import javax.faces.event.ValueChangeEvent;
import javax.faces.model.SelectItem;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class SmscStatusController extends SmscController {

  private final SmscStatusManager smscStatusManager;
  private final AppliableConfiguration appliableConfiguration;

  private String switchToHost;

  public SmscStatusController() {
    smscStatusManager = WebContext.getInstance().getSmscStatusManager();
    appliableConfiguration = WebContext.getInstance().getAppliableConfiguration();
  }

  public void reset() {
    String configName = getRequestParameter("configName");
    if (configName == null)
      return;
    try {
      if (configName.equals("smsc"))
        appliableConfiguration.resetSmscSettings(getUserPrincipal().getName());
      else if (configName.equals("reschedule"))
        appliableConfiguration.resetRescheduleSettings(getUserPrincipal().getName());
      else if (configName.equals("users"))
        appliableConfiguration.resetUsersSettings(getUserPrincipal().getName());

    } catch (AdminException e) {
      logError(e);
    }
  }

  public void applyAll() {
    try {
      appliableConfiguration.applyAll(getUserPrincipal().getName());
    } catch (AdminException e) {
      logError(e);
    }
  }

  public void resetAll() {
    try {
      appliableConfiguration.resetAll(getUserPrincipal().getName());
    } catch (AdminException e) {
      logError(e);
    }
  }

  public DataTableModel getConfigChanges() {
    List<ConfigChanges> result = new ArrayList<ConfigChanges>();
    result.add(new ConfigChanges("smsc", appliableConfiguration.getSmscSettingsChanges()));
    result.add(new ConfigChanges("reschedule", appliableConfiguration.getRescheduleSettingsChanges()));
    result.add(new ConfigChanges("users", appliableConfiguration.getUsersSettingsChanges()));
    return new ListTableModel(result);
  }

  public void switchToHost(ValueChangeEvent e) {
    if (e.getSource() instanceof HtmlSelectOneMenu) {
      HtmlSelectOneMenu src = (HtmlSelectOneMenu) e.getSource();
      Integer selectInstanceNum = (Integer) src.getAttributes().get("instanceNumber");
      if (selectInstanceNum != null && String.valueOf(selectInstanceNum).equals(getRequestParameter("instanceNumber")))
        this.switchToHost = (String) e.getNewValue();
    }
  }

  public void switchOver() {
    String instanceNumber = getRequestParameter("instanceNumber");
    if (instanceNumber != null && switchToHost != null)
      try {
        smscStatusManager.switchSmsc(Integer.parseInt(instanceNumber), switchToHost);
      } catch (AdminException e) {
        logError(e);
      }
  }

  public void start() {
    String instanceNumber = getRequestParameter("instanceNumber");
    if (instanceNumber != null)
      try {
        smscStatusManager.startSmsc(Integer.parseInt(instanceNumber));
      } catch (AdminException e) {
        logError(e);
      }
  }

  public void stop() {
    String instanceNumber = getRequestParameter("instanceNumber");
    if (instanceNumber != null)
      try {
        smscStatusManager.stopSmsc(Integer.parseInt(instanceNumber));
      } catch (AdminException e) {
        logError(e);
      }
  }

  public void startAll(ActionEvent e) {
    try {
      for (int i = 0; i < smscStatusManager.getSmscInstancesNumber(); i++) {
        if (smscStatusManager.getSmscOnlineHost(i) == null)
          smscStatusManager.startSmsc(i);
      }
    } catch (AdminException ex) {
      logError(ex);
    }
  }

  public void stopAll(ActionEvent e) {
    try {
      for (int i = 0; i < smscStatusManager.getSmscInstancesNumber(); i++) {
        if (smscStatusManager.getSmscOnlineHost(i) != null)
          smscStatusManager.stopSmsc(i);
      }
    } catch (AdminException ex) {
      logError(ex);
    }
  }

  private void logError(AdminException e) {
    addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
  }

  public DataTableModel getSmsCenters() {

    List<SmscStatus> result = new ArrayList<SmscStatus>();

    try {
      for (int i = 0; i < smscStatusManager.getSmscInstancesNumber(); i++) {

        String onlineHost = smscStatusManager.getSmscOnlineHost(i);

        SmscStatus status = new SmscStatus(i, onlineHost, smscStatusManager.getSmscHosts(i));
        if (onlineHost != null) {
          status.setMainConfigUpToDate(smscStatusManager.getMainConfigState(i) == SmscConfigurationStatus.UP_TO_DATE);
          status.setRescheduleConfigUpToDate(smscStatusManager.getRescheduleState(i) == SmscConfigurationStatus.UP_TO_DATE);
        }

        result.add(status);
      }
    } catch (AdminException e) {
      logError(e);
    }

    return new ListTableModel(result);
  }

  /**
   *
   */
  public static class ConfigChanges implements Serializable {

    private final String configName;
    private final List<ChangeRecord> changes;

    public ConfigChanges(String configName, List<ChangeLogRecord> changes) {
      this.configName = configName;
      this.changes = new ArrayList<ChangeRecord>(changes.size());
      for(ChangeLogRecord clr : changes) {
        this.changes.add(new ChangeRecord(clr));
      }
    }

    public List<ChangeRecord> getChanges() {
      return changes;
    }

    public String getConfigName() {
      return configName;
    }

    public boolean isChanged() {
      return !changes.isEmpty();
    }
  }

  public static class ChangeRecord implements Serializable {

    private ChangeLogRecord record;

    private String userName;

    private String userLastName;
    private String userDept;

    public ChangeRecord(ChangeLogRecord record) {
      this.record = record;
      User u = WebContext.getInstance().getAppliableConfiguration().getUsersSettings().getUser(record.getUser());
      if(u != null) {
        userName = u.getFirstName();
        userLastName = u.getLastName();
        userDept = u.getDept();
      }
    }

    public long getTime() {
      return record.getTime();
    }

    public String getSubject() {
      return record.getSubject();
    }

    public String getUser() {
      return record.getUser();
    }

    public String getDescription() {
      return record.getDescription();
    }

    public ChangeLogRecord.Type getType() {
      return record.getType();
    }

    public String getUserName() {
      return userName;
    }

    public String getUserLastName() {
      return userLastName;
    }

    public String getUserDept() {
      return userDept;
    }
  }

  /**
   *
   */
  public static class SmscStatus implements Serializable {

    private final int instanceNumber;
    private final String onlineHost;
    private final List<String> hosts;
    private boolean mainConfigUpToDate;
    private boolean rescheduleConfigUpToDate;

    public SmscStatus(int instanceNumber, String onlineHost, List<String> hosts) {
      this.instanceNumber = instanceNumber;
      this.onlineHost = onlineHost;
      this.hosts = hosts;
    }

    public int getInstanceNumber() {
      return instanceNumber;
    }

    public String getOnlineHost() {
      return onlineHost;
    }

    public List<SelectItem> getHosts() {
      List<SelectItem> items = new ArrayList<SelectItem>(hosts.size());
      for (String host : hosts) {
        if (onlineHost == null || !host.equals(onlineHost))
          items.add(new SelectItem(host, host));
      }
      return items;
    }

    public boolean isSwitchAllowed() {
      return hosts != null && hosts.size() > 1;
    }

    public boolean isMainConfigUpToDate() {
      return mainConfigUpToDate;
    }

    public void setMainConfigUpToDate(boolean mainConfigUpToDate) {
      this.mainConfigUpToDate = mainConfigUpToDate;
    }

    public boolean isRescheduleConfigUpToDate() {
      return rescheduleConfigUpToDate;
    }

    public void setRescheduleConfigUpToDate(boolean rescheduleConfigUpToDate) {
      this.rescheduleConfigUpToDate = rescheduleConfigUpToDate;
    }

    public boolean isHasErrors() {
      return onlineHost != null &&
          (!rescheduleConfigUpToDate || !mainConfigUpToDate);
    }
  }

  /**
   *
   */
  public class ListTableModel implements DataTableModel {

    private final List values;

    public ListTableModel(List values) {
      this.values = values;
    }

    public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
      return values;
    }

    public int getRowsCount() {
      return values.size();
    }
  }

}
