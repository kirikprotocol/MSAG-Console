package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.config.SmscStatusManager;

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

  private String switchToHost;

  public SmscStatusController() {
    smscStatusManager = WebContext.getInstance().getSmscStatusManager();
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
          SmscConfigurationStatus mainConfigState = smscStatusManager.getMainConfigState(i);
          status.setMainConfigUpToDate(mainConfigState == null || mainConfigState == SmscConfigurationStatus.UP_TO_DATE);

          SmscConfigurationStatus rescheduleConfigState = smscStatusManager.getRescheduleState(i);
          status.setRescheduleConfigUpToDate(rescheduleConfigState == null || rescheduleConfigState == SmscConfigurationStatus.UP_TO_DATE);

          SmscConfigurationStatus fraudConfigState = smscStatusManager.getFraudConfigState(i);
          status.setFraudConfigUpToDate(fraudConfigState == null || fraudConfigState == SmscConfigurationStatus.UP_TO_DATE);

          SmscConfigurationStatus mapLimitsState = smscStatusManager.getMapLimitState(i);
          status.setMapLimitsUpToDate(mapLimitsState == null || mapLimitsState == SmscConfigurationStatus.UP_TO_DATE);

          SmscConfigurationStatus cgState = smscStatusManager.getClosedGroupsState(i);
          status.setClosedGroupsUpToDate(cgState == null || cgState == SmscConfigurationStatus.UP_TO_DATE);
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
  public static class SmscStatus implements Serializable {

    private final int instanceNumber;
    private final String onlineHost;
    private final List<String> hosts;

    private boolean mainConfigUpToDate;
    private boolean rescheduleConfigUpToDate;
    private boolean fraudConfigUpToDate;
    private boolean mapLimitsUpToDate;
    private boolean closedGroupsUpToDate;

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

    public boolean isFraudConfigUpToDate() {
      return fraudConfigUpToDate;
    }

    public void setFraudConfigUpToDate(boolean fraudConfigUpToDate) {
      this.fraudConfigUpToDate = fraudConfigUpToDate;
    }

    public boolean isMapLimitsUpToDate() {
      return mapLimitsUpToDate;
    }

    public void setMapLimitsUpToDate(boolean mapLimitsUpToDate) {
      this.mapLimitsUpToDate = mapLimitsUpToDate;
    }

    public boolean isClosedGroupsUpToDate() {
      return closedGroupsUpToDate;
    }

    public void setClosedGroupsUpToDate(boolean closedGroupsUpToDate) {
      this.closedGroupsUpToDate = closedGroupsUpToDate;
    }

    public boolean isHasErrors() {
      return onlineHost != null &&
          (
              !rescheduleConfigUpToDate ||
              !mainConfigUpToDate ||
              !fraudConfigUpToDate ||
              !mapLimitsUpToDate ||
              !closedGroupsUpToDate
          );
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
