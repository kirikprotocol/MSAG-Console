package ru.novosoft.smsc.web.controllers;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRow;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRowBase;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.config.SmscStatusManager;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import javax.faces.event.ActionEvent;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class SmscStatusController {

  private static final Logger logger = Logger.getLogger(SmscStatusController.class);

  private final FacesContext fc = FacesContext.getCurrentInstance();
  private final ResourceBundle rb = ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc",
      fc.getExternalContext().getRequestLocale());

  private final SmscStatusManager smscStatusManager;

  public SmscStatusController() {
    smscStatusManager = WebContext.getInstance().getSmscStatusManager();
  }



  public void start() {
    String instanceNumber = FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get("instanceNumber");
    if (instanceNumber != null)
      try {
        smscStatusManager.startSmsc(Integer.parseInt(instanceNumber));
      } catch (AdminException e) {
        logError(e);
      }
  }

  public void stop() {
    String instanceNumber = FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get("instanceNumber");
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
    System.out.println("1");
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
    FacesMessage facesMessage = new FacesMessage(FacesMessage.SEVERITY_ERROR, rb.getString("status.page.load.error"), e.getMessage(fc.getExternalContext().getRequestLocale()));
    fc.addMessage("smsc_errors", facesMessage);
    logger.error(e, e);
  }

  public DataTableModel getSmsCenters() {
    return new DataTableModel() {

      public List<DataTableRow> getRows(int startPos, int count, DataTableSortOrder sortOrder) {
        List<DataTableRow> result = new ArrayList<DataTableRow>(count);
        try {
          for (int i=startPos; i < Math.min(startPos + count, smscStatusManager.getSmscInstancesNumber()); i++) {

            SmscStatus status = new SmscStatus(smscStatusManager.getSmscOnlineHost(i));
            status.addStatus(new SmscCfgState("main", smscStatusManager.getMainConfigState(i)));
            status.addStatus(new SmscCfgState("reschedule", smscStatusManager.getRescheduleState(i)));

            DataTableRowBase row = new DataTableRowBase(String.valueOf(i), status, null);
            if (status.isHasErrors()) {
              for (SmscCfgState state : status.getCfgStatuses());
                
            }

//            result.add();
          }
        } catch (AdminException e) {
          logError(e);
        }
        return result;
      }

      public int getRowsCount() {
        try {
          return smscStatusManager.getSmscInstancesNumber();
        } catch (AdminException e) {
          logError(e);
          return 0;
        }
      }
    };
  }

  public class SmscStatus {

    private String onlineHost;
    private List<SmscCfgState> cfgStates = new ArrayList<SmscCfgState>();

    public SmscStatus(String onlineHost) {
      this.onlineHost = onlineHost;

    }

    public String getOnlineHost() {
      return onlineHost;
    }

    public void addStatus(SmscCfgState cfgState) {
      cfgStates.add(cfgState);
    }

    public List<SmscCfgState> getCfgStatuses() {
      return cfgStates;
    }

    public boolean isHasErrors() {
      for (SmscCfgState s : cfgStates)
        if (s.getState() == SmscConfigurationStatus.OUT_OF_DATE)
          return true;
      return false;
    }
  }


  public class SmscCfgState {
    private String configName;
    private SmscConfigurationStatus state;

    public SmscCfgState(String configName, SmscConfigurationStatus state) {
      this.configName = configName;
      this.state = state;
    }

    public String getConfigName() {
      return configName;
    }

    public void setConfigName(String configName) {
      this.configName = configName;
    }

    public SmscConfigurationStatus getState() {
      return state;
    }

    public void setState(SmscConfigurationStatus state) {
      this.state = state;
    }
  }

}
