package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.route.SubjectList;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.admin.users.User;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Iterator;
import java.util.Collection;
import java.util.Map;

/**
 * User: artem
 * Date: 26.05.2008
 */

public class BlackList extends InfoSmeBean {

  private String msisdn;
  private Boolean found;
  private boolean initiated;

  private String mbLookup;
  private String mbAdd;
  private String mbDelete;

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    try {

      if (msisdn != null) {
        found = null;
        if (!isUserAdmin(request) && !isMsisdnAllowed(msisdn, getUser(request))) {
          found = null;
          return error("infosme.error.blacklist.abonent_region_not_allowed", msisdn);
        }
        found = Boolean.valueOf(getInfoSmeContext().getBlackListManager().contains(msisdn));
        if (mbLookup != null) {
          initiated = true;
        } else if (mbAdd != null && initiated && !found.booleanValue()) {
          getInfoSmeContext().getBlackListManager().add(msisdn);
          found = Boolean.TRUE;
          mbAdd = null;
        } else if (mbDelete != null && initiated && found.booleanValue()) {
          getInfoSmeContext().getBlackListManager().remove(msisdn);
          found = Boolean.FALSE;
          mbDelete = null;
        }
      } else {
        initiated = false;
      }

      return RESULT_OK;

    } catch (AdminException e) {
      return error("Error: ", e);
    }
  }

  private boolean isMsisdnAllowed(String msisdn, User user) throws AdminException {
    Collection regions = appContext.getRegionsManager().getRegions();
    SubjectList subjectsList =  appContext.getRouteSubjectManager().getSubjects();
    for (Iterator iter = regions.iterator(); iter.hasNext();) {
      Region r = (Region)iter.next();
      Collection subjects = r.getSubjects();
      for (Iterator subiter = subjects.iterator(); subiter.hasNext();) {
        Subject s = subjectsList.get((String)subiter.next());
        MaskList masks = s.getMasks();

        for (Iterator maskiter = masks.getNames().iterator(); maskiter.hasNext();) {
          Mask m = masks.get((String)maskiter.next());
          if (m.addressConfirm(msisdn))
            return user.getPrefs().isInfoSmeRegionAllowed(String.valueOf(r.getId()));
        }
      }
    }
    return false;
  }

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) {
    this.msisdn = msisdn;
  }

  public Boolean isFound() {
    return found;
  }

  public void setFound(Boolean found) {
    this.found = found;
  }

  public boolean isInitiated() {
    return initiated;
  }

  public void setInitiated(boolean initiated) {
    this.initiated = initiated;
  }

  public String getMbLookup() {
    return mbLookup;
  }

  public void setMbLookup(String mbLookup) {
    this.mbLookup = mbLookup;
  }

  public String getMbAdd() {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd) {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete() {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }
}
