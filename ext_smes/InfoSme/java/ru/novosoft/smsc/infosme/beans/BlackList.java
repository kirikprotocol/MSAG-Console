package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * User: artem
 * Date: 26.05.2008
 */

public class BlackList extends InfoSmeBean {

  private String msisdn;
  private boolean found;
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
        found = getInfoSmeContext().getBlackListManager().contains(msisdn);
        if (mbLookup != null) {
          initiated = true;
        } else if (mbAdd != null && initiated && !found) {
          System.out.println("ADD MSISDN");
          getInfoSmeContext().getBlackListManager().add(msisdn);
          found = true;
          mbAdd = null;
        } else if (mbDelete != null && initiated && found) {
          System.out.println("DELETE MSISDN");
          getInfoSmeContext().getBlackListManager().remove(msisdn);
          found = false;
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

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) {
    this.msisdn = msisdn;
  }

  public boolean isFound() {
    return found;
  }

  public void setFound(boolean found) {
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
