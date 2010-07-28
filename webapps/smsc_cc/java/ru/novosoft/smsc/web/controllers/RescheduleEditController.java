package ru.novosoft.smsc.web.controllers;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.web.beans.Reschedule;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import javax.faces.event.ActionEvent;
import javax.faces.model.SelectItem;
import javax.servlet.http.HttpSession;
import java.io.Serializable;
import java.util.*;

/**
 * alkhal: alkhal
 */
@SuppressWarnings({"unchecked"})
public class RescheduleEditController implements Serializable{

  private static final Logger logger = Logger.getLogger(RescheduleEditController.class);

  private boolean edit_initialized;

  private String oldReschedule;

  private String newReschedule;

  private Set<Integer> availableStatuses;

  private List<Reschedule.Status> statuses = new LinkedList<Reschedule.Status>();

  private Integer newStatus;

  private Reschedule defaultReschedule;

  private Collection<Reschedule> reschedules;

  public RescheduleEditController() {
    HttpSession session = (HttpSession) FacesContext.getCurrentInstance().getExternalContext().getSession(false);
    defaultReschedule = (Reschedule) session.getAttribute("reschedule.default");
    reschedules = (Collection<Reschedule>) session.getAttribute("reschedule.reschedules");
    if(reschedules == null || defaultReschedule == null) {
      throw new IllegalStateException("Session's parameters aren't initialized correctly");
    }

    Map<String, String> request = FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap();
    if(request.get("edit_initialized") == null) {

      oldReschedule = request.get("reschedule");

      newReschedule = oldReschedule;

      if((oldReschedule != null && oldReschedule.length() != 0) && oldReschedule.equals(defaultReschedule.getIntervals())) {
        availableStatuses = new TreeSet<Integer>();
      }else {
        availableStatuses = new TreeSet<Integer>(Arrays.asList(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 17, 19, 20, 21, 51, 52, 64, 66, 67, 68, 69, 72, 73,
            80, 81, 83, 84, 85, 88, 97, 98, 99, 100, 101, 102, 103, 192, 193, 194, 195, 196, 254, 255, 260,
            1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1134, 1136, 1137, 1138, 1139, 1140, 1141, 1142, 1143,
            1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1153, 1154, 1155, 1157, 1158, 1160, 1161, 1163, 1164,
            1165, 1173, 1179, 1183, 1184, 1185, 1186, 1187, 1188));
        for(Reschedule r : reschedules) {
          if((oldReschedule != null && oldReschedule.length() != 0) && r.getIntervals().equals(oldReschedule)) {
            statuses = new ArrayList<Reschedule.Status>(r.getStatuses());
          }
          for(Reschedule.Status s : r.getStatuses()) {
            availableStatuses.remove(s.getStatus());
          }
        }
      }
      edit_initialized = true;
    }
  }


  public boolean isAvailable() {
    return !availableStatuses.isEmpty();
  }

  public Integer getNewStatus() {
    return newStatus;
  }

  public void setNewStatus(Integer newStatus) {
    this.newStatus = newStatus;
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

  public Set<Integer> getAvailableStatuses() {
    return availableStatuses;
  }

  public List<SelectItem> getAvailableStatusesItems() {
    SmppStatusConverter statusConverter = new SmppStatusConverter();
    List<SelectItem> res = new ArrayList<SelectItem>(availableStatuses.size());
    for(Integer s : availableStatuses) {
      res.add(new SelectItem(s, statusConverter.getAsString(FacesContext.getCurrentInstance(), null, s)));
    }
    return res;
  }

  public void setAvailableStatuses(Set<Integer> availableStatuses) {
    this.availableStatuses = availableStatuses;
  }

  public List<Reschedule.Status> getStatuses() {
    return statuses;
  }

  public void setStatuses(List<Reschedule.Status> statuses) {
    this.statuses = statuses;
  }


  public void done(ActionEvent ev) {
    if(newStatus != null) {
      addStatus(null);
    }

    FacesContext fc = FacesContext.getCurrentInstance();

    for(Reschedule r : reschedules) {
      if(r.getIntervals().equals(newReschedule) && (oldReschedule == null || oldReschedule.length() == 0 || !oldReschedule.equals(newReschedule))) {
        FacesMessage facesMessage = new FacesMessage(FacesMessage.SEVERITY_WARN,
            ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", fc.getExternalContext().getRequestLocale()).getString("smsc.reschedule.duplicate"),
            null);
        fc.addMessage("smsc_errors", facesMessage);
        return;
      }
    }
      if((oldReschedule != null && oldReschedule.length() != 0) && oldReschedule.equals(defaultReschedule.getIntervals())) {

        defaultReschedule.setIntervals(newReschedule);

      }else {
        if(statuses.isEmpty()) {
          FacesMessage facesMessage = new FacesMessage(FacesMessage.SEVERITY_WARN,
              ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", fc.getExternalContext().getRequestLocale()).getString("smsc.reschedule.statuses.empty"),
              null);
          fc.addMessage("smsc_errors", facesMessage);
          return;
        }

        if(oldReschedule != null && oldReschedule.length() != 0) {
          Iterator<Reschedule> i = reschedules.iterator();
          while(i.hasNext()) {
            Reschedule r = i.next();
            if(r.getIntervals().equals(oldReschedule)) {
              i.remove();
              break;
            }
          }
        }

        reschedules.add(new Reschedule(newReschedule, statuses));
//        rm.setReschedules(newRss);
      }

      fc.getApplication().getNavigationHandler().handleNavigation(fc, null, "RESCHEDULE");
  }

  public void addStatus(ActionEvent e) {
    if(newStatus != null) {
      availableStatuses.remove(newStatus);
      statuses.add(new Reschedule.Status(newStatus));
      newStatus = null;
    }
  }
  public void removeStatus(ActionEvent e) {
    String toRemoveS = FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get("removeStatus");
    Integer toRemove = Integer.parseInt(toRemoveS);
    availableStatuses.add(toRemove);
    statuses.remove(new Reschedule.Status(toRemove));
    newStatus = null;
  }
}
