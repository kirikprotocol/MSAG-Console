package ru.novosoft.smsc.web.controllers.mcisme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.mcisme.MCISme;
import ru.novosoft.smsc.admin.mcisme.Profile;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;


/**
 * User: artem
 * Date: 24.11.11
 */
public class MCISmeProfilesController extends SmscController {

  private String subscriberAddress = "";
  private Profile profile;

  private boolean profileExists;

  public String lookupProfile() {
    if (subscriberAddress == null || subscriberAddress.trim().length() == 0)
      return null;

    MCISme mciSme = WebContext.getInstance().getMciSme();
    try {
      if (mciSme == null || !mciSme.isOnline()) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.services.mcisme.offline");
        return null;
      }

      Address addr = new Address(subscriberAddress);
      Profile p = mciSme.getProfile(new Address(subscriberAddress));
      profileExists = p != null;
      if (!profileExists)
        p = getDefaultProfile(addr);

      this.profile = p;
    } catch (AdminException e) {
      addError(e);
    }

    return null;
  }

  private Profile getDefaultProfile(Address subscriberAddress) {
    Profile p = new Profile();
    p.setSubscriber(subscriberAddress);
    p.setInform(true);                   //todo read flags from config
    p.setNotify(false);
    p.setWantNotifyMe(false);
    p.setAbsent(false);
    p.setBusy(false);
    p.setDetach(false);
    p.setNoReplay(false);
    p.setUnconditional(false);
    p.setInformTemplateId((byte)-1);
    p.setNotifyTemplateId((byte)-1);
    return p;
  }

  public String clear() {
    profile = null;
    subscriberAddress = null;
    return null;
  }

  public String saveProfile() {

    if (profile == null || subscriberAddress == null || subscriberAddress.trim().length() == 0)
      return null;

    MCISme mciSme = WebContext.getInstance().getMciSme();
    try {
      if (mciSme == null || !mciSme.isOnline()) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.services.mcisme.offline");
        return null;
      }

      profile.setSubscriber(new Address(subscriberAddress));
      mciSme.saveProfile(profile);

      this.profileExists = true;

    } catch (AdminException e) {
      addError(e);
    }

    return null;
  }

  public Profile getProfile() {
    return profile;
  }

  public String getSubscriberAddress() {
    return subscriberAddress;
  }

  public void setSubscriberAddress(String subscriberAddress) {
    this.subscriberAddress = subscriberAddress;
  }

  public boolean isProfileExists() {
    return profileExists;
  }

  public boolean isLookupCalled() {
    return profile != null;
  }
}
