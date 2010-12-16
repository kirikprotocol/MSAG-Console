package mobi.eyeline.informer.web.controllers.blacklist;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;

/**
 * @author Aleksandr Khalitov
 */
public class BlacklistController extends InformerController {

  private Address msisdn;

  private boolean init = false;

  private boolean contains;  

  public boolean isBlacklistEnabled() {
    try{
      return getConfig().isBlackListEnabled();
    }catch (AdminException e) {
      addError(e);
      return false;
    }
  }

  public String lookup() {
    if (validateMsisdn()) {
      init = true;
      try {
        contains = getConfig().blacklistContains(msisdn);
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String add() {
    if (validateMsisdn()) {
      try {
        getConfig().addInBlacklist(msisdn, getUserName());
        contains = true;
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String remove() {
    if (validateMsisdn()) {
      try {
        getConfig().removeFromBlacklist(msisdn, getUserName());
        contains = false;
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public boolean isContains() {
    return contains;
  }

  public void setContains(boolean contains) {
    this.contains = contains;
  }

  public Address getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(Address msisdn){
    this.msisdn = msisdn;
  }

  private boolean validateMsisdn() {
    if (msisdn == null) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "validation.msisdn");
      return false;
    }
    return true;
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }


}
