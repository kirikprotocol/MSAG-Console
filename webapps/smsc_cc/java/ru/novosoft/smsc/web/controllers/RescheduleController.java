package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.admin.reschedule.Reschedule;

import javax.servlet.http.HttpSession;
import java.util.Map;

/**
 * author: alkhal
 */
@SuppressWarnings({"unchecked"})
public class RescheduleController extends SmscController {
  

  protected Reschedule getDefRescheduleFromSession() {
    return getDefRescheduleFromSession(getSession(false));
  }

  protected Reschedule getDefRescheduleFromSession(HttpSession s) {
    return (Reschedule)s.getAttribute("reschedule.default");
  }

  protected Map<String, Reschedule> getReschedulesFromSession() {
    return getReschedulesFromSession(getSession(false));
  }

  protected Map<String, Reschedule> getReschedulesFromSession(HttpSession s) {
    return (Map<String, Reschedule>)s.getAttribute("reschedule.reschedules");
  }

  protected void putDefRescheduleToSession(Reschedule r) {
    putDefRescheduleToSession(getSession(false), r);
  }

  protected void putDefRescheduleToSession(HttpSession s, Reschedule r) {
    s.setAttribute("reschedule.default", r);
  }

  protected void putReschedulesToSession(Map<String, Reschedule> rs) {
    putReschedulesToSession(getSession(false), rs);
  }

  protected void putReschedulesToSession(HttpSession s, Map<String, Reschedule> rs) {
    s.setAttribute("reschedule.reschedules", rs);
  }

  protected void setLastUpdate(long u) {
    setLastUpdate(getSession(false), u);
  }

  protected void setLastUpdate(HttpSession s, long u) {
    s.setAttribute("reschedule.last.update", u);
  }

  protected Long getLastUpdate() {
    return getLastUpdate(getSession(false));
  }

  protected Long getLastUpdate(HttpSession s) {
    return (Long)s.getAttribute("reschedule.last.update");
  }

  protected void cleanSession() {
    cleanSession(getSession(false));
  }

  protected void cleanSession(HttpSession s) {
    s.removeAttribute("reschedule.reschedules");
    s.removeAttribute("reschedule.last.update");
    s.removeAttribute("reschedule.default");
  }

  public void setChanged(boolean changed) {
    HttpSession s = getSession(false);
    if(changed) {
      s.setAttribute("reschedule.changed", true);
    }else {
      s.removeAttribute("reschedule.changed");
    }
  }

  public boolean isChanged() {
    return getSession(false).getAttribute("reschedule.changed") != null;
  }

}
