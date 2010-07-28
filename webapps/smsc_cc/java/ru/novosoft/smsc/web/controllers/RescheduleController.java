package ru.novosoft.smsc.web.controllers;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.reschedule.RescheduleManager;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.beans.Reschedule;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import javax.faces.event.ActionEvent;
import javax.servlet.http.HttpSession;
import java.io.Serializable;
import java.util.*;

/**
 * author: alkhal
 */
public class RescheduleController implements Serializable{

  private List<Reschedule> reschedules;

  private Reschedule defaultReschedule;

  private AdminContext adminContext;

  private static final Logger logger = Logger.getLogger(RescheduleController.class);

  private boolean index_initialized;

  private HttpSession session;

  public RescheduleController() {
    session = (HttpSession)FacesContext.getCurrentInstance().getExternalContext().getSession(false);
    adminContext = WebContext.getInstance().getAdminContext();
    if(FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get("index_initialized") == null) {
      initReschedules();
      index_initialized = true;
    }
  }

  private void initReschedules() {
    if(session.getAttribute("reschedule.default") == null || session.getAttribute("reschedule.reschedules") == null) {
      session.setAttribute("reschedule.last.update", adminContext.getRescheduleManager().getLastChangeTime());
      Collection<ru.novosoft.smsc.admin.reschedule.Reschedule> rs = adminContext.getRescheduleManager().getReschedules();
      reschedules = new ArrayList<Reschedule>(rs.size());
      for(ru.novosoft.smsc.admin.reschedule.Reschedule r : rs) {
        Reschedule reschedule = new Reschedule();
        reschedule.setIntervals(r.getIntervals());
        for(Integer s : r.getStatuses()) {
          reschedule.addStatus(new Reschedule.Status(s));
        }
        reschedules.add(reschedule);
      }
      defaultReschedule = new Reschedule(adminContext.getRescheduleManager().getDefaultReschedule());
    }else {
      reschedules = (List<Reschedule>)session.getAttribute("reschedule.reschedules");
      defaultReschedule = (Reschedule)session.getAttribute("reschedule.default");
    }
  }

  public void removeSelected(ActionEvent ev) {
    Iterator<Reschedule> i = reschedules.iterator();
    while(i.hasNext()) {
      Reschedule r = i.next();
      if(r.isChecked()) {
        i.remove();
      }
    }
    session.setAttribute("reschedule.reschedules", reschedules);
  }

  public void submit(ActionEvent ev) {
    FacesContext fc = FacesContext.getCurrentInstance();
    RescheduleManager rm = adminContext.getRescheduleManager();

    Long lastChange = (Long)session.getAttribute("reschedule.last.update");
    if(lastChange != rm.getLastChangeTime()) {
      FacesMessage facesMessage = new FacesMessage(FacesMessage.SEVERITY_ERROR,
          ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc",
              fc.getExternalContext().getRequestLocale()).getString("smsc.config.not.actual"), "");
      fc.addMessage("smsc_errors", facesMessage);
      return;
    }

    try{

      Collection<ru.novosoft.smsc.admin.reschedule.Reschedule> newReschedules =
          new ArrayList<ru.novosoft.smsc.admin.reschedule.Reschedule>(reschedules.size());
      for(Reschedule r : reschedules) {
        newReschedules.add(convert(r));
      }
      rm.setReschedules(newReschedules);
      rm.setDefaultReschedule(defaultReschedule.getIntervals());

      session.removeAttribute("reschedule.reschedules");
      session.removeAttribute("reschedule.default");
      session.removeAttribute("reschedule.last.update");
      fc.getApplication().getNavigationHandler().handleNavigation(fc, null, "INDEX");

    } catch (AdminException e) {
      logger.warn(e,e);

      FacesMessage facesMessage = new FacesMessage(FacesMessage.SEVERITY_ERROR,
          e.getMessage(fc.getExternalContext().getRequestLocale()), "");
      fc.addMessage("smsc_errors", facesMessage);
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
    session.setAttribute("reschedule.default", defaultReschedule);    }


  public ru.novosoft.smsc.admin.reschedule.Reschedule convert(Reschedule reschedule) throws AdminException{
    Collection<Integer> statuses = new ArrayList<Integer>(reschedule.getStatuses().size());
    for(Reschedule.Status c : reschedule.getStatuses()) {
      statuses.add(c.getStatus());
    }
    return new ru.novosoft.smsc.admin.reschedule.Reschedule(reschedule.getIntervals(), statuses);
  }


  public List<Reschedule> getReschedules() {
    return reschedules;
  }

  public Reschedule getDefaultReschedule() {
    return defaultReschedule;
  }

  public void setDefaultReschedule(Reschedule defaultReschedule) {
    this.defaultReschedule = defaultReschedule;
  }

  public void setReschedules(List<Reschedule> reschedules) {
    this.reschedules = reschedules;
  }

  public boolean isIndex_initialized() {
    return index_initialized;
  }

  public void setIndex_initialized(boolean index_initialized) {
    this.index_initialized = index_initialized;
  }
}
