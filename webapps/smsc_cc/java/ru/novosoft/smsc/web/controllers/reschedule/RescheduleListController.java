package ru.novosoft.smsc.web.controllers.reschedule;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.reschedule.Reschedule;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import java.util.*;

/**
 * author: alkhal
 */
public class RescheduleListController extends RescheduleController {

  private static final Logger logger = Logger.getLogger(RescheduleController.class);

  private String defaultReschedule;
  private List selectedRows;
  private Collection<Reschedule> reschedules;

  public RescheduleListController() {
    super(true);

    RescheduleSettings s = getSettings();
    defaultReschedule = s.getDefaultReschedule();
    reschedules = s.getReschedules();
  }

  public String getReschedulePattern() {
    return ru.novosoft.smsc.admin.reschedule.Reschedule.intervalsPattern.pattern();
  }

  public void setSelectedRows(List rows) {
    selectedRows = rows;
  }

  @SuppressWarnings({"unchecked"})
  public void removeSelected(ActionEvent e) {

    try {
      if (selectedRows != null && !selectedRows.isEmpty()) {
        for (String s : (List<String>) selectedRows) {
          for (Iterator<Reschedule> iter = reschedules.iterator(); iter.hasNext();) {
            if (iter.next().getIntervals().equals(s)) {
              iter.remove();
              break;
            }
          }
        }
      }

      RescheduleSettings settings = getSettings();
      settings.setReschedules(reschedules);
      setSettings(settings);
      checkChanges();
    } catch (AdminException ex) {
      addError(ex);
    }
  }

  public String submit() {
    if (!checkDefault()) {
      return null;
    }

    try {
      RescheduleSettings s = getSettings();
      s.setDefaultReschedule(defaultReschedule);
      setSettings(s);
      
      submitSettings();

      return "INDEX";
    } catch (AdminException e) {
      logger.warn(e, e);
      addError(e);
    }

    return null;
  }

  private boolean checkDefault() {
    RescheduleSettings s = getSettings();
    for (Reschedule r : s.getReschedules()) {
      if (r.getIntervals().equals(defaultReschedule)) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.reschedule.duplicate");
        return false;
      }
    }
    return true;
  }

  public String reset() {
    try {
      resetSettings();
    } catch (AdminException e) {
      addError(e);
    }
    return "RESCHEDULE";
  }

  public String edit() {
    if (!checkDefault()) {
      return null;
    }
    return "RESCHEDULE_EDIT";
  }

  public DataTableModel getRescheduleModel() {
    final Collection<Reschedule> reschedules = getSettings().getReschedules();

    return new DataTableModel() {

      public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
        List<Reschedule> result = new ArrayList<Reschedule>(count);
        if (count <= 0) {
          return result;
        }

        for (Iterator<Reschedule> i = reschedules.iterator(); i.hasNext() && count > 0;) {
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

  public String getDefaultReschedule() {
    return defaultReschedule;
  }

  public void setDefaultReschedule(String defaultReschedule) {
    this.defaultReschedule = defaultReschedule;
  }

}
