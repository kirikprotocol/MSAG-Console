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
  private ProfileLookupResult profileLookupResult;

  public String getAddress() {
    return address == null ? "" : address.getSimpleAddress();
  }

  public void setAddress(String address) {
    this.address = new Address(address);
  }

  public ProfileLookupResult getLookupResult() {
    return profileLookupResult;
  }

  public String lookup() {
    if (address != null) {
      try {
        profileLookupResult = mngr.lookupProfile(address);
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

}
