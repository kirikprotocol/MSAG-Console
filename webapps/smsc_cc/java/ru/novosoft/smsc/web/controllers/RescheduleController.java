package ru.novosoft.smsc.web.controllers;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.beans.Reschedule;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRow;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRowBase;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.config.AppliableConfiguration;
import ru.novosoft.smsc.web.config.UpdateInfo;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import javax.servlet.http.HttpSession;
import java.security.Principal;
import java.util.*;

/**
 * author: alkhal
 */
@SuppressWarnings({"unchecked"})
public class RescheduleController extends SmscController {

  private Map<String, Reschedule> reschedules;

  private Reschedule defaultReschedule;

  private AppliableConfiguration conf;

  private static final Logger logger = Logger.getLogger(RescheduleController.class);

  private boolean index_initialized;

  private HttpSession session;

  private DataTableModel rescheduleModel;

  public RescheduleController() {
    session = getSession(false);
    conf = WebContext.getInstance().getAppliableConfiguration();      
    if(getRequestParameter("index_initialized") == null) {
      initReschedules();
      index_initialized = true;
    }
    rescheduleModel = new DataTableModel() {

      public List<DataTableRow> getRows(int startPos, int count, DataTableSortOrder sortOrder) {
        List<DataTableRow> result = new ArrayList<DataTableRow>(count);
        if(count <= 0) {
          return result;
        }
        if(--startPos < 0) {
          result.add(new DataTableRowBase(defaultReschedule.getIntervals(), defaultReschedule, null));
          count--;
        }
        for(Iterator<Reschedule> i = reschedules.values().iterator();i.hasNext() && count>0;) {
          Reschedule r = i.next();
          if(--startPos < 0) {
            result.add(new DataTableRowBase(r.getIntervals(), r, ""));
            count--;
          }
        }
        return result;
      }

      public int getRowsCount() {
        return reschedules.size() + 1;
      }
    };
  }

  private void initReschedules() {
    if(session.getAttribute("reschedule.default") == null || session.getAttribute("reschedule.reschedules") == null) {
      session.setAttribute("reschedule.last.update", conf.getRescheduleSettingsUpdateInfo());
      RescheduleSettings rescheduleSettings =conf.getRescheduleSettings();
      Collection<ru.novosoft.smsc.admin.reschedule.Reschedule> rs = rescheduleSettings.getReschedules();
      reschedules = new LinkedHashMap<String, Reschedule>(rs.size());
      for(ru.novosoft.smsc.admin.reschedule.Reschedule r : rs) {
        Reschedule reschedule = new Reschedule();
        reschedule.setIntervals(r.getIntervals());
        for(Integer s : r.getStatuses()) {
          reschedule.addStatus(new Reschedule.Status(s));
        }
        reschedules.put(reschedule.getIntervals(), reschedule);
      }
      defaultReschedule = new Reschedule(rescheduleSettings.getDefaultReschedule());
    }else {
      reschedules = (Map<String,Reschedule>)session.getAttribute("reschedule.reschedules");
      defaultReschedule = (Reschedule)session.getAttribute("reschedule.default");
    }
  }


  private List selectedRows;

  public void setSelectedRows(List rows) {
    selectedRows = rows;
    System.out.println("Selected: "+rows);

  }

  public void removeSelected(ActionEvent e) {
    if(selectedRows != null && !selectedRows.isEmpty()) {
      for(String s : (List<String>)selectedRows) {
        reschedules.remove(s);
      }
    }
    session.setAttribute("reschedule.reschedules", reschedules);
  }

  public String submit() {

    UpdateInfo lastChange = (UpdateInfo)session.getAttribute("reschedule.last.update");
    if(lastChange != conf.getRescheduleSettingsUpdateInfo()) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual");
      return null;
    }

    try{
      RescheduleSettings settings = conf.getRescheduleSettings();

      Collection<ru.novosoft.smsc.admin.reschedule.Reschedule> newReschedules =
          new ArrayList<ru.novosoft.smsc.admin.reschedule.Reschedule>(reschedules.size());
      for(Reschedule r : reschedules.values()) {
        newReschedules.add(convert(r));
      }
      settings.setReschedules(newReschedules);
      settings.setDefaultReschedule(defaultReschedule.getIntervals());

      Principal p = getUserPrincipal();

      conf.setRescheduleSettings(settings, p.getName());

      session.removeAttribute("reschedule.reschedules");
      session.removeAttribute("reschedule.default");
      session.removeAttribute("reschedule.last.update");
      return "INDEX";

    } catch (AdminException e) {
      logger.warn(e,e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));

      return null;
    }
  }

  public void reset(ActionEvent ev) {
    session.removeAttribute("reschedule.reschedules");
    session.removeAttribute("reschedule.default");
    session.removeAttribute("reschedule.last.update");
    initReschedules();
  }

  public void edit(ActionEvent ev) {
    session.setAttribute("reschedule.reschedules", reschedules);
    session.setAttribute("reschedule.default", defaultReschedule);
  }


  public ru.novosoft.smsc.admin.reschedule.Reschedule convert(Reschedule reschedule) throws AdminException{
    Collection<Integer> statuses = new ArrayList<Integer>(reschedule.getStatuses().size());
    for(Reschedule.Status c : reschedule.getStatuses()) {
      statuses.add(c.getStatus());
    }
    return new ru.novosoft.smsc.admin.reschedule.Reschedule(reschedule.getIntervals(), statuses);
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
}
