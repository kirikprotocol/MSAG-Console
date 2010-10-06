package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;

import javax.faces.application.FacesMessage;
import javax.faces.component.html.HtmlSelectOneMenu;
import javax.faces.event.ActionEvent;
import javax.faces.event.ValueChangeEvent;
import javax.faces.model.SelectItem;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class SmscStatusController extends SmscController {

  private String switchToHost;

  private final SmscManager smscManager;
  private final Map<String, SmscConfiguration> configs;

  public SmscStatusController() {
    WebContext ctx = WebContext.getInstance();
    smscManager = ctx.getSmscManager();
    configs = new HashMap<String, SmscConfiguration>();

    configs.put("acl", ctx.getAclManager());
    configs.put("alias", ctx.getAliasManager());
    configs.put("closed_group", ctx.getClosedGroupManager());
    configs.put("fraud", ctx.getFraudManager());
    configs.put("map_limit", ctx.getMapLimitManager());
    configs.put("msc", ctx.getMscManager());
    configs.put("profile", ctx.getProfileManager());
    configs.put("reschedule", ctx.getRescheduleManager());
    configs.put("resource", ctx.getResourceManager());
    configs.put("sme", ctx.getSmeManager());
    configs.put("smsc", ctx.getSmscManager());
    configs.put("snmp", ctx.getSnmpManager());
    configs.put("timezone", ctx.getTimezoneManager());
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
        smscManager.switchSmsc(Integer.parseInt(instanceNumber), switchToHost);
      } catch (AdminException e) {
        logError(e);
      }
  }

  public void start() {
    String instanceNumber = getRequestParameter("instanceNumber");
    if (instanceNumber != null)
      try {
        smscManager.startSmsc(Integer.parseInt(instanceNumber));
      } catch (AdminException e) {
        logError(e);
      }
  }

  public void stop() {
    String instanceNumber = getRequestParameter("instanceNumber");
    if (instanceNumber != null)
      try {
        smscManager.stopSmsc(Integer.parseInt(instanceNumber));
      } catch (AdminException e) {
        logError(e);
      }
  }

  public void startAll(ActionEvent e) {
    try {
      for (int i = 0; i < smscManager.getSettings().getSmscInstancesCount(); i++) {
        if (smscManager.getSmscOnlineHost(i) == null)
          smscManager.startSmsc(i);
      }
    } catch (AdminException ex) {
      logError(ex);
    }
  }

  public void stopAll(ActionEvent e) {
    try {
      for (int i = 0; i < smscManager.getSettings().getSmscInstancesCount(); i++) {
        if (smscManager.getSmscOnlineHost(i) != null)
          smscManager.stopSmsc(i);
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
      for (int i = 0; i < smscManager.getSettings().getSmscInstancesCount(); i++) {
        String onlineHost = smscManager.getSmscOnlineHost(i);
        SmscStatus status = new SmscStatus(i, onlineHost, smscManager.getSmscHosts(i));
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
  public class SmscStatus implements Serializable {

    private final int instanceNumber;
    private final String onlineHost;
    private final List<String> hosts;
    private List<String> errors;

    public SmscStatus(int instanceNumber, String onlineHost, List<String> hosts) {
      this.instanceNumber = instanceNumber;
      this.onlineHost = onlineHost;
      this.hosts = hosts;

      if (onlineHost != null) {
        errors = new ArrayList<String>();
        for (Map.Entry<String, SmscConfiguration> e : configs.entrySet()) {
          SmscConfiguration cfg = e.getValue();
          try {
            if (cfg.getStatusForSmscs().get(instanceNumber) == SmscConfigurationStatus.OUT_OF_DATE)
              errors.add("status.page." + e.getKey());
          } catch (AdminException e1) {
            addError(e1);
          }
        }
      }
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

    public List<String> getErrors() {
      return errors;
    }

    public boolean isHasErrors() {
      return onlineHost != null && !errors.isEmpty();
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
