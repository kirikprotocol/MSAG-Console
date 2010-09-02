package ru.novosoft.smsc.web.controllers;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.reschedule.Reschedule;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import javax.servlet.http.HttpSession;
import java.util.*;

/**
 * alkhal: alkhal
 */
@SuppressWarnings({"unchecked"})
public class RescheduleEditController extends RescheduleController{

  private static final Logger logger = Logger.getLogger(RescheduleEditController.class);

  private boolean edit_initialized;

  private String oldReschedule;

  private String newReschedule;

  private Set<Integer> availableStatuses;
  private Reschedule defaultReschedule;

  private Map<String, Reschedule> reschedules;

  private DynamicTableModel dynamicModel;

  private SmppStatusConverter statusConverter = new SmppStatusConverter();

  public RescheduleEditController() {
    HttpSession session = getSession(false);
    defaultReschedule = getDefRescheduleFromSession(session);
    reschedules = getReschedulesFromSession(session);
    if(reschedules == null || defaultReschedule == null) {
      throw new IllegalStateException("Session's parameters aren't initialized correctly");
    }

    if(getRequestParameter("edit_initialized") == null) {
      oldReschedule = getRequestParameter("reschedule");
      newReschedule = oldReschedule;
      edit_initialized = true;
    }

    dynamicModel = new DynamicTableModel();

    availableStatuses = new TreeSet<Integer>(Arrays.asList(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 17, 19, 20, 21, 51, 52, 64, 66, 67, 68, 69, 72, 73,
        80, 81, 83, 84, 85, 88, 97, 98, 99, 100, 101, 102, 103, 192, 193, 194, 195, 196, 254, 255, 260,
        1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1134, 1136, 1137, 1138, 1139, 1140, 1141, 1142, 1143,
        1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1153, 1154, 1155, 1157, 1158, 1160, 1161, 1163, 1164,
        1165, 1173, 1179, 1183, 1184, 1185, 1186, 1187, 1188));
    for(Reschedule r : reschedules.values()) {
      if((oldReschedule != null && oldReschedule.length() != 0) && r.getIntervals().equals(oldReschedule)) {
        for(Integer st : r.getStatuses()) {
          DynamicTableRow row = new DynamicTableRow();
          row.setValue("code", statusConverter.getAsString(FacesContext.getCurrentInstance(), st));
          dynamicModel.addRow(row);
        }
      }else {
        for(Integer s : r.getStatuses()) {
          availableStatuses.remove(s);
        }
      }
    }
  }

  public DynamicTableModel getDynamicModel() {
    return dynamicModel;
  }

  public void setDynamicModel(DynamicTableModel dynamicModel) {
    this.dynamicModel = dynamicModel;
  }

  public boolean isAvailable() {
    return !availableStatuses.isEmpty();
  }


  public boolean isEdit_initialized() {
    return edit_initialized;
  }

  public void setEdit_initialized(boolean edit_initialized) {
    this.edit_initialized = edit_initialized;
  }

  public String getOldReschedule() {
    return oldReschedule;
  }

  public void setOldReschedule(String oldReschedule) {
    this.oldReschedule = oldReschedule;
  }

  public String getNewReschedule() {
    return newReschedule;
  }

  public void setNewReschedule(String newReschedule) {
    this.newReschedule = newReschedule;
  }

  public String getReschedulePattern() {
    return ru.novosoft.smsc.admin.reschedule.Reschedule.intervalsPattern.pattern();
  }


  public List<String> getAvailableStatuses() {
    List<String> res = new ArrayList<String>(availableStatuses.size());
    for(Integer s : availableStatuses) {
      res.add(statusConverter.getAsString(FacesContext.getCurrentInstance(), null, s));
    }
    return res;
  }

  public String done() {

    if(defaultReschedule.getIntervals().equals(newReschedule)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.reschedule.duplicate");
      return null;
    }

    for(Reschedule r : reschedules.values()) {
      if(r.getIntervals().equals(newReschedule) && (oldReschedule == null || oldReschedule.length() == 0 || !oldReschedule.equals(newReschedule))) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.reschedule.duplicate");
        return null;
      }
    }

    if(dynamicModel.getRowCount() == 0) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.reschedule.statuses.empty");
      return null;
    }

    List<Integer> statuses = new ArrayList<Integer>(dynamicModel.getRowCount());
    for(DynamicTableRow row : dynamicModel.getRows()) {
      statuses.add(statusConverter.getAsInteger((String)row.getValue("code")));
    }

    try{
      Reschedule toAdd = new Reschedule(newReschedule, statuses);
      if(oldReschedule != null && oldReschedule.length() != 0) {
        reschedules.remove(oldReschedule);
      }
      reschedules.put(newReschedule, toAdd);
      setChanged(true);
    }catch (AdminException e) {
      logger.error(e,e);
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, e.getMessage(getLocale()));
      return null;
    }

    return "RESCHEDULE";
  }

}
