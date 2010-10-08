package ru.novosoft.smsc.web.controllers.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profile.Profile;
import ru.novosoft.smsc.admin.profile.ProfileLookupResult;
import ru.novosoft.smsc.util.Address;

/**
 * @author Artem Snopkov
 */
public class LookupProfileController extends ProfileController {

  private Address address;
  private boolean canAdd;
  private boolean canEdit;
  private boolean canView;
  private ProfileLookupResult profileLookupResult;

  public LookupProfileController() {
    canAdd = getRequestParameter("canAdd") != null && getRequestParameter("canAdd").equals("true");
    canEdit = getRequestParameter("canEdit") != null && getRequestParameter("canEdit").equals("true");
    canView = getRequestParameter("canView") != null && getRequestParameter("canView").equals("true");
  }

  public String getAddress() {
    return address == null ? "" : address.getSimpleAddress();
  }

  public void setAddress(String address) {
    this.address = new Address(address);
  }

  public ProfileLookupResult getLookupResult() {
    return profileLookupResult;
  }

  public boolean isCanAdd() {
    return canAdd;
  }

  public boolean isCanEdit() {
    return canEdit;
  }

  public boolean isCanView() {
    return canView;
  }

  public String lookup() {
    if (address != null) {
      try {
        profileLookupResult = mngr.lookupProfile(address);
        canAdd = !profileLookupResult.isExactMatch();
        canEdit = profileLookupResult.isExactMatch();
        canView = true;
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String remove() {
    if (address != null) {
      try {
        mngr.deleteProfile(new Address(address));
        canEdit = false;
        canAdd = false;
        canView = false;
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

}
