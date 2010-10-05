package mobi.eyeline.informer.web.controllers.blacklist;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;

/**
 * @author Aleksandr Khalitov
 */
public class BlacklistController extends InformerController{

  private String msisdn;

  private boolean init = false;

  private boolean contains;

  public String lookup() {
    if(validateMsisdn()) {
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
    if(validateMsisdn()) {
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
    if(validateMsisdn()) {
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

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) throws AdminException{
    this.msisdn = msisdn;
  }

  private boolean validateMsisdn() {
    if(msisdn == null || !Address.validate(msisdn)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "blacklist.validation.msisdn");
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
