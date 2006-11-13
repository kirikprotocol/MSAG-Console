package ru.novosoft.smsc.jsp.smsc.smscenters;

import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * User: artem
 * Date: 13.11.2006
 */

public class Index extends SmscBean {
  private SMSCenters smsCenters = null;
  private String mbSave = null;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      smsCenters = SMSCenters.getInstance(appContext);
      if (smsCenters == null) {
        throw new Throwable();
      }
    }
    catch (Throwable e) {
      logger.error("couldn't instantiate timezones", e);
      return error(SMSCErrors.error.smsc.timezones.couldntGetTimeZones, e);
    }

    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    if (mbSave != null)
      return processApply(request);

    return RESULT_OK;
  }

  public Collection getAllRoutes() {
    Collection usedSubjs = getSubjects();
    Collection allSubjs = routeSubjectManager.getSubjects().getNames();
    for (Iterator i = usedSubjs.iterator(); i.hasNext();) {
      allSubjs.remove(i.next());
    }
    return allSubjs;
  }

  public Collection getMasks() {
    if (smsCenters == null) return new LinkedList();
    return smsCenters.getMasks();
  }

  public String getMasksSMSCNumber(String mask) {
    return (smsCenters == null) ? null : smsCenters.getMasksSMSCNumber(mask);
  }

  public String getRoutesSMSCNumber(String route) {
    return (smsCenters == null) ? null : smsCenters.getRoutesSMSCNumber(route);
  }

  public Collection getSubjects() {
    if (smsCenters == null) return new LinkedList();
    System.out.println("ROUTES:");
    for (Iterator iterator = smsCenters.getRoutes().iterator(); iterator.hasNext();)
      System.out.println(iterator.next());
    return smsCenters.getRoutes();
  }

  private int processApply(HttpServletRequest request) {
    int result = RESULT_DONE;

    Enumeration parameterNames = request.getParameterNames();

    String newMaskName = null;
    String newMaskValue = null;

    Collection oldMasks = smsCenters.getMasks();
    Collection oldSubjects = smsCenters.getRoutes();
    System.out.println("save");
    while (parameterNames.hasMoreElements()) {
      String s = (String) parameterNames.nextElement();
      if (s.equals("newParamName_" + SMSCenters.SMS_CENTERS_MASKS_PREFIX)) {
        newMaskName = request.getParameter(s);
        continue;
      }
      if (s.equals("newParamValue_" + SMSCenters.SMS_CENTERS_MASKS_PREFIX)) {
        newMaskValue = request.getParameter(s);
        continue;
      }

      if (s.indexOf('.') <= 0) continue;

      final String parameter = request.getParameter(s);
      if (s.startsWith(SMSCenters.SMS_CENTERS_MASKS_PREFIX)) {
        s = s.substring(SMSCenters.SMS_CENTERS_MASKS_PREFIX.length() + 1);
        oldMasks.remove(s);
        smsCenters.addMask(s, parameter);
        continue;
      }

      if (s.startsWith(SMSCenters.SMS_CENTERS_SUBJECTS_PREFIX)) {
        s = s.substring(SMSCenters.SMS_CENTERS_SUBJECTS_PREFIX.length() + 1);
        oldSubjects.remove(s);
        smsCenters.addSubject(s, parameter);
      }
    }

    for (Iterator i = oldMasks.iterator(); i.hasNext();)
      smsCenters.removeMask((String)i.next());

    for (Iterator i = oldSubjects.iterator(); i.hasNext();)
      smsCenters.removeSubject((String)i.next());

    try {
      if (newMaskName != null && !(newMaskName.equals("")) && newMaskValue != null && !(newMaskValue.equals("")))
        smsCenters.addMask(newMaskName, newMaskValue);

      smsCenters.save();
    } catch (Exception e) {
      return error(SMSCErrors.error.smsc.smscenters.couldntSave, e);
    }
    return result;
  }

  public String getMbSave() {
    return mbSave;
  }

  public void setMbSave(String mbSave) {
    this.mbSave = mbSave;
  }
}
