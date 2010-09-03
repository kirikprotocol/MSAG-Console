package ru.novosoft.smsc.web.controllers.reschedule;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.reschedule.Reschedule;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.config.AppliableConfiguration;
import ru.novosoft.smsc.web.config.SmscStatusManager;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import javax.servlet.http.HttpSession;
import java.security.Principal;
import java.text.MessageFormat;
import java.util.*;

/**
 * author: alkhal
 */
public class RescheduleListController extends RescheduleController {
  private Map<String, Reschedule> reschedules;

  private Reschedule defaultReschedule;

  private AppliableConfiguration conf;

  private static final Logger logger = Logger.getLogger(RescheduleController.class);

  private boolean index_initialized;

  private DataTableModel rescheduleModel;

  public RescheduleListController() {
    System.out.println("RescheduleController RescheduleController RescheduleController");
    conf = WebContext.getInstance().getAppliableConfiguration();
    if (getRequestParameter("index_initialized") == null) {
      initReschedules();
      index_initialized = true;
    }

    SmscStatusManager smscStatusManager = WebContext.getInstance().getSmscStatusManager();
    try {
      List<Integer> outOfDate = new LinkedList<Integer>();
      for (int i = 0; i < smscStatusManager.getSmscInstancesNumber(); i++) {
        if (smscStatusManager.getRescheduleState(i) == SmscConfigurationStatus.OUT_OF_DATE)
          outOfDate.add(i);
      }
      if (!outOfDate.isEmpty()) {
        String message = MessageFormat.format(
            ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", getLocale()).getString("smsc.config.instance.out_of_date"),
            outOfDate.toString());
        System.out.println("Out of date:");
        addMessage(FacesMessage.SEVERITY_WARN, message);
      }
    } catch (AdminException e) {
      logger.error(e, e);
      e.printStackTrace();
    }

    rescheduleModel = new DataTableModel() {

      public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
        List<Reschedule> result = new ArrayList<Reschedule>(count);
        if (count <= 0) {
          return result;
        }
        for (Iterator<Reschedule> i = reschedules.values().iterator(); i.hasNext() && count > 0;) {
          Reschedule r = i.next();
          if (--startPos < 0) {
            result.add(r);
            count--;
          }
        }
        return result;
      }

      public int getRowsCount() {
        return reschedules.size();
      }
    };
  }

  private void initReschedules() {
    HttpSession session = getSession(false);
    if(isChanged()) {
      reschedules = getReschedulesFromSession(session);
      defaultReschedule = getDefRescheduleFromSession(session);
      viewChanges();
    }else{
      setLastUpdate(conf.getRescheduleSettingsUpdateInfo().getLastUpdateTime());
      try {
        RescheduleSettings rescheduleSettings = conf.getRescheduleSettings();
        Collection<ru.novosoft.smsc.admin.reschedule.Reschedule> rs = rescheduleSettings.getReschedules();
        reschedules = new LinkedHashMap<String, Reschedule>(rs.size());
        for (ru.novosoft.smsc.admin.reschedule.Reschedule r : rs) {
          reschedules.put(r.getIntervals(), r);
        }
        defaultReschedule = new Reschedule(rescheduleSettings.getDefaultReschedule());
      } catch (AdminException e) {
        logger.error(e, e);
        addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
      }
    }
  }

  public String getReschedulePattern() {
    return ru.novosoft.smsc.admin.reschedule.Reschedule.intervalsPattern.pattern();
  }


  private List selectedRows;

  public void setSelectedRows(List rows) {
    selectedRows = rows;
    System.out.println("Selected: " + rows);

  }

  @SuppressWarnings({"unchecked"})
  public void removeSelected(ActionEvent e) {
    if (selectedRows != null && !selectedRows.isEmpty()) {
      for (String s : (List<String>) selectedRows) {
        reschedules.remove(s);
      }
      setChanged(true);
      viewChanges();
    }
    putReschedulesToSession(reschedules);
    putDefRescheduleToSession(defaultReschedule);
  }

  public String submit() {
    HttpSession s = getSession(false);
    long lastChange = getLastUpdate();
    if (lastChange != conf.getRescheduleSettingsUpdateInfo().getLastUpdateTime()) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual");
      return null;
    }

    if (!checkDefault()) {
      return null;
    }

    try {
      RescheduleSettings settings = conf.getRescheduleSettings();

      Collection<ru.novosoft.smsc.admin.reschedule.Reschedule> newReschedules =
          new ArrayList<ru.novosoft.smsc.admin.reschedule.Reschedule>(reschedules.size());
      for (Reschedule r : reschedules.values()) {
        newReschedules.add(r);
      }
      settings.setReschedules(newReschedules);
      settings.setDefaultReschedule(defaultReschedule.getIntervals());

      Principal p = getUserPrincipal();

      conf.setRescheduleSettings(settings, p.getName());

      cleanSession(s);
      setChanged(false);

      return "INDEX";
    } catch (AdminException e) {
      logger.warn(e, e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));

      return null;
    }
  }

  private boolean checkDefault() {
    for (Reschedule r : reschedules.values()) {
      if (r.getIntervals().equals(defaultReschedule.getIntervals())) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.reschedule.duplicate");
        return false;
      }
    }
    return true;
  }


  public String reset() {
    cleanSession();
    setChanged(false);
    return "RESCHEDULE";
  }

  public String edit() {
    if (!checkDefault()) {
      return null;
    }
    putReschedulesToSession(reschedules);
    putDefRescheduleToSession(defaultReschedule);
    return "RESCHEDULE_EDIT";
  }

  public DataTableModel getRescheduleModel() {
    return rescheduleModel;
  }

  public boolean isIndex_initialized() {
    return index_initialized;
  }

  public void setIndex_initialized(boolean index_initialized) {
    this.index_initialized = index_initialized;
  }

  public Reschedule getDefaultReschedule() {
    return defaultReschedule;
  }

  public void setDefaultReschedule(Reschedule defaultReschedule) {
    this.defaultReschedule = defaultReschedule;
  }

  public Map<String, Reschedule> getReschedules() {
    return reschedules;
  }

  public void setReschedules(Map<String, Reschedule> reschedules) {
    this.reschedules = reschedules;
  }

  public void viewChanges() {
    System.out.println("Changes:");
    addLocalizedMessage(FacesMessage.SEVERITY_INFO, "smsc.reschedule.submit.hint");
  }
}
